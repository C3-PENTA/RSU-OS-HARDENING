/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <debug.h>
#include <smem.h>
//#include <msm_panel.h>
//#include <pm8x41.h>
//#include <pm8x41_wled.h>
#include <board.h>
//#include <mdp5.h>
#include <platform/gpio.h>
#include <target/display.h>
#include <dev/fbcon.h>

//static struct msm_fb_panel_data panel;
static uint8_t display_enable;

static struct fbcon_config *fb_config;

void display_init(void)
{
	uint32_t hw_id = 0 ;//board_hardware_id();
//	uint32_t soc_ver = board_soc_version();

	dprintf(INFO, "display_init(),target_id=%d.\n", hw_id);
#if (1)
	fb_config = lcdc_init();
	ASSERT(fb_config);
	fbcon_setup(fb_config);
    
#if !defined(DISPLAY_SPLASH_SCREEN_DIRECT)
       dprintf(INFO, "Splash Display int: start\n");    
       splash_image_load("bootlogo_480x272", fb_config);
       display_splash_logo(fb_config);
       dprintf(INFO, "Splash Display int: end\n");
#endif

#else
	switch (hw_id) {
	case HW_PLATFORM_MTP:
	case HW_PLATFORM_FLUID:
	case HW_PLATFORM_SURF:
		mipi_toshiba_video_720p_init(&(panel.panel_info));
		panel.clk_func = msm8974_mdss_dsi_panel_clock;
		panel.power_func = msm8974_mipi_panel_power;
		panel.fb.base = MIPI_FB_ADDR;
		panel.fb.width =  panel.panel_info.xres;
		panel.fb.height =  panel.panel_info.yres;
		panel.fb.stride =  panel.panel_info.xres;
		panel.fb.bpp =  panel.panel_info.bpp;
		panel.fb.format = FB_FORMAT_RGB888;
		panel.mdp_rev = MDP_REV_50;
		break;
	default:
		return;
	};

	if (msm_display_init(&panel)) {
		dprintf(CRITICAL, "Display init failed!\n");
		return;
	}
#endif
	display_enable = 1;
}

void display_shutdown(void)
{
	if (display_enable)
		;//msm_display_off();
}
