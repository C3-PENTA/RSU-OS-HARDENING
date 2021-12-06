/* Copyright (c) 2010-2011, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <debug.h>
#include <arch/arm.h>
#include <dev/udc.h>
#include <string.h>
#include <kernel/thread.h>
#include <arch/ops.h>

#include <dev/flash.h>
#include <lib/ptable.h>
#include <dev/keys.h>
#include <platform.h>
#include <partition_parser.h>
#include <mmc.h>
#include <sfl.h>

#include "recovery.h"
#include "bootimg.h"
#include "smem.h"

#define BOOT_FLAGS	1
#define UPDATE_STATUS	2
#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))

static const int MISC_PAGES = 3;			// number of pages to save
static const int MISC_COMMAND_PAGE = 1;		// bootloader command is this page
static char buf[4096];
unsigned boot_into_recovery = 0;

int set_recovery_message(struct recovery_message *out)
{
  char *ptn_name = "misc";
  unsigned long long ptn = 0;
  unsigned int size = ROUND_TO_PAGE(sizeof(*out),511);
  unsigned char data[size];
  unsigned int ptn_index;

  ptn_index = partition_get_index(ptn_name);
  ptn = partition_get_offset(ptn_index);
  if(ptn == 0) {
    dprintf(CRITICAL,"partition %s doesn't exist\n",ptn_name);
    return -1;
  }

  memcpy(data, out, sizeof(*out));

  if (tcc_write(ptn_name, ptn, size, (unsigned int*)data))
  {
    dprintf(CRITICAL,"write failure %s %d\n",ptn_name, sizeof(*out));
    return -1;
  }
  return 0;
}

int get_recovery_message(struct recovery_message *in)
{
  char *ptn_name = "misc";
  unsigned long long ptn = 0;
  unsigned int size = ROUND_TO_PAGE(sizeof(*in),511);
  unsigned char data[4096];
  unsigned int ptn_index;

  ptn_index = partition_get_index(ptn_name);
  ptn = partition_get_offset(ptn_index);
  if(ptn <= 0) {
    dprintf(CRITICAL,"partition %s doesn't exist\n",ptn_name);
    return -1;
  }

  if (tcc_read(ptn, data, size))
  {
    dprintf(CRITICAL,"read failure %s %d\n",ptn_name, size);
    return -1;
  }

  memcpy(in, data, sizeof(*in));
  return 0;
}

int update_firmware_image(struct update_header *header, char *name)
{
  char *ptn_name = "cache";
  unsigned int offset = 0;
  unsigned int pagesize = flash_page_size();
  unsigned int pagemask = pagesize -1;
  unsigned int n = 0;
  unsigned long long ptn = 0;
  unsigned int ret;
  unsigned int ptn_index;

  ptn_index = partition_get_index(ptn_name);
  ptn = partition_get_offset(ptn_index);
  if (ptn == 0) {
    dprintf(CRITICAL, "ERROR: No cache partition found\n");
    return -1;
  }

  offset += header->image_offset;
#if _EMMC_BOOT
  n = header->image_length;
#else
  char dummy_serial[4096]; n = NAND_GetSerialNumber(dummy_serial, 32); n = 0;
  n = (header->image_length + pagemask) & (~pagemask);
#endif

  if (tcc_read(ptn, SCRATCH_ADDR, n+offset))
  {
    dprintf(CRITICAL, "ERROR: Cannot read bootloader image\n");
    return -1;
  }

  memcpy(SCRATCH_ADDR,SCRATCH_ADDR+offset, n);

  if (!strcmp(name, "bootloader"))
  {
    dprintf(ALWAYS, "\nBootloader Update Start !!! \n");
    ret = tcc_write(name,
	ptn, /* Important!!
		This parameter is NOT actually used inside flash_write_tnftl_v8 function.
		Because of parameter size problem, ptn argument is set as dummy parameter.
		*/
	n,
	SCRATCH_ADDR);
    if(ret != 0)
    {
      dprintf(ALWAYS, "\nBootloader Update Fail [ret:0x%08X]!!! \n", ret);
      return ret;
    }
    else
      dprintf(ALWAYS, "\nBootloader Update Success !!! \n");

  }
  dprintf(INFO, "Partition writen successfully!");
  return 0;
}

int read_update_header_for_bootloader(struct update_header *header)
{
  char *ptn_name = "cache";
  unsigned long long ptn = 0;
  unsigned int pagesize = 512;
  unsigned int ptn_index;

  ptn_index = partition_get_index(ptn_name);
  ptn = partition_get_offset(ptn_index);
  if (ptn == 0) {
    dprintf(CRITICAL, "ERROR: No cache partition found\n");
    return -1;
  }

  if (tcc_read(ptn, buf, pagesize))
  {
    dprintf(CRITICAL, "ERROR: Cannot read recovery_header\n");
    return -1;
  }
  memcpy(header, buf, sizeof(*header));

  if(strncmp(header->MAGIC, UPDATE_MAGIC, UPDATE_MAGIC_SIZE))
    return -1;
  return 0;
}

//+[TCCQB] QuickBoot System Updete by Recovery Mode
extern unsigned skip_loading_quickboot;
//-[TCCQB]
//
int recovery_init(void)
{
  int update_status = 0;
  struct recovery_message msg;
  struct update_header header;
  unsigned int valid_command  = 0;

  if(get_recovery_message(&msg))
    return -1;
  if (msg.command[0] != 0 && msg.command[0] != 255) {
//+[TCCQB] QuickBoot System Updete by Recovery Mode
    dprintf(INFO,"Recovery command:%s\n",msg.command);
//-[TCCQB]
//
  }
  msg.command[sizeof(msg.command)-1] = '\0';

#if (defined(TARGET_BOARD_TSVA) || defined(TARGET_BOARD_J87K))
	if (!strcmp("boot-recovery",msg.command))
	{
		boot_into_recovery = 1;
		return 0;
	}
#else
  if (!strcmp("boot-recovery",msg.command)) {
    valid_command = 1;
    strcpy(msg.command, "");
    strcpy(msg.status, "OKAY");
    set_recovery_message(&msg);
    boot_into_recovery = 1;
    return 0;
  }
#endif

//+[TCCQB] QuickBoot System Updete by Recovery Mode
	if (!strcmp("boot-force_normal",msg.command)) {
		printf("Enter normal boot mode.\n");
		skip_loading_quickboot = 1;
		return 0;
	}
//-[TCCQB]
//

#if (!defined(TARGET_BOARD_TSVA) && !defined(TARGET_BOARD_J87K))
  if (!strcmp("update-bootloader",msg.command))
  {
    read_update_header_for_bootloader(&header);
    if(update_firmware_image(&header , "bootloader") == -1)
      return -1;
  }
  else
    return 0;

  strcpy(msg.command, "boot-recovery");
  set_recovery_message(&msg);
  boot_into_recovery = 1;
  reboot_device(0);
#endif
  return 0;
}
