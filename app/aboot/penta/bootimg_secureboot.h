#ifndef BOOTIMG_SECUREBOOT_H_
#define BOOTIMG_SECUREBOOT_H_ 

#include "bootimg.h"
#include "sfl.h"

extern unsigned page_size;
extern unsigned page_mask;

/*
 * return 값이 0이면 성공 -1이면 실패
 */
int bootimg_secureboot(struct boot_img_hdr *hdr, unsigned offset, unsigned long long ptn); 

#endif
