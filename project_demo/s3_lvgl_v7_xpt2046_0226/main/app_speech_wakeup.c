/*
 * @Descripttion :
 * @version      :
 * @Author       : Kevincoooool
 * @Date         : 2021-05-25 16:03:28
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-11 18:21:38
 * @FilePath     : \s3_lvgl_v7\main\app_speech_wakeup.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "xtensa/core-macros.h"
#include "esp_partition.h"
#include "app_speech_srcif.h"
#include "sdkconfig.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "dl_lib_coefgetter_if.h"
#include "app_main.h"
#include "app_led.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
// #include "ringbuf.h"
#include "st7789.h"
#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"
#include "page_calendar.h"
#include "page_home.h"
#include "page_wakeup.h"
#include "page_cam.h"

#include "esp_ns.h"
#include "esp_agc.h"
#include "irTask.h"
#include "wav_player.h"

#define TAG "SPEECH"
static const esp_wn_iface_t *wakenet = &WAKENET_MODEL;
static const model_coeff_getter_t *model_coeff_getter = &WAKENET_COEFF;
// MultiNet
static const esp_mn_iface_t *multinet = &MULTINET_MODEL;
model_iface_data_t *model_data_mn = NULL;

static src_cfg_t srcif;
static model_iface_data_t *model_data;

QueueHandle_t sndQueue;

extern lv_obj_t *label_speech;

/*
 * 语音识别处理函数
 */
static esp_err_t asr_multinet_control(int commit_id)
{

	if (commit_id >= 0 && commit_id < ID_MAX)
	{
		switch (commit_id)
		{
		case ID0_TIAODAOERSHIDU:
			play_spiffs_name("haode.wav");
			ac_set_temp(20);
			ESP_LOGI(TAG, "ID0_TIAODAOERSHIDU");
			lv_label_set_text(label_speech, "已调到20度");
			break;
		case ID1_TIAODAOERSHIYIDU:
			play_spiffs_name("haode.wav");
			ac_set_temp(21);
			ESP_LOGI(TAG, "ID1_TIAODAOERSHIYIDU");
			lv_label_set_text(label_speech, "已调到21度");
			break;
		case ID2_TIAODAOERSHIERDU:
			play_spiffs_name("haode.wav");
			ac_set_temp(22);
			ESP_LOGI(TAG, "ID2_TIAODAOERSHIERDU");
			lv_label_set_text(label_speech, "已调到22度");
			break;
		case ID3_TIAODAOERSHISANDU:
			play_spiffs_name("haode.wav");
			ac_set_temp(23);
			ESP_LOGI(TAG, "ID3_TIAODAOERSHISANDU");
			lv_label_set_text(label_speech, "已调到23度");
			break;
		case ID4_TIAODAOERSHISIDU:
			play_spiffs_name("haode.wav");
			ac_set_temp(24);
			ESP_LOGI(TAG, "ID4_TIAODAOERSHISIDU");
			lv_label_set_text(label_speech, "已调到24度");
			break;
		case ID5_TIAODAOERSHIWUDU:
			play_spiffs_name("haode.wav");
			ac_set_temp(25);
			ESP_LOGI(TAG, "ID5_TIAODAOERSHIWUDU");
			lv_label_set_text(label_speech, "已调到25度");
			break;
		case ID6_TIAODAOERSHLIUIDU:
			play_spiffs_name("haode.wav");
			ac_set_temp(26);
			ESP_LOGI(TAG, "ID6_TIAODAOERSHLIUIDU");
			lv_label_set_text(label_speech, "已调到26度");
			break;
		case ID7_TIAODAOERSHIQIDU:
			play_spiffs_name("haode.wav");
			ac_set_temp(27);
			ESP_LOGI(TAG, "ID7_TIAODAOERSHIQIDU");
			lv_label_set_text(label_speech, "已调到27度");
			break;
		case ID8_TIAODAOERSHIBADU:
			play_spiffs_name("haode.wav");
			ac_set_temp(28);
			ESP_LOGI(TAG, "ID8_TIAODAOERSHIBADU");
			lv_label_set_text(label_speech, "已调到28度");
			break;
		case ID9_QIDONGKONGTIAOSAOFENG:
			play_spiffs_name("haode.wav");
			ac_set_swing(true);
			ESP_LOGI(TAG, "ID9_QIDONGKONGTIAOSAOFENG");
			lv_label_set_text(label_speech, "已打开扫风");
			break;
		case ID10_TINGZHIKONGTIAOSAOFENG:
			play_spiffs_name("haode.wav");
			ac_set_swing(false);
			ESP_LOGI(TAG, "ID10_TINGZHIKONGTIAOSAOFENG");
			lv_label_set_text(label_speech, "已关闭扫风");
			break;
		case ID11_TIAODAOZIDONGFENGSU:
			play_spiffs_name("haode.wav");
			ac_set_wind_speed(0);
			ESP_LOGI(TAG, "ID11_TIAODAOZIDONGFENGSU");
			break;
		case ID12_TIAODAOYIJIFENGSU:
			play_spiffs_name("haode.wav");
			ac_set_wind_speed(1);
			ESP_LOGI(TAG, "ID12_TIAODAOYIJIFENGSU");
			break;
		case ID13_TIAODAOERJIFENGSU:
			play_spiffs_name("haode.wav");
			ac_set_wind_speed(2);
			ESP_LOGI(TAG, "ID13_TIAODAOERJIFENGSU");
			break;
		case ID14_TIAODAOSANJIFENGSU:
			play_spiffs_name("haode.wav");
			ac_set_wind_speed(3);
			ESP_LOGI(TAG, "ID14_TIAODAOSANJIFENGSU");
			break;
		case ID15_DAKAIKONGTIAO:
			play_spiffs_name("airon.wav");
			// play_spiffs_name("airon.wav");
			ac_open(1);
			ESP_LOGI(TAG, "ID15_DAKAIKONGTIAO");
			lv_label_set_text(label_speech, "已打开空调");

			break;
		case ID16_GUANBIKONGTIAO:
			play_spiffs_name("airoff.wav");
			// play_spiffs_name("airoff.wav");
			ac_open(0);
			ESP_LOGI(TAG, "ID16_GUANBIKONGTIAO");
			lv_label_set_text(label_speech, "已关闭空调");

			break;
		case ID17_GELIKONGTIAO:
			play_spiffs_name("haode.wav");
			ac_set_type(0);
			ESP_LOGI(TAG, "ID17_GELIKONGTIAO");
			lv_label_set_text(label_speech, "已设置为格力空调");
			break;
		case ID18_MEIDIKONGTIAO:
			play_spiffs_name("haode.wav");
			ac_set_type(1);
			ESP_LOGI(TAG, "ID18_MEIDIKONGTIAO");
			lv_label_set_text(label_speech, "已设置为美的空调");
			break;
		case ID19_AOKESIKONGTIAO:
			play_spiffs_name("haode.wav");
			ac_set_type(6);
			ESP_LOGI(TAG, "ID19_AOKESIKONGTIAO");
			lv_label_set_text(label_speech, "已设置为奥克斯空调");
			break;
		case ID20_HAIERKONGTIAO:
			play_spiffs_name("haode.wav");
			ac_set_type(4);
			ESP_LOGI(TAG, "ID20_HAIERKONGTIAO");
			lv_label_set_text(label_speech, "已设置为海尔空调");
			break;
		case ID21_KAIDENG:
			play_spiffs_name("haode.wav");
			app_led_set_all(30, 30, 30);
			ESP_LOGI(TAG, "ID21_KAIDENG");
			lv_label_set_text(label_speech, "已开灯");
			break;
		case ID22_GUANDENG:
			play_spiffs_name("haode.wav");
			app_led_set_all(0, 0, 0);
			ESP_LOGI(TAG, "ID22_GUANDENG");
			lv_label_set_text(label_speech, "已关灯");
			break;
		default:
			ESP_LOGI(TAG, "not supportint mode");
			break;
		}

		return ESP_OK;
	}
	return ESP_FAIL;
}
void tips_rec(void)
{
	static uint8_t i = 0;
	i++;
	if (i > 200)
	{
		i = 0;
		printf("recing...\r\n");
	}
}
void tips_wake(void)
{
	static uint8_t i = 0;
	i++;
	if (i > 200)
	{
		i = 0;
		printf("waiting for waking...\r\n");
	}
}
void nnTask(void *arg)
{
	int frequency = wakenet->get_samp_rate(model_data);
	int audio_chunksize = wakenet->get_samp_chunksize(model_data);
	int chunk_num = multinet->get_samp_chunknum(model_data_mn);
	printf("chunk_num = %d\n", chunk_num);

	int16_t *buffer = malloc(audio_chunksize * sizeof(int16_t));
	assert(buffer);
	int chunks = 0;
	int mn_chunks = 0;
	bool detect_flag = 0;
	while (1)
	{
		xQueueReceive(sndQueue, buffer, portMAX_DELAY);

		if (detect_flag == 0)
		{
			tips_wake();
			int r = wakenet->detect(model_data, buffer);
			if (r)
			{
				
				float ms = (chunks * audio_chunksize * 1000.0) / frequency;
				printf("%.2f: %s DETECTED.\n", (float)ms / 1000.0, wakenet->get_word_name(model_data, r));
				page_wakeup_start();
				play_spiffs_name("wozai.wav");
				detect_flag = 1;
				lv_label_set_text(label_speech, "我在听呢！\n请说命令词叭");
				printf("-----------------LISTENING-----------------\n\n");
				vTaskDelay(10);
				
				// play_spiffs_name("wozai.wav");
			}
			else
			{
				vTaskDelay(2);
			}
		}
		else
		{
			tips_rec();
			mn_chunks++;
			int command_id = multinet->detect(model_data_mn, buffer);

			if (asr_multinet_control(command_id) == ESP_OK)
			{

				mn_chunks = 0;
				detect_flag = 0;
				printf("\n-----------awaits to be waken up-----------\n");
				vTaskDelay(500);
				page_wakeup_end();
				vTaskDelay(2);
			}

			/* 本地语音识别未能完成识别，交给语音在线识别*/
			if (mn_chunks == chunk_num)
			{
				// asr_nothinf_control();
				// xTaskCreatePinnedToCore(&asr_nothinf_control, "asr_nothinf_control", 5 * 1024, NULL, 14, NULL, 0);
				playing = true;
				vTaskDelay(100);
				lv_label_set_text(label_speech, "刚刚没听清耶...");
				play_spiffs_name("what.wav");
				printf("\n-----------awaits to be waken up-----------\n");
				page_wakeup_end();
				vTaskDelay(2);
				mn_chunks = 0;
				detect_flag = 0;

				// ESP_LOGI(TAG, "stop listening");
				// // memset(http_buff, 0, MAX_HTTP_LEN);
				// // baidu_asr((const char *)recoder, total);
				//  mn_chunks = 0;
				// detect_flag = 0;
			}

			// if (mn_chunks == chunk_num || command_id > -1)
			// {
			// 	mn_chunks = 0;
			// 	detect_flag = 0;
			// 	if (command_id > -1)
			// 	{
			// 		asr_multinet_control(command_id);
			// 		vTaskDelay(500);
			// 		mn_chunks = 0;
			// 		detect_flag = 0;
			// 		// printf("\n-----------awaits to be waken up-----------\n");
			// 		// page_wakeup_end();
			// 		// lv_label_set_text(label_speech, "请说 Hi LeXin 唤醒我吧");
			// 	}
			// 	else
			// 	{
			// 		playing = true;
			// 		vTaskDelay(100);
			// 		ESP_LOGE(TAG, "can not recognize any speech commands");
			// 		lv_label_set_text(label_speech, "刚刚没听清耶...");
			// 		play_spiffs_name("what.wav");
			// 		mn_chunks = 0;
			// 		detect_flag = 0;
			// 		// play_spiffs_name("what.wav");
			// 	}
			// 	printf("\n-----------awaits to be waken up-----------\n");
			// 	page_wakeup_end();
			// }
		}
		// chunks++;
	}

	free(buffer);
	vTaskDelete(NULL);
}

void app_speech_wakeup_init()
{
	// Initialize NN model
	model_data = wakenet->create(model_coeff_getter, DET_MODE_95);

	int wake_word_num = wakenet->get_word_num(model_data);
	char *wake_word_list = wakenet->get_word_name(model_data, 1);
	if (wake_word_num)
		printf("wake word number = %d, word1 name = %s\n",
			   wake_word_num, wake_word_list);

	int audio_chunksize = wakenet->get_samp_chunksize(model_data);

	model_data_mn = multinet->create(&MULTINET_COEFF, 4000);
	// Initialize sound source
	sndQueue = xQueueCreate(5, (audio_chunksize * sizeof(int16_t)));
	srcif.queue = &sndQueue;
	srcif.item_size = audio_chunksize * sizeof(int16_t);

	xTaskCreatePinnedToCore(&recsrcTask, "recsrcTask", 3 * 1024, (void *)&srcif, 13, NULL, 0);

	xTaskCreatePinnedToCore(&nnTask, "nnTask", 5 * 1024, NULL, 14, NULL, 0);
}
