//+[TCCQB] QuickBoot Image Loading
/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/****************************************************************
  Snapshot Partition is needed 4byte ( PAGE ) Align.
 *****************************************************************/

#include <app.h>
#include <debug.h>
#include <arch/arm.h>
#include <string.h>
#include <kernel/thread.h>
#include <arch/ops.h>
#include <platform.h>
#include <arch/tcc_used_mem.h>      // add by Telechips
#include <platform/reg_physical.h>
#include <partition_parser.h>

#include "qb_watchdog.h"	// for qb watchdog
#include "snapshot_mem.h"	// for snapshot lib ( jump to kernel )

#ifdef CONFIG_ARM_TRUSTZONE
#include <platform/smc.h>
extern int lk_in_normalworld;
#endif


/*========================================================
  = For TEST
  ========================================================*/
//#define NO_BUFFER	// Do not use gpDataBuf ( Storage Read Buffer ). - Buf, gpDataBuf Area is needed.
                    // It Read snapshot partition whenever read_data() is called.
//#define ERROR_STOP	// Stop Booting if some Error is Occured when Loading QuickBoot Image.
  						// It Doesn't Stop NO QB_SIG.
//#define USE_MULTI_PAGE_LOAD	// Support Multi Page Read at read_data( ), load_data_to_buffer( ).
 								// Usually read_data( ), load_data_to_buffer( ) - work for only 1 PAGE Data.
  								// Do not use this option. It makes QB Loading slow.
//#define NO_COMPRESSION		// For No compressed snapshot image. Should be enabled the option below.
  								// Kernel : Power management options -> [*] No compressed snapshot image
//#define CHECK_LOADED_QB_DATA	// Check QB Image Data which is Copyed to Original DRAM Area.
								// It except MetaData Loading. CheckSum may fail if Meta is damaged.
								// It makes QB Booting slow.


/*========================================================
  = definition
  ========================================================*/
#define ROUND_TO_PAGE(x,y)		(((x) + (y)) & (~(y)))
#define BYTE_2_SECTOR(x)		((x + 511) >> 9) 	//  (x)/512 - ROUND UP
#define BYTE_2_MB(x)			(x + (1024*1024 -1) >> 20)	// x/(1024*1024) - ROUND UP
#define PAGE_2_MB(x)			((x << 2) + (1024-1) >> 10) // x*4/1024 - ROUND UP
#define PAGE_SIZE				4096
#define SECTOR_SIZE				512
#define MAP_PAGE_ENTRIES		(PAGE_SIZE / sizeof(page_ofs) - 1)	// the number of entries are 511.
#define addr(x) (0xF0000000+x)

#define IO_OFFSET	0x00000000
#define io_p2v(pa)	((pa) - IO_OFFSET)
#define tcc_p2v(pa)         io_p2v(pa)

/*		QuickBoot SIG							*/
#define SWSUSP_SIG				"S1SUSPEND"
#define QB_SIG_SIZE			10
#define QB_SIG_SIZE_ALIGN	(((QB_SIG_SIZE+3)/4)*4)			// DIVIDE by 4 and ROUND UP - 4 byte align

/*		QuickBoot Image Loading Error Value		*/
#define NO_ERROR	0		// No Error.
#define UNKNOWN		-1		// Unknown Error.
#define PTN_TABLE	-2		// Error to load Snapshot Partition Table.
#define QB_LOADING	-3		// QB Image Loading Error while loading image.
#define QB_ERASE	-4		// Erase QuickBoot Image is failed.
#define HEADER_SIG	-5		// QB Image Header is invalid.
#define HEADER_READ	-6		// QB Snapshot Header Read fail. ( from Storage )


#if defined(TSBM_ENABLE)
#define TSQB_SECURE_SIG		"TSQB_SECURE_SIG"	// 16byte
#define TSQB_SWSUSP_SIG		"TSQB_SUSP"			// 10byte
#define TSQB_HDR_SIZE		(428)
#define TSQB_ENC_SIZE		(164+TSQB_HDR_SIZE+4)
#define TSQB_TBUF_SIZE		(512)
#endif


/*===================================================================
 *		 			QB_SCRATCH Buffer Memory Map.					*
 *=================================================================*/
#define SNAP_BUF_SIZE			0x00080000	// 512 kB
#define SNAP_UNC_SIZE			0x00040000	// 256 kB
#define SNAP_CMP_SIZE			0x00040000	// 256 kB
#define SNAP_META_SIZE			0x00080000	// 512 kB
//TCC_SNAPSHOT_SIZE				0x1000 * 7	// 28 kb
//#define SNAP_INFO_SIZE			0x00001000	// 4 kB(1PAGE)	- NO SCRATCH AREA

/* We need to remember how much compressed data we need to read. */
#define LZ4_HEADER	sizeof(uint32_t) * 2

/* Number of pages/bytes we'll compress at one time. */
#define LZ4_UNC_PAGES	32
#define LZ4_UNC_SIZE	(LZ4_UNC_PAGES * PAGE_SIZE)

static void *gpDataBuf = NULL;
static void *gpDataUnc = NULL;
static void *gpDataCmp = NULL;
static void *gpDataMeta = NULL;
static struct tcc_snapshot  *gpSnapshot = NULL;



/*========================================================
  = Variable 
  ========================================================*/
typedef uint64_t page_ofs;		// Storage Address (Page Offset)
#ifndef CONFIG_ARM_TRUSTZONE
typedef uint32_t u32;
#endif

struct swap_map_page {
	page_ofs entries[MAP_PAGE_ENTRIES];
	page_ofs next_swap;
};

struct swsusp_header {
	char		secure_sig[16];
	u32			master_checksum;
	page_ofs	image;		
	uint32_t	flags;	/* Flags to pass to the "boot" kernel */	
	uint32_t	reg[64];	
	char 		reserved[PAGE_SIZE - sizeof(char)*16 - sizeof(u32) - sizeof(page_ofs)
					- sizeof(unsigned int) - sizeof(unsigned int)*64 - sizeof(char)*10*2];
	char		orig_sig[10];
	char		sig[10];		
}__attribute__((packed));

struct new_utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
};

struct swsusp_info {
	struct new_utsname	uts;
	uint32_t			version_code;
	unsigned long		num_physpages;
	int					cpus;
	unsigned long 		image_pages;
	unsigned long 		pages;
	unsigned long 		size;
} __attribute__((aligned(PAGE_SIZE)));

static struct swsusp_header		*gpSwsusp_header;
static struct swsusp_info		*gpSwsusp_info;
static struct swap_map_page		*gpSwap_map_page;
static uint32_t					*gpMeta_page_info;

static uint32_t			gswap_map_idx = 0;	// index of swap_map_page->entries.
static unsigned int		gnr_meta_pages = 0;
static unsigned int		gnr_copy_pages = 0;

static unsigned int		storage_read_size = 0;
static unsigned int		sizeQBImagePage = 0;	// QuickBoot Image size ( Page unit )

static time_t			time_qb_load = 0, time_total = 0, time_start = 0, time_storage = 0;
static time_t			time_memcpy = 0, time_lz4 = 0;
#if defined(PLATFORM_TCC893X)
static time_t	time_chipboot = 600;
#elif defined(PLATFORM_TCC896X)
static time_t	time_chipboot = 800;
#elif defined(PLATFORM_TCC897X)
static time_t	time_chipboot = 700;
#endif

struct tcc_snapshot {
	unsigned char	swsusp_header[2*PAGE_SIZE];
	unsigned char	swsusp_info[2*PAGE_SIZE];
	unsigned char	swap_map_page[2*PAGE_SIZE];
	unsigned char	check_page_buf[PAGE_SIZE];
};

static unsigned long long	gswap_start;	// snapshot partition start address (byte) in whole Storage.
static unsigned long long	gswap_end;		// snapshot partition end address (byte) in whole Storage.
static unsigned long long	gswap_size;		// snapshot partition size (byte).

static page_ofs buf_swap_paddr = 0;			// Storage Address about the data in buffer. ( in snapshot page offset )
static page_ofs buf_swap_pend = 0;			// The end of Storage Address about the data in buffer. ( in snapshot page offset )

static char check_sig[QB_SIG_SIZE_ALIGN];	// QB SIG loading Buffer

/*==========================================================================
  = externs
  ===========================================================================*/
extern void tcc_memcpy(void *dest, void *src);
extern int LZ4_uncompress_unknownOutputSize(const char* source,	char* dest, int isize, int maxOutputSize);
extern unsigned erase_quickboot;


/*===================================================================
 *		 			Loading Functions from Storage					*
 *=================================================================*/
/*		Support Time Check about tcc_memcpy()		*/
static inline void tcc_memcpy_tm(void *dest, void *src)
{
	time_t temp = current_time();

	tcc_memcpy(dest, src);

	time_memcpy += current_time() - temp;
}

static inline int unLZ4_tm(const char* source,	char* dest, int isize, int maxOutputSize)
{
	time_t temp = current_time();
	int ret;

	ret = LZ4_uncompress_unknownOutputSize(source, dest, isize, maxOutputSize);

	time_lz4 += current_time() - temp;
	return ret;
}

/*		Check QuickBoot SIG is correct or not.		*/
static int check_snapshot_sig(void)
{
	int ret = -1;

	/*		to Check QB SIG if QB SIG length is not a multiple of 4.	
	 *		memcpy() checks data by 4 byte unit.						
	 *		So, it is changed to strncpy() and it needs null data the end of string.	*/
	memset(check_sig, 0, 12);
	memcpy(check_sig, gpSwsusp_header->sig, 10);

#if defined(TSBM_ENABLE) && defined(_EMMC_BOOT)
if (target_use_signed_kernel()) {
	dprintf(INFO, "secure quicboot start ~~ \n");

	if(!strncmp(TSQB_SWSUSP_SIG, check_sig, strlen(TSQB_SWSUSP_SIG)))
	{   
		unsigned char pTmpBuf[TSQB_TBUF_SIZE] = {0,};
		unsigned char *m_pTmpBuf = &pTmpBuf[64];
		
		if(tcsb_api_check_secureboot(0, NULL, NULL) == 0)
		{   
			dprintf(CRITICAL , "ERROR : it does not support secure boot\n");
		}   
		else if(tcsb_api_decrypt((void *)&gpSwsusp_header->secure_sig[0], (void *)&m_pTmpBuf[0], TSQB_ENC_SIZE) != NULL)
		{   

			if(!memcmp(TSQB_SECURE_SIG, &m_pTmpBuf[0], 16))
			{   
				memcpy(gpSwsusp_header, &m_pTmpBuf[0], TSQB_ENC_SIZE);
				ret = 0;

#ifdef CONFIG_ARM_TRUSTZONE
				if (lk_in_normalworld)
					_tz_smc(SMC_CMD_BOOTUP, 0, 0, 0);
#endif
			}   
			else
				dprintf(CRITICAL, "ERROR : Secure QuickBoot Header(%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)\n",
						m_pTmpBuf[0], m_pTmpBuf[1], m_pTmpBuf[2], m_pTmpBuf[3], m_pTmpBuf[4], m_pTmpBuf[5], m_pTmpBuf[6], m_pTmpBuf[7]);
		}   
		else
			dprintf(CRITICAL, "ERROR : Secure QuickBoot Image Decrypt(%d:%c%c%c%02x)\n", TSQB_ENC_SIZE,
					gpSwsusp_header->secure_sig[0], gpSwsusp_header->secure_sig[1], 
					gpSwsusp_header->secure_sig[2], gpSwsusp_header->secure_sig[3]);
	}   
	else
		dprintf(CRITICAL, "ERROR : Secure QuickBoot Signature\n");

	return ret;
}
#endif
	/*	compare lenght is depending on 'SWSUSP_SIG'.
	 *	Becouse Trash data can be attached at the tail of 'SWSUSP_SIG' by compiler,
	 *	If it's length is not a multiple of 4 byte.								*/
	ret = strncmp(SWSUSP_SIG, check_sig, strlen(SWSUSP_SIG));

	return ret;		// if(ret == 0) : OK, else : Failed.
}

/*			Read Swap Header 			*/
static int check_swap_header(page_ofs read_paddr)	// page
{
	if (tcc_read(gswap_start, gpSwsusp_header, PAGE_SIZE) != 0) {
//		dprintf(INFO, "ERROR: snapshot header read fail...\n");
		return HEADER_READ;
	}

	if (check_snapshot_sig()) {
//		dprintf(CRITICAL, "ERROR: snapshot header sig compare fail.\n");
		return HEADER_SIG;
	}

	return NO_ERROR;
}

/*		Read Swap Data from Storage Snapshot Partition to gpDataBuf		*/
static inline int load_data_to_buffer(page_ofs read_paddr)	// page
{
	int remain_psize = 0;	// page
	int idx = 0;	// page
	int data_psize = SNAP_BUF_SIZE/PAGE_SIZE;	// page
	void *src = NULL;	// byte
	void *dest = gpDataBuf;	// byte

#ifdef USE_MULTI_PAGE_LOAD
	// move remained buffer data to Buffer Start Address.
	if (unlikely(read_paddr <= buf_swap_pend && read_paddr >= buf_swap_paddr && buf_swap_pend != 0)) {
		remain_psize = SNAP_BUF_SIZE/PAGE_SIZE - (read_paddr - buf_swap_paddr);
		data_psize -= remain_psize;
		src = (void *)(gpDataBuf + (read_paddr - buf_swap_paddr)*PAGE_SIZE);
		while(likely(idx < remain_psize)) {
			tcc_memcpy_tm(dest, src);
			src += PAGE_SIZE;
			dest += PAGE_SIZE;
			idx++;
		}
	}
#endif

	if (unlikely(((read_paddr + remain_psize + data_psize)*PAGE_SIZE + gswap_start) > gswap_end))
		data_psize = (gswap_end-gswap_start)/PAGE_SIZE - (read_paddr + remain_psize);	// Shrink data_size by the end of storage.

	// read next data from storage.
	if (unlikely(tcc_read((read_paddr+remain_psize)*PAGE_SIZE + gswap_start, dest, data_psize*PAGE_SIZE) != 0)) {
		dprintf(CRITICAL, "ERROR: failed to read data from storage... Storage_read_addr:%llu,  data_size:%d\n", (read_paddr+remain_psize)*PAGE_SIZE, data_psize);
		buf_swap_paddr = 0;
		buf_swap_pend = 0;
		return -1;
	}
	storage_read_size += data_psize*PAGE_SIZE;		// for debug

	buf_swap_paddr = read_paddr;
	buf_swap_pend = buf_swap_paddr + data_psize + remain_psize;

	return 0;
}

/*			Read Data from Buffer 			*/
static inline int read_data(page_ofs read_paddr, void *dest, int req_psize)	// page, byte, page
{
	time_t 	temp = current_time();
	void 	*src = NULL;
	page_ofs end_paddr = read_paddr + (page_ofs)req_psize;

	// Check if Buffer has enough data.
	if (unlikely(end_paddr < buf_swap_paddr || end_paddr > buf_swap_pend ||
			read_paddr < buf_swap_paddr || read_paddr > buf_swap_pend ||
			buf_swap_paddr == buf_swap_pend)) {
		// Requested data is not in Buffer. Load Data From Storage.
		load_data_to_buffer(read_paddr);
	}

	// All requested data is in Buffer. Copy it to dest.
	src = (void *)(gpDataBuf + (read_paddr - buf_swap_paddr)*PAGE_SIZE);
#ifdef USE_MULTI_PAGE_LOAD
	while(likely(req_psize)) {
		tcc_memcpy_tm(dest, src);
		src += PAGE_SIZE;
		req_psize--;
	}
#else
	tcc_memcpy_tm(dest, src);
#endif

#ifdef NO_BUFFER	// For TEST. It's not enabled in common.
	buf_swap_paddr = 0;
	buf_swap_pend = 0;
#endif

	time_storage += current_time() - temp;

	return 0;
}

/*	Load PAGE_MAP which is indicate 1Page data in snapshot Partition	*/
static inline void get_next_swap_page_info() 
{
	if (unlikely(gswap_map_idx >= MAP_PAGE_ENTRIES)) {
		read_data(gpSwap_map_page->next_swap, gpSwap_map_page, 1);
		gswap_map_idx = 0;
	}
}

/*	Load Data from snapshot partiton to Memory ( *out ). It use Buffer	*/
static void get_next_swap_data(void *out, unsigned int req_page) 
{
	unsigned int index;
	void *dest;

	dest = out;

	for (index = 0; index < req_page; index++) {
		// update swap map page
		get_next_swap_page_info();

		read_data(gpSwap_map_page->entries[gswap_map_idx], dest, 1);
		dest += PAGE_SIZE;
		gswap_map_idx++;
	}

	sizeQBImagePage += req_page;
}


static inline u32 checksum(u32 *addr, int len)
{
	u32 csum = 0;

	len /= sizeof(*addr);
	while (len-- > 0)
		csum ^= *addr++;
	csum = ((csum>>1) & 0x55555555)  ^  (csum & 0x55555555);

	return csum;
}

/*	Load QuickBoot Data from snapshot partition to Memory & restore Memory	*/
static int storage_load_snapshot_image()
{
	unsigned int	index, one_percent_pages, next_percent_pages, percent = 0;
	unsigned int	loop, max_copy_pages;
	uint32_t		cmp_data_size;
	uint32_t		unc_data_size;
	uint32_t		unc_qb_size;
	uint32_t		cmp_page_num;
	uint32_t		copy_page_num;
	u32				saved_checksum_data;
	u32				cal_checksum_data;
	u32				cal_master_checksum = 0;
	void			*dest, *src;
	uint32_t		meta_page_offset;
	time_t			time_load_start;

	watchdog_init();	// Initiatlize & Start WatchDog.

	time_load_start = current_time();


	/*===================================================================
	 *																	*
	 *		 			Load Swap_Info from Storage						*
	 *																	*
	 *=================================================================*/
	// load swap map page info.
	read_data(gpSwsusp_header->image, gpSnapshot->swap_map_page, 1);	// get swap map page

	// load swsusp_info
	get_next_swap_data((void *)gpSwsusp_info, 1);

	gnr_meta_pages = gpSwsusp_info->pages - gpSwsusp_info->image_pages - 1;
	gnr_copy_pages = gpSwsusp_info->image_pages;
	copy_page_num = gnr_copy_pages;

	// load meta data 
	max_copy_pages = 0;


	/*===================================================================
	 *																	*
	 *		 			Load MetaData from Storage						*
	 *																	*
	 *=================================================================*/
	while (max_copy_pages < gnr_meta_pages) {
#ifdef NO_COMPRESSION
		unc_data_size = LZ4_UNC_PAGES*PAGE_SIZE;
		get_next_swap_data(gpDataMeta + max_copy_pages * PAGE_SIZE, unc_data_size/PAGE_SIZE);
#else
		// get meta data
		get_next_swap_data(gpDataCmp, 1);

		// uncompress data
		cmp_data_size = *(uint32_t *)gpDataCmp;	// 4byte
		saved_checksum_data = *(u32 *)(gpDataCmp + 4) ; // 4byte

		cmp_page_num = (LZ4_HEADER + cmp_data_size + PAGE_SIZE - 1) / PAGE_SIZE;

		get_next_swap_data(gpDataCmp + PAGE_SIZE, (cmp_page_num - 1));

		unc_data_size = unLZ4_tm(gpDataCmp + LZ4_HEADER,
				(gpDataMeta + max_copy_pages * PAGE_SIZE), cmp_data_size, LZ4_UNC_SIZE);
		if (unlikely(unc_data_size <= 0 || unc_data_size > LZ4_UNC_SIZE)) {
			dprintf(CRITICAL, " 01. LZ4 Decompression failed. UNC_LEN:[%u] CMP_LEN:[0x%u]\n", unc_data_size, cmp_data_size);
			return -1;
		}

	#ifdef CONFIG_SNAPSHOT_CHECKSUM
		cal_checksum_data = checksum((gpDataMeta + max_copy_pages * PAGE_SIZE), unc_data_size);

		if(unlikely(cal_checksum_data != saved_checksum_data)) {
			dprintf(CRITICAL, " 01. LZ4 Decompression Checksum check failed. CAL:[%x] SAVED:[%x]\n", cal_checksum_data, saved_checksum_data);
			hexdump((gpDataMeta + max_copy_pages * PAGE_SIZE), 512 );
			return -1;
		}

		/*	Master Checksum	*/
		cal_master_checksum = cal_master_checksum ^ cal_checksum_data;
	#endif	// CONFIG_SNAPSHOT_CHECKSUM
#endif	// NO_COMPRESSION

		// copy swap page data.
		max_copy_pages += (unc_data_size / PAGE_SIZE);
	}

	meta_page_offset = 0;
	copy_page_num = copy_page_num - (max_copy_pages - gnr_meta_pages);
	unc_qb_size = max_copy_pages - gnr_meta_pages;

	for (index = gnr_meta_pages;index < max_copy_pages;index++) {
		tcc_memcpy_tm((void *)(gpMeta_page_info[meta_page_offset++]*PAGE_SIZE), (void *)(gpDataMeta + index*PAGE_SIZE));
	}
//	dprintf(CRITICAL, "load_image_from_storage.... meta[%6u] copy[%6u]\n", gnr_meta_pages, gnr_copy_pages);


	/*===================================================================
	 *																	*
	 *		 			Load Snapshot Data from Storage					*
	 *																	*
	 *=================================================================*/
	one_percent_pages = copy_page_num/100;	// which pages num is one percent.
	next_percent_pages = one_percent_pages;
	for (index = 0;index < copy_page_num;) {
#ifdef NO_COMPRESSION
		if (copy_page_num > (index + LZ4_UNC_PAGES))
			unc_data_size = LZ4_UNC_PAGES*PAGE_SIZE;
		else
			unc_data_size = (copy_page_num - index)*PAGE_SIZE;

		get_next_swap_data(gpDataUnc, unc_data_size/PAGE_SIZE);
#else
		// get meta data
		get_next_swap_data(gpDataCmp, 1);

		// uncompress data
		unc_data_size = LZ4_UNC_SIZE;
		cmp_data_size = *(uint32_t *)gpDataCmp;
		saved_checksum_data = *(u32 *)(gpDataCmp + 4) ;


		cmp_page_num  = (LZ4_HEADER + cmp_data_size + PAGE_SIZE - 1) / PAGE_SIZE;

		get_next_swap_data(gpDataCmp + PAGE_SIZE, (cmp_page_num - 1));

		unc_data_size = unLZ4_tm(gpDataCmp + LZ4_HEADER, gpDataUnc, cmp_data_size, LZ4_UNC_SIZE);
		if (unlikely(unc_data_size <= 0 || unc_data_size > LZ4_UNC_SIZE)) {
			dprintf(CRITICAL, " 02. LZ4 Decompression failed. UNC_LEN:[%u] CMP_LEN:[%u]\n", unc_data_size, cmp_data_size);
			return -1;
		}
		unc_qb_size += unc_data_size;

	#ifdef CONFIG_SNAPSHOT_CHECKSUM
		cal_checksum_data = checksum(gpDataUnc, unc_data_size );

		if(unlikely(cal_checksum_data != saved_checksum_data )) {
			dprintf(CRITICAL, " 02. LZ4 Decompression Checksum check failed. CAL:[%x] SAVED:[%x]\n", cal_checksum_data, saved_checksum_data);
			hexdump(gpDataUnc, 512 );
			return -1;
		}

		/*	Master Checksum	*/
		cal_master_checksum = cal_master_checksum ^ cal_checksum_data;
	#endif	// CONFIG_SNAPSHOT_CHECKSUM
#endif	// NO_COMPRESSION

		max_copy_pages = unc_data_size / PAGE_SIZE;

		/*		Copy UNC data to Original position on the Memory.	*/
		src = gpDataUnc;
		for (loop = 0;loop < max_copy_pages;loop++) {
			dest = (void *)(gpMeta_page_info[meta_page_offset++]*PAGE_SIZE);
			//memcpy(dest, src, PAGE_SIZE);
			tcc_memcpy_tm(dest, src);

#if defined(CHECK_LOADED_QB_DATA)
			/*		Check Coped Page Data Validation		*/
			tcc_memcpy(gpSnapshot->check_page_buf, dest);	// Copy to Cached Buffer.
			if(unlikely(memcmp(gpSnapshot->check_page_buf, src, PAGE_SIZE))) {
				dprintf(CRITICAL, " QB Image Page Copy is failed. dest[Addr:%#x]\n", dest);
				dprintf(CRITICAL, "\n SRC DATA [Addr:%#x] =============================\n", src);
				hexdump(src, PAGE_SIZE );
				dprintf(CRITICAL, "\n DEST DATA[Addr:%#x] =============================\n", dest);
				hexdump(dest, PAGE_SIZE );
				return -1;
			}
#endif	// CHECK_LOADED_QB_DATA

			src += PAGE_SIZE;
		}

		index += max_copy_pages;

		if (unlikely(index > next_percent_pages)) {
			next_percent_pages += one_percent_pages;
			dprintf(CRITICAL, " load %3u % \r", ++percent);
			watchdog_clear();   // Clear WatchDog.
		}
	}

#ifndef NO_COMPRESSION
	#ifdef CONFIG_SNAPSHOT_CHECKSUM
	/*		Check Master Checksum	*/
	if (cal_master_checksum != gpSwsusp_header->master_checksum) {
		dprintf(CRITICAL,"\x1b[1;31mERROR Invalid Master Checksum! orig[0x%08x] cal[0x%08x]\x1b[0m\n",
				gpSwsusp_header->master_checksum, cal_master_checksum);
		return -1;
	}
	#endif
#endif	// NO_COMPRESSION

	dprintf(CRITICAL, "\n");
	dprintf(CRITICAL, " load Done. \n");
	dprintf(CRITICAL, "load_image_from_storage.... last_unc_data_size[%lu byte]\n", unc_data_size);
	dprintf(CRITICAL, "gnr copy pages[%6u] gnr meta pages[%6u] gnr data pages[%6u]\n",
			gnr_copy_pages, gnr_meta_pages, gnr_copy_pages - gnr_meta_pages);
	dprintf(CRITICAL, "read data from storage[%uMB] / CMP QB Image[%dMB] / UNC QB Image[%dMB]\n",
			BYTE_2_MB(storage_read_size), PAGE_2_MB(sizeQBImagePage), BYTE_2_MB(unc_qb_size));

	time_qb_load = time_memcpy + time_lz4 + time_storage;
	time_total = current_time() - time_load_start;
	dprintf(CRITICAL, "\x1b[1;32mQB_Total[%4lums] = mmemcpy[%4lums] + lz4[%4lums] + "
			"load_storage[%4lums][%uMBps] + etc[%4lums]\x1b[0m\n",
			time_total, time_memcpy, time_lz4, time_storage,
			storage_read_size/1024/time_storage, time_total - time_qb_load);


	return 0;
}

static int snapshot_boot()
{
	int ret = 0;

	dprintf(CRITICAL, "snapshot_boot()...\n");

	/*===================================================================
	 *																	*
	 *		 			Load MetaData from Storage						*
	 *																	*
	 *=================================================================*/

	// swap map page
	gswap_map_idx = 0;

	// swsusp info
	gnr_copy_pages = 0;
	gnr_meta_pages = 0;

	gpSwsusp_info	 = (struct swsusp_info   *)gpSnapshot->swsusp_info;
	gpSwap_map_page	 = (struct swap_map_page *)gpSnapshot->swap_map_page;
	gpMeta_page_info = gpDataMeta;

	// Load Image
	sizeQBImagePage = 1;	// QuickBoot Image Size ( Page unit ). HEADER = 1 page.
	ret = storage_load_snapshot_image();

	return ret;
}

extern void set_lk_boottime(unsigned int usts);	// Send LK boot time to kernel by PMU_USSTATUS reg.
extern int jump_to_kernel(unsigned int *header_data, unsigned int arch);	// excure kernel jump code in sram.
static int storage_snapshot_jump_to_resume(unsigned int *regs)
{
	watchdog_clear();	// Clear WatchDog.

	enter_critical_section();
	platform_uninit_timer();

#ifdef LCDC0_USE
#define LCDC_NUM        0
#else
#if defined(DEFAULT_DISPLAY_LCD)
#define LCDC_NUM        1
#elif defined(TARGET_TCC8930ST_EVM)
#define LCDC_NUM        0
#elif defined(TARGET_TCC8960ST_EVM)
#define LCDC_NUM        0
#else
#define LCDC_NUM        1
#endif
#endif

	DEV_LCDC_Wait_signal(LCDC_NUM);	
	arch_disable_cache(UCACHE);
#ifdef CONFIG_CACHE_PL310
	pl310_cache_clean_all();
	//   pl310_cache_flush_all();
#endif
#if ARM_WITH_MMU
	arch_disable_mmu();
#endif

	time_t lk_total = time_start + time_total + time_chipboot;
	dprintf(CRITICAL, "\x1b[1;32mLK BootTime: LK_Total[%4lums] = LK_init[%4lums] + QB Image load [%4lums] + chip boot[%4lums] \x1b[0m\n",
		lk_total, time_start, time_total, time_chipboot);

	/*       Send LK Total Time to Kernel    */
	set_lk_boottime((unsigned int)lk_total);

	watchdog_clear();	// Clear WatchDog.

#if defined(PLATFORM_TCC893X)
	dprintf(CRITICAL, "Jump to libsnapshot. ( Arch : Tcc893x )\n");
	return jump_to_kernel(regs, 0x04);	// drop by for restoring some register & mmu.
#elif defined(PLATFORM_TCC896X)
	dprintf(CRITICAL, "Jump to libsnapshot. ( Arch : Tcc896x )\n");
	return jump_to_kernel(regs, 0x08);	// drop by for restoring some register & mmu.
#elif defined(PLATFORM_TCC897X)
	dprintf(CRITICAL, "Jump to libsnapshot. ( Arch : Tcc897x )\n");
	return jump_to_kernel(regs, 0x09);	// drop by for restoring some register & mmu.
#endif
}

void show_snapshot_area()
{
	/*	Show Snapshot Memory Buffer Area */
	dprintf(CRITICAL, "gpDataBuf[%p] gpDataUnc[%p] gpDataCmp[%p] gpSnapshot[%p]\n", gpDataBuf, gpDataUnc, gpDataCmp, gpSnapshot);
	
	/*	Show Snapshot Partition Area */
	dprintf(CRITICAL, "gswap_start[%llu] gswap_end[%llu] gswap_size[%llu]\n", gswap_start, gswap_end, gswap_size);
}

int init_snapshot()
{
	char *ptn_name = "snapshot";
	unsigned int ptn_index;
	storage_read_size = 0;		// for debug

	/*	Set Temporary buffer space in QB_SCRATCH Area	*/
	gpDataBuf  = (void *)(QB_SCRATCH_ADDR);
	gpDataUnc  = (void *)(gpDataBuf + SNAP_BUF_SIZE);
	gpDataCmp  = (void *)(gpDataUnc + SNAP_UNC_SIZE);
	gpDataMeta = (void *)(gpDataCmp + SNAP_CMP_SIZE);
	gpSnapshot = (struct tcc_snapshot *)(gpDataMeta + SNAP_META_SIZE);

	/*		Get Snapshot Partition Info.	*/
	ptn_index = partition_get_index(ptn_name);
	gswap_start = partition_get_offset(ptn_index);
	gswap_size = partition_get_size(ptn_index);
	if (gswap_start == 0 || gswap_size == 0) {
		return PTN_TABLE;
	}

	/*		Set Storage offsets		*/
	gswap_end = gswap_start + gswap_size;

	/* read snapshot header */
	gpSwsusp_header  = (struct swsusp_header *)gpSnapshot->swsusp_header;
	memset(gpSwsusp_header, 0, PAGE_SIZE);

	/*		erase quickboot image header - in case of 'e' ( erase quickboot ) 	*/
	if (erase_quickboot) {
		if(tcc_write("snapshot", gswap_start, PAGE_SIZE, gpSwsusp_header) != 0) {
//			dprintf(CRITICAL, "\x1b[1;32mFailed to erase QuickBoot image!\x1b[0m\n");
			return QB_ERASE;
		} else {
			dprintf(CRITICAL, "\x1b[1;32mComplete to erase QuickBoot image!\x1b[0m\n");
			return NO_ERROR;
		}
	}

	/*		Check QuickBoot Image is exist or not.	*/
	switch(check_swap_header(gswap_start)) {
		case NO_ERROR :
			time_start = current_time();	// QuickBoot Start Time.
			show_snapshot_area();

			/*	kernel memory page restore	*/
			if (snapshot_boot() == 0)
				return storage_snapshot_jump_to_resume(gpSwsusp_header->reg); // Jump to Kernel
			else {
				watchdog_disable();	// Disable WatchDog
			#ifdef ERROR_STOP
				while(1);
			#endif
				return QB_LOADING;
			}
		case HEADER_SIG : 
			return HEADER_SIG;
		case HEADER_READ : 
			return HEADER_READ;
	}

	return UNKNOWN;
}

/*==============================================================
  = Snapshot Boot
  =============================================================*/
int snapshot_boot_from_storage()
{
	int idx = 1;
	int ret = UNKNOWN;

	for (idx = 1; idx <= 10 && ret != NO_ERROR; idx++) {
		dprintf(CRITICAL, "Try to load QB Image[%d]\n", idx);
		ret = init_snapshot();
	}

	/*	If QuickBoot Image Loading is failed, Show Failed Reason. */
	show_snapshot_area();
	switch (ret) {
		case PTN_TABLE :
			dprintf(CRITICAL, "\x1b[1;31m ERROR: snapshot partition table is not found. \x1b[0m\n");
			break;
		case QB_LOADING :
			dprintf(CRITICAL, "ERROR: Snapshot Image Loading fail...\n");
			break;
		case HEADER_SIG :
			dprintf(CRITICAL, "QB Header: QB_SIG[] length[%d]\n", /*check_sig,*/ strlen(check_sig));
			if (!strcmp(check_sig, "")) 
				dprintf(CRITICAL, "\x1b[1;32mQuickBoot Image is not found. \x1b[0m\n");
			else
				dprintf(CRITICAL, "QuickBoot Image is invalid.\n");
			break;
		case HEADER_READ :
			dprintf(INFO, "ERROR: Snapshot Header read fail...\n");
			break;
		case UNKNOWN :
			dprintf(CRITICAL, "QuickBoot Image Loading is failed. Unknown Error.\n");
			break;
		case QB_ERASE :
			dprintf(CRITICAL, "QuickBoot Image Erasing is failed.\n");
			break;
	}

	dprintf(CRITICAL, "Stop to loading QB Image. Start Normal Boot.\n");

	return -1;
}
//-[TCCQB]
//
