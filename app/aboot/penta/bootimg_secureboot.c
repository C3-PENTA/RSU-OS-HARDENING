#include "bootimg_secureboot.h"
#include "secureboot.h"
#include "sha256.h"

#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))

int bootimg_secureboot(struct boot_img_hdr *hdr, unsigned offset, unsigned long long ptn) {
    unsigned char hash_buf[32];
    unsigned sha256_addr, kernel_actual, ramdisk_actual, size_boot_img;

	kernel_actual  = ROUND_TO_PAGE(hdr->kernel_size,  page_mask);
	ramdisk_actual = ROUND_TO_PAGE(hdr->ramdisk_size, page_mask);

    /*
     * read code sign image from falsh for secure boot
     * 1. 시큐어부트를 할 것인지 선택할 수 있는 기능추가 필요함.
     * 2. codesign 이미지 데이터가 2048 page 크기보다 클 때
     *    추가 사이즈를 메모리에 로드하는 코드가 필요하다.
     */
    if (tcc_read(ptn + offset, (void *)(hdr->ramdisk_addr + ramdisk_actual), page_size)) {
        dprintf(CRITICAL, "ERROR: Cannot read codesign image\n");
        return -1;
    } 

    size_boot_img = page_size + kernel_actual + ramdisk_actual;
    sha256_addr = hdr->ramdisk_addr + ramdisk_actual + page_size;

    if (tcc_read(ptn, (void *)sha256_addr, size_boot_img)) {
        dprintf(CRITICAL, "ERROR: Cannot read boot image header\n");
        return -1;
    }
    if (!SHA256_hash((void *)sha256_addr, size_boot_img, hash_buf)) {
        dprintf(CRITICAL, "ERROR: Couldn't make sha256 hash from image header\n");
        return -1;
    }
    if (secure_boot((void *)(hdr->ramdisk_addr + ramdisk_actual), hash_buf) != SECURE_BOOT_OK) { 
        dprintf(CRITICAL, "ERROR: Failing secure boot!\n");
        return -1;
    }
    return 0;
}

