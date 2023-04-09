
#ifndef _page_imu_
#define _page_imu_

#ifdef __cplusplus
extern "C" {
#endif
/*********************
* INCLUDES
*********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
#include "lv_ex_conf.h"
#else
#include "../../lvgl/lvgl.h"
#endif

extern uint8_t imu_en;

void Imu_Task(void *pvParameters);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif // _TEST_


