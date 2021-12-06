#ifndef __SENSOR_IF_H__
#define __SENSOR_IF_H__

#if 0
enum camera_type {
	CAM_TYPE_DEFAULT	= 0,
	CAM_TYPE_SVM		= 0,
	CAM_TYPE_AVM,
	CAM_TYPE_AUX,
	CAM_TYPE_CMMB,
	CAM_TYPE_LVDS,
	CAM_TYPE_MAX
};
#endif

enum camera_type {
	CAM_TYPE_DEFAULT	= 0,
	CAM_TYPE_CVBS		= 0,
	CAM_TYPE_SVIDEO,
	CAM_TYPE_COMPONENT,
	CAM_TYPE_AUX,
	CAM_TYPE_CMMB,
	CAM_TYPE_LVDS,
	CAM_TYPE_UPDATE,
	CAM_TYPE_MAX
};

enum camera_enc {
	CAM_ENC_DEFAULT		= 0,
	CAM_ENC_NTSC		= 0,
	CAM_ENC_PAL,
	CAM_ENC_MAX
};

typedef struct {
	int (* close)(void);
	int (* open)(int camera_type, int camera_encode);
	int (* tune)(unsigned int camera_type, unsigned int camera_encode);
} SENSOR_FUNC_TYPE;

extern int sensor_if_connect_api(SENSOR_FUNC_TYPE * sensor_func);
extern int sensor_if_close(void);
extern int sensor_if_open(void);
extern int sensor_if_tune(int camera_type, int camera_encode);

#endif//__SENSOR_IF_H__

