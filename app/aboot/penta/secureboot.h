#ifndef SECUREBOOT_H_
#define SECUREBOOT_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <platform.h>
#include <platform/timer.h>
#include <i2c.h>

#define SECURE_BOOT_OK 0
#define SECURE_BOOT_NOK -1

#define G3_MAX_COMMAND_NAME_LEN 32
#define G3_MAX_COMMAND_LEN 256
#define G3_MAX_CERT_CHUNK_LEN 240
#define G3_DEF_CMD_LEN 7 // len(1) + ins(1) + P1(1) + P2(2) + crc(2)

#define WAIT_FOR_G3_RES(buf, num, cmd_name)                                    \
    do {                                                                       \
        time_t start_time = current_time();                                    \
        while (1) {                                                            \
            if ((current_time() - start_time) > 3000) {                        \
                dprintf(CRITICAL, "(%s) no response g3\n", cmd_name);          \
                return -1;                                                     \
            }                                                                  \
            if (read_from_puf(buf, num) < 0) {                                 \
                dprintf(CRITICAL, "(%s) fail to read from g3\n", cmd_name);    \
                return -1;                                                     \
            }                                                                  \
            if (buf[0] == num) break;                                          \
        }                                                                      \
    } while (0)

struct code_sign_info
{
    uint8_t  magic[8]; 
    uint64_t signature_size;
    uint64_t cert_size;
};

struct g3_command_info
{
    char cmd_name[G3_MAX_COMMAND_NAME_LEN];
    unsigned char cmd[G3_MAX_COMMAND_LEN];
};

enum g3_auth_command_list
{
    G3_INITIAL_VERIFYING,
    G3_WRITE_SETUP_SECTOR_4,
    G3_WRITE_SETUP_SECTOR_2,
    G3_WRITE_PWD_SECTOR_1,
    G3_VERIFY_PWD_SECTOR_1,
    G3_WRITE_SET_SECTOR_5,
    G3_AUTH_COMMAND_END 
};

/*
 * 인증서의 크기가 최대 1024byte 를 지원할 수 있도록 한다.
 *  -  PART 당 240 byte 를 담을 수 있으니 240 * 5 로 계산한다.
 */
enum g3_verify_cert_command_list
{
    G3_CERTIFICATE_INS_PART_1,
    G3_CERTIFICATE_INS_PART_2,
    G3_CERTIFICATE_INS_PART_3,
    G3_CERTIFICATE_INS_PART_4,
    G3_CERTIFICATE_INS_PART_5,
    G3_VERI_CERT_COMMAND_END
};

enum g3_verify_sig_command_list
{
    G3_GET_PUB_KEY_6,
    G3_VERIFY_SIGNATURE,
    G3_VERI_SIG_COMMAND_END
};

int8_t secure_boot(void *codesign_addr, unsigned char *hash);

#endif 

