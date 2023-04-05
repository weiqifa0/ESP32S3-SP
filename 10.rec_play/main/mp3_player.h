/*
 * @Author: your name
 * @Date: 2021-10-13 17:25:16
 * @LastEditTime: 2022-04-26 15:20:27
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \S3_DEMO\26.rec_and_play\main\mp3_player.h
 */
/*** 
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-05-25 09:20:06
 * @LastEditors  : Kevincoooool
 * @LastEditTime : 2021-10-13 10:42:21
 * @FilePath     : \esp-idf\pro\S3_LVGL\main\app_camera.h
 */
#ifndef _wav_player_h_
#define _wav_player_h_
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#define IIS_SCLK 16
#define IIS_LCLK 7
#define IIS_DSIN 6
#define IIS_DOUT 15

extern bool playing;
esp_err_t play_wav(const char *filepath);
void play_sdfile_name(char *file_name);
void play_spiffs_name(char *file_name);
void play_i2s_init(void);
void audio_play(uint8_t index);
void record_i2s_init(void);
void all_i2s_deinit(void);
#ifdef __cplusplus
}
#endif
#endif