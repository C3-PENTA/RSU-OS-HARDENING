
/****************************************************************************
  Copyright (C) 2016 Telechips, Inc.
****************************************************************************/

#ifndef __CAM_DEBUG_H__
#define __CAM_DEBUG_H__

#include <debug.h>

#define log(msg...)     if(debug) { dprintf(INFO, "!@#---- %s - ", __func__); printf(msg); }
#define FUNCTION_IN     log("In\n");
#define FUNCTION_OUT    log("Out\n");

#endif//__CAM_DEBUG_H__

