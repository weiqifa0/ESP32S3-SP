/*
 * @Author: Kevincoooool
 * @Date: 2022-03-04 13:45:19
 * @Description:
 * @version:
 * @Filename: Do not Edit
 * @LastEditTime: 2022-03-04 13:48:56
 * @FilePath: \SP_DEMO\9.fft\main\page\page_fft.h
 */
/***
 * @Descripttion :
 * @version      :
 * @Author       : Kevincoooool
 * @Date         : 2021-06-05 10:13:51
 * @LastEditors  : Kevincoooool
 * @LastEditTime : 2021-07-06 14:22:21
 * @FilePath     : \esp-idf\pro\KSDIY_ESPCAM\main\page\page_fft.h
 */


#ifndef _page_fft_
#define _page_fft_

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
void move_task_fft(uint8_t move);

void FFT_Task(void *arg);
void FFT_Setup(void);
#ifdef __cplusplus
} /* extern "C" */
#endif




#endif // _TEST_


