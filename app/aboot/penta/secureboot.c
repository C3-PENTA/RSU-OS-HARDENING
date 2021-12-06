#include "secureboot.h"
#include "crc16.h"

int read_from_puf(unsigned char *buf, unsigned char len) {
    return i2c_xfer(0xc8, 0, 0, len, buf, 1);
}
int write_to_puf(unsigned char *buf, unsigned char len) {
    return i2c_xfer(0xc8, len, buf, 0, 0, 1);
}

int check_wake_up(void) { 
    unsigned char buf[4];
    int ret;

    ret = read_from_puf(buf, 4);
    if (ret < 0) {
        dprintf(CRITICAL, "fail to read from g3(%d)\n", ret);
        return 0;
    }
    if (buf[1] == 0x11) {
        dprintf(CRITICAL, "g3 works\n");
        return 0;
    }
    return -1;
}

void load_cert_chunked(unsigned char *cert, uint64_t size, struct g3_command_info *g3_coms, uint32_t part_num) {
    unsigned char g3_cert_common_head[] = {0x03,0x00,0x8D,0x06,0x01};
    char name_buf[G3_MAX_COMMAND_NAME_LEN];
    unsigned char *ptr = g3_coms->cmd;
    unsigned char *cmd_start_ptr = &g3_coms->cmd[1]; //it points start address of total length

    memset(name_buf, 0x00, G3_MAX_COMMAND_NAME_LEN);
    snprintf(name_buf, G3_MAX_COMMAND_NAME_LEN, "cert_ins_part_%u", part_num);
    memcpy(g3_coms->cmd_name, name_buf, G3_MAX_COMMAND_NAME_LEN);
    memcpy(ptr, g3_cert_common_head, sizeof(g3_cert_common_head));
    ptr += sizeof(g3_cert_common_head);

    if (size < G3_MAX_CERT_CHUNK_LEN) {
        // no remainder
        if (!size) {
            return;
        }
        *ptr++ = 0xff;
        memcpy(ptr, cert, size);
        *cmd_start_ptr = G3_DEF_CMD_LEN + size; // update cmd total length
        get_crc16_reflected(cmd_start_ptr, (*cmd_start_ptr-2), ptr+size); // '-2' means without trailing crc
        return;
    }
    *ptr++ = part_num;
    memcpy(ptr, cert, G3_MAX_CERT_CHUNK_LEN);
    *cmd_start_ptr = G3_DEF_CMD_LEN + G3_MAX_CERT_CHUNK_LEN; // update cmd total length
    get_crc16_reflected(cmd_start_ptr, (*cmd_start_ptr-2), ptr+G3_MAX_CERT_CHUNK_LEN);
    load_cert_chunked(cert + G3_MAX_CERT_CHUNK_LEN,
                      size - G3_MAX_CERT_CHUNK_LEN,
                      ++g3_coms, ++part_num);
    return;
}

int verify_cert_g3(unsigned char *cert, uint64_t size) {
    struct g3_command_info g3_coms[G3_VERI_CERT_COMMAND_END];
    unsigned char res[4];
    int i,j;

    memset(res, 0x00, sizeof(res));
    load_cert_chunked(cert, size, g3_coms, 0);

    for (i = 0; i < G3_VERI_CERT_COMMAND_END; i++) {
        struct g3_command_info *g3_cmd = &(g3_coms[i]);
        unsigned char *cmd_term_ptr = &(g3_cmd->cmd[5]);

        if (write_to_puf(g3_cmd->cmd, (g3_cmd->cmd[1] + 0x01)) < 0) {
            dprintf(CRITICAL, "fail to write to g3\n");
            return -1;
        }
        udelay(340);
        WAIT_FOR_G3_RES(res, 4, g3_cmd->cmd_name);
        dprintf(CRITICAL, "name (%s) ", g3_cmd->cmd_name);
        if (res[1] != 0x00) {
            dprintf(CRITICAL, "Raise Error G3 command name (%s) retrun code ",g3_cmd->cmd_name);
            for (j = 0; j < 4; j++) {
                dprintf(CRITICAL, "%02x ", res[j]);
            }
            dprintf(CRITICAL, "\n");
            return -1;
        }
        dprintf(CRITICAL, "ok..\n");
        if (*cmd_term_ptr == 0xff) break;
    }
    dprintf(CRITICAL, "Verifying Cert... OK!\n");

    return 0;
}

int verify_sig_g3(unsigned char *sig, int sig_len, unsigned char *hash, int hash_len) {
    struct g3_command_info g3_coms[G3_VERI_SIG_COMMAND_END] = {
        {
            .cmd_name = "g3_get_pub_key_6",
            .cmd = { 0x03,0x07,0x8C,0x06,0x00,0x01,0x89,0x7D }
        },
        {
            .cmd_name = "g3_verify_signature",
            .cmd = {}
        }
    };
    unsigned char g3_verify_head[] = { 0x03,0x00,0x87,0x06,0x00,0x00 };
    unsigned char *g3_verify_cmd = g3_coms[G3_VERIFY_SIGNATURE].cmd;
    unsigned char *ptr = g3_verify_cmd;
    unsigned char *cmd_start_ptr = &g3_verify_cmd[1];
    unsigned char res[4];
    int i,j;

    memcpy(ptr, g3_verify_head, sizeof(g3_verify_head));
    ptr += sizeof(g3_verify_head);
    memcpy(ptr, hash, hash_len);
    ptr += hash_len;
    memcpy(ptr, sig, sig_len);
    *cmd_start_ptr = G3_DEF_CMD_LEN + sig_len + hash_len;
    get_crc16_reflected(cmd_start_ptr, (*cmd_start_ptr-2), ptr+sig_len);

    for (i = 0; i < G3_VERI_SIG_COMMAND_END; i++) {
        struct g3_command_info *g3_cmd = &(g3_coms[i]);

        /* get pub key 상관없이 verify signature 가 동작하므로 skip 한다. */
        if (i == G3_GET_PUB_KEY_6) continue;

        if (write_to_puf(g3_cmd->cmd, (g3_cmd->cmd[1] + 0x01)) < 0) {
            dprintf(CRITICAL, "fail to write to g3\n");
            return -1;
        }
        udelay(340);
        WAIT_FOR_G3_RES(res, 4, g3_cmd->cmd_name);
        dprintf(CRITICAL, "name (%s) ", g3_cmd->cmd_name);
        if (res[1] != 0x00) {
            dprintf(CRITICAL, "Raise Error G3 command name (%s) retrun code ",g3_cmd->cmd_name);
            for (j = 0; j < 4; j++) {
                dprintf(CRITICAL, "%02x ", res[j]);
            }
            dprintf(CRITICAL, "\n");
            return -1;
        }
        dprintf(CRITICAL, "ok..\n");
    }
    dprintf(CRITICAL, "Verifying Signature... OK!\n");
    return 0; 
}

int authentication_g3(void) {
    struct g3_command_info g3_coms[G3_AUTH_COMMAND_END] = {
        // 초기 비밀번호가 이미 설정되어 있을 경우 필요함
        // TODO 비번이 맞다면 아래 명령어가 실행될 필요 없음.
        {
            .cmd_name = "g3_initial_verifying",
            .cmd = { 0x03,0x0B,0x82,0x01,0x00,0x00,0x11,0x22,0x33,0x44,0x3D,0xDA }
        },
        // 초기 비밀번호가 설정되어 있지 않을 경우 1번 keysector 를 비밀번호용으로 설정
        {
            .cmd_name = "g3_write_setup_sector_4",
            .cmd = {
                0x03,0x27,0x81,0x04,0x00,0x00,0x0E,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x8E,0x54,
                0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x54,
                0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x10
            },
        },
        // Root sector 에 1번 keysector 가 비밀번호용으로 설정되었음을 알림
        {
            .cmd_name = "g3_write_setup_sector_2",
            .cmd = {
                0x03,0x27,0x81,0x02,0x00,0x00,0x0E,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x0E,0x54,
                0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x54,
                0x00,0x00,0x00,0x00,0x00,0x00,0xB8,0x83
            }
        },
        // 1번 keysector 에 비밀번호를 설정함. (최초 한 번만 설정하면 됨)
        {
            .cmd_name = "g3_write_pwd_sector_1",
            .cmd = {
                0x03,0x27,0x81,0x01,0x00,0x01,0x04,0x05,0x05,0x05,0x11,0x22,0x33,0x44,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x0F
            }
        },
        // verify password instruction 을 통해 1번 keysector 에 설정된 비밀번호를 검증함.
        {
            .cmd_name = "g3_verify_pwd_secotr_1",
            .cmd = {
                0x03,0x0B,0x82,0x01,0x00,0x00,0x11,0x22,0x33,0x44,0x3D,0xDA
            }
        },
        // 인증서, 공개키 저장 keysector 를 설정함.
        {
            .cmd_name = "g3_write_set_sector_5",
            .cmd = {
                0x03,0x27,0x81,0x05,0x00,0x00,0x3E,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x54,
                0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x0E,0x54,
                0x00,0x00,0x00,0x00,0x00,0x00,0x6F,0xEF
            }
        } 
    };
    unsigned char res[4];
    int i,j; 

    memset(res, 0x00, sizeof(res));

    for (i = 0; i < G3_AUTH_COMMAND_END; i++) {
        struct g3_command_info *g3_cmd = &(g3_coms[i]);
        if (write_to_puf(g3_cmd->cmd, (g3_cmd->cmd[1] + 0x01)) < 0) {
            dprintf(CRITICAL, "fail to write to g3\n");
            return -1;
        }
        udelay(340);
        WAIT_FOR_G3_RES(res, 4, g3_cmd->cmd_name);
        dprintf(CRITICAL, "g3_com name(%s), g3_com len(%u) ",
            g3_cmd->cmd_name, (g3_cmd->cmd[1] + 0x01));
        if (res[1] != 0x00) {
            if (i == G3_INITIAL_VERIFYING) {
                dprintf(CRITICAL, "intial verifying may be failed when device is new one\n");
                continue;
            }
            dprintf(CRITICAL, "Raise Error G3 command name (%s) retrun code ",g3_cmd->cmd_name);
            for (j = 0; j < 4; j++) {
                dprintf(CRITICAL, "%02x ", res[j]);
            }
            dprintf(CRITICAL, "\n");
            return -1;
        }
        dprintf(CRITICAL, "ok..\n");
    }
    dprintf(CRITICAL, "Authentication... OK!\n");
    return 0;
}

int8_t secure_boot(void *codesign_addr, unsigned char *image_hash) {

    unsigned char *ptr = (unsigned char *)codesign_addr;
    unsigned char *signature, *cert;
    struct code_sign_info *csi=NULL;
    char magic_str[9]; 

    /*
     * get codesign header
     */
    csi = (struct code_sign_info *)codesign_addr;
    memset(magic_str, 0x00, sizeof(magic_str));
    memcpy(magic_str, csi->magic, 8);
    dprintf(CRITICAL, "magic str(%s), signature_size(%llu), cert_size(%llu)\n",
            magic_str, csi->signature_size, csi->cert_size);

    if (memcmp(magic_str, "PENTASEC", 8)) {
        dprintf(CRITICAL, "not found image'signature. it may be invalid image\n");
        goto _ERROR_ACCURRED;
    }

    ptr = (unsigned char *)(csi + 1);
    signature = ptr;
    cert = ptr + csi->signature_size;

    if (check_wake_up() < 0) {
        dprintf(CRITICAL, "g3 needs to wake up\n");
        goto _ERROR_ACCURRED;
    }
    if (authentication_g3() < 0 ||
        verify_cert_g3(cert, csi->cert_size) < 0 ||
        verify_sig_g3(signature, csi->signature_size, image_hash, 32) < 0) {
        goto _ERROR_ACCURRED;
    }
    return SECURE_BOOT_OK;
_ERROR_ACCURRED:
    return SECURE_BOOT_NOK;
} 
