/*
** Copyright (c) 2023 The tinyuac project. All rights reserved.
** Created by crisqifawei 2023
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <esp_system.h>
#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"
#include "lv_port_indev.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"
#include "number.h"
#include "fft.h"
#include "fft_user_interface.h"
#include "audio_queue.h"
#include "platform_i2s_interface.h"

#include "app_main.h"


#define APP_WIN_HEIGHT 240
#define APP_WIN_WIDTH  240

#define DISP_MAX_HOR        LV_HOR_RES
#define DISP_MAX_VER        LV_VER_RES
QueueHandle_t g_audioQueue;
#define ANIEND  while(lv_anim_count_running())lv_task_handler();//等待动画完成

/*此页面窗口*/
static lv_obj_t *appWindow;
/*标题栏*/
static lv_obj_t *labelTitle;

/*标题栏分隔线*/
static lv_obj_t *lineTitle;
/*图标显示容器，用于裁剪显示*/
static lv_obj_t *contDisp;

extern uint8_t fft_en;

lv_obj_t *chart_fft;
lv_chart_series_t *series_fft;

/**
 * @brief  创建标题栏
 * @param  无
 * @retval 无
 */
static void TitleTextCreate()
{
	LV_FONT_DECLARE(number);
	appWindow = lv_cont_create(lv_scr_act(), NULL);
	static lv_style_t style_cont;
	lv_style_set_pad_left(&style_cont, LV_STATE_DEFAULT, 0);
	lv_style_set_bg_opa(&style_cont, LV_STATE_DEFAULT, 255);
	lv_style_set_pad_top(&style_cont, LV_STATE_DEFAULT, 0);
	lv_style_set_border_color(&style_cont, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_border_width(&style_cont, LV_STATE_DEFAULT, 0);
	lv_style_set_border_opa(&style_cont, LV_STATE_DEFAULT, 255);
	lv_style_set_bg_color(&style_cont, LV_STATE_DEFAULT, LV_COLOR_BLACK); //设置屏幕背景
	lv_obj_add_style(appWindow, LV_BTN_PART_MAIN, &style_cont);			  /*Default button style*/
	lv_obj_set_pos(appWindow, 0, 0);
	lv_obj_set_size(appWindow, APP_WIN_WIDTH, APP_WIN_HEIGHT);

	labelTitle = lv_label_create(appWindow, NULL);

	static lv_style_t style_label;
	lv_style_init(&style_label);

	// Write style state: LV_STATE_DEFAULT for style_label
	lv_style_set_radius(&style_label, LV_STATE_DEFAULT, 0);
	lv_style_set_bg_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_bg_grad_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_bg_grad_dir(&style_label, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_label, LV_STATE_DEFAULT, 255);
	lv_style_set_text_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_style_local_text_font(labelTitle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &number);
	lv_style_set_text_letter_space(&style_label, LV_STATE_DEFAULT, 2);
	lv_style_set_pad_left(&style_label, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_right(&style_label, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_top(&style_label, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_bottom(&style_label, LV_STATE_DEFAULT, 0);

	lv_obj_add_style(labelTitle, LV_LABEL_PART_MAIN, &style_label);
	lv_label_set_long_mode(labelTitle, LV_LABEL_LONG_BREAK);
	lv_label_set_align(labelTitle, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_pos(labelTitle, 0, 0);
	lv_obj_set_size(labelTitle, APP_WIN_WIDTH, 60);
	lv_label_set_recolor(labelTitle, true);

	/*默认选中的是第二个图标*/
	lv_label_set_static_text(labelTitle, "MicFFT");
	lv_obj_align(labelTitle, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	lv_obj_set_auto_realign(labelTitle, true);

	lineTitle = lv_line_create(appWindow, NULL);
	static lv_style_t style_line;
	lv_style_init(&style_line);
	lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, lv_color_make(0xFF, 0x3E, 0x96));
	lv_style_set_line_width(&style_line, LV_STATE_DEFAULT, 5);
	lv_obj_add_style(lineTitle, LV_LINE_PART_MAIN, &style_line);
	lv_obj_set_pos(lineTitle, 0, 55);
	lv_obj_set_size(lineTitle, DISP_MAX_HOR, 5);

	static lv_point_t screen_line3[] = {{0, 0}, {LV_HOR_RES_MAX, 0}};
	lv_line_set_points(lineTitle, screen_line3, 2);
}
//创建包含框界面
static void MicDataShowCreate(void)
{
	contDisp = lv_cont_create(appWindow, NULL);
	static lv_style_t style_cont;
	lv_style_set_pad_left(&style_cont, LV_STATE_DEFAULT, 0);
	lv_style_set_bg_opa(&style_cont, LV_STATE_DEFAULT, 50);
	lv_style_set_pad_top(&style_cont, LV_STATE_DEFAULT, 0);
	lv_style_set_border_color(&style_cont, LV_STATE_DEFAULT, LV_COLOR_MAROON);
	lv_style_set_border_width(&style_cont, LV_STATE_DEFAULT, 0);
	lv_style_set_border_opa(&style_cont, LV_STATE_DEFAULT, 50);
	lv_style_set_bg_color(&style_cont, LV_STATE_DEFAULT, LV_COLOR_MAROON); //设置屏幕背景
	lv_obj_add_style(contDisp, LV_BTN_PART_MAIN, &style_cont);			  /*Default button style*/
	lv_obj_set_size(contDisp, APP_WIN_WIDTH, APP_WIN_HEIGHT - 60);
	lv_obj_set_pos(contDisp, 0, 60);

	chart_fft = lv_chart_create(contDisp, NULL);
	lv_obj_set_size(chart_fft, APP_WIN_WIDTH, APP_WIN_HEIGHT - 60);
	lv_chart_set_point_count(chart_fft, 500);
	lv_obj_align(chart_fft, NULL, LV_ALIGN_CENTER, 0, 0);

	lv_obj_set_style_local_bg_opa(chart_fft, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 100); /*Max. opa.*/
	lv_obj_set_style_local_bg_grad_dir(chart_fft, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
	lv_obj_set_style_local_bg_main_stop(chart_fft, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 255); /*Max opa on the top*/
	lv_obj_set_style_local_bg_grad_stop(chart_fft, LV_CHART_PART_SERIES, LV_STATE_DEFAULT, 255); /*Transparent on the bottom*/

	lv_chart_set_type(chart_fft, LV_CHART_TYPE_SCATTER);

	//lv_chart_set_type(chart_fft, LV_CHART_TYPE_LINE); //折线模式

	lv_chart_set_range(chart_fft, 0, APP_WIN_HEIGHT);

	series_fft = lv_chart_add_series(chart_fft, LV_COLOR_GREEN);
}

static void Exit(void)
{
	fft_en = 0;
	lv_obj_del(appWindow);
	printf("%s !Dram: %d bytes\r\n", __func__, heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
}

void FFT_Setup(void)
{

	//获取芯片可用内存
	printf(" page_fft_start    esp_get_free_heap_size : %ld  \n", esp_get_free_heap_size());
	//获取从未使用过的最小内存
	printf(" page_fft_start    esp_get_minimum_free_heap_size : %ld  \n", esp_get_minimum_free_heap_size());
	printf("%s !Dram: %d bytes\r\n", __func__, heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
	TitleTextCreate();
	MicDataShowCreate();
	fft_en = 1;

	xTaskCreatePinnedToCore(&FFT_Task, "FFT_Task", 1024 * 8, NULL, 6, NULL, 0);
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	long divisor = (in_max - in_min);
	if (divisor == 0)
	{
		return -1; // AVR returns -1, SAM returns 0
	}
	return (x - in_min) * (out_max - out_min) / divisor + out_min;
}

#define CANVAS_WIDTH 240
#define CANVAS_HEIGHT 240
#define FFT_AUDIO_DATA_SIZE 64

uint8_t fft_en = 0;
uint8_t fft_dis_buff[240] = {0};
extern lv_obj_t *chart_fft;
extern lv_chart_series_t *series_fft;

int8_t i2s_read_rawdata[1024];
/**
 * @descripttion: fft频谱任务
 * @param {void} *arg
 * @return {*}
 */
void FFT_Task(void *arg)
{
	size_t bytesread;
	int16_t *buffptr;
	float *samples_fc32 = (float *)calloc(AUDIO_PACKET_SIZE, sizeof(float));
	double data = 0;
	AudioPacket packet;

	// 创建音频数据队列
	g_audioQueue = xQueueCreate(AUDIO_QUEUE_LENGTH, sizeof(AudioPacket));
	if (g_audioQueue == NULL) {
		printf("Failed to create audio queue\n");
	}
	int16_t fft_audio_buffer[FFT_AUDIO_DATA_SIZE] = {0};
	while (1)
	{
		if (fft_en == 1)
		{
			if (xQueueReceive(g_audioQueue, &packet, portTICK_PERIOD_MS) ) {
				//memcpy(fft_audio_buffer, &packet.data, packet.size > FFT_AUDIO_DATA_SIZE ? FFT_AUDIO_DATA_SIZE : packet.size);
				if (packet.size > 0) {
					printf("packet.size=%u\n", packet.size);
				}
				// fft_config_t *real_fft_plan = fft_init(FFT_AUDIO_DATA_SIZE, FFT_REAL, FFT_FORWARD, NULL, NULL);
				// buffptr = (int16_t *)fft_audio_buffer;
				// for (uint16_t count_n = 0; count_n < real_fft_plan->size; count_n++)
				// {
				// 	real_fft_plan->input[count_n] = (float)map(buffptr[count_n], INT16_MIN, INT16_MAX, -1000, 1000);
				// }
				// fft_execute(real_fft_plan);
				// for (uint16_t count_n = 1; count_n < CANVAS_HEIGHT; count_n++)
				// {
				// 	data = sqrt(real_fft_plan->output[2 * count_n] * real_fft_plan->output[2 * count_n] + real_fft_plan->output[2 * count_n + 1] * real_fft_plan->output[2 * count_n + 1]);
				// 	fft_dis_buff[CANVAS_HEIGHT - count_n] = map(data, 0, 2000, 0, 240);
				// }
				// fft_destroy(real_fft_plan);

				// for (uint16_t count_y = 0; count_y < 240;)
				// {
				// 	lv_chart_set_next(chart_fft, series_fft, fft_dis_buff[count_y]);
				// 	count_y += 5;
				// }
			}
		}
		else
		{
			i2s_driver_uninstall(I2S_NUM_0);
			vTaskDelete(NULL);
		}
	}
}