/*** 
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-06-12 11:01:25
 * @LastEditors  : Kevincoooool
 * @LastEditTime : 2021-10-13 14:33:02
 * @FilePath     : \S3_LVGL\main\app_face.h
 */

#ifndef _APP_FACEDEC_
#define _APP_FACEDEC_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"
#include "fb_gfx.h"

#include "esp_camera.h"
void Face_DEC(void *arg);
void draw_fillRect(camera_fb_t *image_matrix, int32_t x, int32_t y, int32_t w, int32_t h);
#endif