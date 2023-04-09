/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-06-05 10:13:51
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-12-09 14:20:24
 * @FilePath: \s3_lvgl_v7_xpt2046\main\page\page_cam.c
 */
#include "page_cam.h"
#include "page_start.h"
#include "page_menu.h"
#include "app_main.h"
#include "app_face.h"
#include "EasyTracer.h"
#include "button.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"
#include "lv_port_indev.h"
#include "app_camera.h"
#include "fb_gfx.h"
#include <esp_system.h>
#include "esp_log.h"
#include "lv_port_indev.h"
#define TAG "PAGE_CAM"

lv_obj_t *img_cam; //要显示图像
lv_group_t *group_button;
extern lv_img_dsc_t imgfft;
extern lv_img_dsc_t img_dsc;
extern camera_fb_t *fb;
RESULT Resu;
uint8_t color_type = 0;
TARGET_CONDI Condition[3] = {
	{50, 120, 70, 250, 10, 180, 40, 40, 120, 120},	//绿色
	{180, 255, 70, 250, 10, 180, 40, 40, 120, 120}, //红色
	{130, 170, 70, 250, 10, 180, 40, 40, 120, 120}, //蓝色
};

void Cam_Task(void *pvParameters)
{

	// /* 入口处检测一次 */
	ESP_LOGI(TAG, "Run Run uxHighWaterMark = %d", uxTaskGetStackHighWaterMark(NULL));
	// FILE *fp = NULL;
	portTickType xLastWakeTime;

	while (1)
	{
		if (cam_en)
		{
			if (face_en == 0)
			{
				static int64_t last_frame = 0;
				if (!last_frame)
				{
					last_frame = esp_timer_get_time();
				}
				fb = esp_camera_fb_get();
				if (fb == NULL)
				{
					vTaskDelay(100);
					ESP_LOGE(TAG, "Get image failed!");
				}
				else
				{
					if (Trace(&Condition[color_type], &Resu) && color_en == 1)
					{
						printf("x:%d y:%d w:%d h:%d ", Resu.x, Resu.y, Resu.w, Resu.h);
						if (Resu.x > 0 && Resu.y > 0)
						{
							draw_fillRect(fb, Resu.x - Resu.w / 2, Resu.y - Resu.h / 2, Resu.w, Resu.h);
						}
					}
					// static uint16_t i = 0;
					// if (i == 0)
					// {
					// ESP_LOGI(TAG, "Reading file");
					// fp = fopen("/spiffs/output.rgb", "r");
					// if (fp == NULL)
					// {
					// 	ESP_LOGE(TAG, "Failed to open file for reading");
					// 	return;
					// }
					// printf("打开文件成功\n");
					// fgets((char *)dis_buf, 100 * 100 * 2, fp);
					// fclose(fp);

					// //ffmpeg -t 30 -i bad_apple_30.mp4 -vf "fps=10,scale=-1:100:flags=lanczos,crop=100:in_h:(in_w-100)/2:0,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -c:v rawvideo -pix_fmt rgb565be output.rgb

					// fgets((char *)dis_buf, 100 * 100 * 2, fp);

					img_dsc.data = fb->buf;
					lv_img_set_src(img_cam, &img_dsc);

					// i++;
					// if (i == 400)
					// {
					// 	i = 0;
					// 	fclose(fp);
					// }
					esp_camera_fb_return(fb);
					fb = NULL;
					int64_t fr_end = esp_timer_get_time();
					int64_t frame_time = fr_end - last_frame;
					last_frame = fr_end;
					frame_time /= 1000;
					ESP_LOGI("esp", "MJPG:  %ums (%.1ffps)", (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
					// vTaskDelayUntil(&xLastWakeTime, (20 / portTICK_RATE_MS));
				}
			}
		}
		else
		{
			if (fb)
			{
				esp_camera_fb_return(fb);
				free(fb);
			}

			fb = NULL;
			vTaskDelete(NULL);
		}
	}

	// never reach
	while (1)
	{
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
//当前界面是菜单时MOVE任务要做的事情
void move_task_cam(uint8_t move)
{

	switch (move)
	{
	case BT1_DOWN: //往上移动
		// color_type--;
		// color_type = color_type <= 0 ? 0 : color_type;
		break;
	case BT1_LONG: //往上移动

		break;
	case BT1_LONGFREE: //往上移动
		break;
	case BT3_DOWN: //往下移动
		// color_type++;
		// color_type = color_type >= 2 ? 2 : color_type;
		break;
	case BT3_LONG: //往下移动

		break;
	case BT3_LONGFREE: //往上移动
		break;

	default:
		break;
	}
}
//当前界面是菜单时MOVE任务要做的事情
void move_task_color(uint8_t move)
{

	switch (move)
	{
	case BT1_DOWN: //往上移动
		color_type--;
		color_type = color_type < 0 ? 0 : color_type;
		break;
	case BT1_LONG: //往上移动

		break;
	case BT1_LONGFREE: //往上移动
		break;
	case BT3_DOWN: //往下移动
		color_type++;
		color_type = color_type > 2 ? 2 : color_type;
		break;
	case BT3_LONG: //往下移动

		break;
	case BT3_LONGFREE: //往上移动
		break;

	default:
		break;
	}
}
void imgcam_init(void)
{

	img_cam = lv_img_create(lv_scr_act(), NULL);
	static lv_style_t style_img;
	lv_style_init(&style_img);

	//Write style state: LV_STATE_DEFAULT for style_img
	lv_style_set_image_recolor(&style_img, LV_STATE_DEFAULT, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_image_recolor_opa(&style_img, LV_STATE_DEFAULT, 0);
	lv_style_set_image_opa(&style_img, LV_STATE_DEFAULT, 255);
	lv_obj_add_style(img_cam, LV_IMG_PART_MAIN, &style_img);
	lv_obj_set_pos(img_cam, 0, 0);
	lv_obj_set_size(img_cam, 320, 240);
}
lv_task_t *task_cam;
static void event_handler_touch(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_GESTURE)
	{

		switch (lv_indev_get_gesture_dir(lv_indev_get_act()))
		{
		case LV_GESTURE_DIR_TOP:

			printf("LV_GESTURE_DIR_TOP.\n\r");
			break;
		case LV_GESTURE_DIR_BOTTOM:
			printf("LV_GESTURE_DIR_BOTTOM.\n\r");
			/*长按OK，退出上一个页面*/
			// page.PagePop();
			break;
		case LV_GESTURE_DIR_RIGHT:
			
			printf("LV_GESTURE_DIR_RIGHT.\n\r");
			break;
		case LV_GESTURE_DIR_LEFT:

			printf("LV_GESTURE_DIR_LEFT.\n\r");
			break;
		default:
			break;
		}
	}
	switch (event)
	{
	case LV_EVENT_LONG_PRESSED: /* 长按 */
		page.PagePop();
		printf("Long press\n");
		break;
	}

	/*Etc.*/
}
void page_cam_load()
{
	app_camera_init();
	imgcam_init();
	obj_add_anim(
		img_cam,						   //动画对象
		(lv_anim_exec_xcb_t)lv_obj_set_x,  //动画函数
		lv_anim_speed_to_time(300, 0, 50), //动画速度
		320,							   //起始值
		0,								   //结束值
		lv_anim_path_linear				   //动画特效:模拟弹性物体下落
	);
	ANIEND
	xTaskCreatePinnedToCore(&Cam_Task, "Cam_Task", 1024 * 5, NULL, 14, NULL, 0);
	lv_obj_set_click(lv_layer_top(), true);
	lv_obj_set_event_cb(lv_layer_top(), event_handler_touch);
}

static void Exit(void)
{
	cam_en = 0,color_en = 0, face_en = 0;
	vTaskDelay(200);
	esp_camera_deinit();
	obj_add_anim(
		img_cam,						   //动画对象
		(lv_anim_exec_xcb_t)lv_obj_set_x,  //动画函数
		lv_anim_speed_to_time(300, 0, 50), //动画速度
		0,								   //起始值
		320,							   //结束值
		lv_anim_path_linear				   //动画特效:模拟弹性物体下落
	);
	ANIEND
	if (fb)
	{
		esp_camera_fb_return(fb);
		free(fb);
	}
	lv_obj_del(img_cam);
}
// extern en_fsm_state g_state;

static void Setup_Cam(void)
{
	//获取芯片可用内存
	printf(" page_cam_start    esp_get_free_heap_size : %d  \n", esp_get_free_heap_size());
	//获取从未使用过的最小内存
	printf(" page_cam_start    esp_get_minimum_free_heap_size : %d  \n", esp_get_minimum_free_heap_size());
	cam_en = 1;
	printf("%s !Dram: %d bytes\r\n", __func__, heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
	page_cam_load();
}
static void Setup_Color(void)
{
	//获取芯片可用内存
	printf(" page_cam_start    esp_get_free_heap_size : %d  \n", esp_get_free_heap_size());
	//获取从未使用过的最小内存
	printf(" page_cam_start    esp_get_minimum_free_heap_size : %d  \n", esp_get_minimum_free_heap_size());
	cam_en = 1;
	color_en = 1;
	printf("%s !Dram: %d bytes\r\n", __func__, heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
	page_cam_load();
}
static void Setup_Face(void)
{
	//获取芯片可用内存
	printf(" page_cam_start    esp_get_free_heap_size : %d  \n", esp_get_free_heap_size());
	//获取从未使用过的最小内存
	printf(" page_cam_start    esp_get_minimum_free_heap_size : %d  \n", esp_get_minimum_free_heap_size());

	printf("%s !Dram: %d bytes\r\n", __func__, heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
	page_cam_load();
	face_en = 1;
	g_state = START_RECOGNITION;
	xTaskCreatePinnedToCore(&Face_DEC, "Face_DEC", 1024 * 4, NULL, 5, NULL, 0);
}
/**
  * @brief  页面事件
  * @param  btn:发出事件的按键
  * @param  event:事件编号
  * @retval 无
  */
static void Event(void *btn, int event)
{
}

/**
  * @brief  页面注册
  * @param  pageID:为此页面分配的ID号
  * @retval 无
  */
void PageRegister_Cam(uint8_t pageID)
{
	/*获取分配给此页面的窗口*/
	// appWindow = AppWindow_GetCont(pageID);

	/*注册至页面调度器*/
	page.PageRegister(pageID, Setup_Cam, NULL, Exit, NULL);
	printf("/*注册Cam至页面调度器*/");
}
/**
  * @brief  页面注册
  * @param  pageID:为此页面分配的ID号
  * @retval 无
  */
void PageRegister_Color(uint8_t pageID)
{
	/*获取分配给此页面的窗口*/
	// appWindow = AppWindow_GetCont(pageID);

	/*注册至页面调度器*/
	page.PageRegister(pageID, Setup_Color, NULL, Exit, NULL);
	printf("/*注册Color至页面调度器*/");
}
/**
  * @brief  页面注册
  * @param  pageID:为此页面分配的ID号
  * @retval 无
  */
void PageRegister_Face(uint8_t pageID)
{
	/*获取分配给此页面的窗口*/
	// appWindow = AppWindow_GetCont(pageID);

	/*注册至页面调度器*/
	page.PageRegister(pageID, Setup_Face, NULL, Exit, NULL);
	printf("/*注册Face至页面调度器*/");
}
