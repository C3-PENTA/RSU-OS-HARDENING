/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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
struct guid_partition android_boot_partitions[] = {
	{ "guid",	"raw",		17},
	{ "boot",	"raw",		15*1024},
	{ "system",	"ext4", 	650*1024},
	{ "cache",	"ext4", 	150*1024},
	{ "recovery",	"raw",		15*1024},
	{ "dtb",	"raw",		5*1024},
	{ "splash",	"raw",		4*1024},
	{ "misc",	"raw",		1*1024},
	{ "tcc", 	"raw",		1*1024},
	{ "aboot", 	"raw",		1*1024},
	{ "userdata",	"ext4", 	0},
	{ 0, 0, 0},
};

struct guid_partition chrome_boot_partitions[] = {
	{ "guid",	"raw",		17},
	{ "STATE",	"ext4",		2*1024*1024},
	{ "KERN-A",	"raw", 		32*1024},
	{ "ROOT-A",	"raw", 		6*1024*1024},
	{ "KERN-B",	"raw",		32*1024},
	{ "ROOT-B",	"raw",		4*1024},
	{ "KERN-C",	"raw",		1*1024},
	{ "ROOT-C",	"raw",		1*1024},
	{ "OEM", 	"ext4",		32*1024},
	{ "reserved", 	"raw",		1*1024},
	{ "reserved", 	"raw",		1*1024},
	{ "RWFW",	"ext4", 	16*1024},
	{ "EFI-SYSTEM",	"vfat", 	32*1024},
	{ "aboot",	"raw",		1*1024},
	{ 0, 0, 0},
};

struct guid_partition dual_boot_partitions[] = {
	{ "guid",	"raw",		17},
	{ "STATE",	"ext4",		2*1024*1024},
	{ "KERN-A",	"raw", 		32*1024},
	{ "ROOT-A",	"raw", 		6*1024*1024},
	{ "KERN-B",	"raw",		32*1024},
	{ "ROOT-B",	"raw",		4*1024},
	{ "KERN-C",	"raw",		1*1024},
	{ "ROOT-C",	"raw",		1*1024},
	{ "OEM", 	"ext4",		32*1024},
	{ "reserved", 	"raw",		1*1024},
	{ "reserved", 	"raw",		1*1024},
	{ "RWFW",	"ext4", 	16*1024},
	{ "EFI-SYSTEM",	"vfat", 	32*1024},
	{ "cleared",	"raw",		1*1024},
	{ "boot",	"raw",		15*1024},
	{ "system",	"ext4", 	650*1024},
	{ "cache",	"ext4", 	150*1024},
	{ "recovery",	"raw",		15*1024},
	{ "dtb",	"raw",		5*1024},
	{ "splash",	"raw",		4*1024},
	{ "misc",	"raw",		1*1024},
	{ "tcc", 	"raw",		1*1024},
	{ "aboot", 	"raw",		1*1024},
	{ "userdata",	"ext4", 	0},
	{ 0, 0, 0},
};
