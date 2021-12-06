
#include <debug.h>
#include <dev/camera/sensor_if.h>

SENSOR_FUNC_TYPE	sensor_func;

int sensor_if_close(void) {
	dprintf(INFO, "!@#---- %s()\n", __func__);
	
	return sensor_func.close();
}

int sensor_if_open(void) {
	dprintf(INFO, "!@#---- %s()\n", __func__);
	
	sensor_if_connect_api(&sensor_func);
	return sensor_func.open();
}

int sensor_if_tune(int camera_type, int camera_encode) {
	dprintf(INFO, "!@#---- %s() - type = 0x%x, encode = 0x%x\n", __func__, camera_type, camera_encode);
	
	return sensor_func.tune(camera_type, camera_encode);
}

