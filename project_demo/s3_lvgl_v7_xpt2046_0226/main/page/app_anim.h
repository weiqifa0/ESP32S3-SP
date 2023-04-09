/*
 * @Author: your name
 * @Date: 2021-10-13 17:29:16
 * @LastEditTime: 2021-11-13 17:23:46
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \s3_lvgl_v7\main\page\app_anim.h
 */
#ifndef _APP_ANIM_H_
#define _APP_ANIM_H_

#include "lvgl.h"
#include "DisplayPrivate.h"
#define DISP_MAX_HOR        LV_HOR_RES
#define DISP_MAX_VER        LV_VER_RES 

#define ANIEND  while(lv_anim_count_running())lv_task_handler();//等待动画完成

void obj_add_anim(lv_obj_t *obj, lv_anim_exec_xcb_t exec_cb, uint16_t time, lv_coord_t start, lv_coord_t end, lv_anim_path_cb_t path_cb);


#endif

