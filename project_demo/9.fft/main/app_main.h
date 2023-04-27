/*
** Copyright (c) 2023 The tinyuac project. All rights reserved.
** Created by crisqifawei 2023
*/
/***
 * @Descripttion :
 * @version      :
 * @Author       : Kevincoooool
 * @Date         : 2021-05-25 14:56:58
 * @LastEditors  : Kevincoooool
 * @LastEditTime : 2021-10-13 14:35:11
 * @FilePath     : \S3_LVGL\main\app_main.h
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>
#define VERSION "0.9.0"
typedef enum {
	WAIT_FOR_WAKEUP,
	WAIT_FOR_CONNECT,
	START_DETECT,
	START_RECOGNITION,
	START_ENROLL,
	START_DELETE,
} en_fsm_state;

extern en_fsm_state g_state;
extern int g_is_enrolling;
extern int g_is_deleting;

#endif