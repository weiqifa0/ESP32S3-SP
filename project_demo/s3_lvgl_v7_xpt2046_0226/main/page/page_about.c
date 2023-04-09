/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-06-05 10:13:51
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-13 17:17:52
 * @FilePath     : \esp-idf\pro\KSDIY_ESPCAM\main\page\page_about.c
 */
#include "page_about.h"
#include "page_start.h"
#include "page_menu.h"
#include "app_main.h"
#include "button.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"
#include "lv_port_indev.h"
#include <esp_system.h>
#include "esp_log.h"
#include "lv_port_indev.h"

#include "lv_gif.h"

/*此页面窗口*/
static lv_obj_t *appWindow;
/*标题栏*/
static lv_obj_t *labelTitle;

/*标题栏分隔线*/
static lv_obj_t *lineTitle;
/*图标显示容器，用于裁剪显示*/
static lv_obj_t *contDisp;

/**
  * @brief  创建标题栏
  * @param  无
  * @retval 无
  */

extern const uint8_t myeye_map[]; /*Use the example gif*/
/**
 * @Descripttion: 创建标题栏
 * @param {*}
 * @return {*}
 * @Author: Kevincoooool
 */
static void Title_Create()
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

	//Write style state: LV_STATE_DEFAULT for style_label
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
	lv_obj_set_size(labelTitle, APP_WIN_WIDTH, 55);
	lv_label_set_recolor(labelTitle, true);

	/*默认选中的是第二个图标*/
	lv_label_set_static_text(labelTitle, "About");
	lv_obj_align(labelTitle, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	lv_obj_set_auto_realign(labelTitle, true);

	lineTitle = lv_line_create(appWindow, NULL);
	static lv_style_t style_line;
	lv_style_init(&style_line);
	lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, lv_color_make(0xFa, 0x05, 0x05));
	lv_style_set_line_width(&style_line, LV_STATE_DEFAULT, 5);
	lv_obj_add_style(lineTitle, LV_LINE_PART_MAIN, &style_line);
	lv_obj_set_pos(lineTitle, 0, 55);
	lv_obj_set_size(lineTitle, DISP_MAX_HOR, 5);
	
	static lv_point_t screen_line3[] = {{0, 0}, {LV_HOR_RES_MAX, 0}};
	lv_line_set_points(lineTitle, screen_line3, 2);
}

/**
 * @Descripttion: 创建下方布局
 * @param {*}
 * @return {*}
 * @Author: Kevincoooool
 */
static void Cont_create(void)
{
	LV_FONT_DECLARE(myFont);
	contDisp = lv_cont_create(appWindow, NULL);
	static lv_style_t style_cont;
	lv_style_set_pad_left(&style_cont, LV_STATE_DEFAULT, 0);
	lv_style_set_bg_opa(&style_cont, LV_STATE_DEFAULT, 255);
	lv_style_set_pad_top(&style_cont, LV_STATE_DEFAULT, 0);
	lv_style_set_border_color(&style_cont, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_border_width(&style_cont, LV_STATE_DEFAULT, 0);
	lv_style_set_border_opa(&style_cont, LV_STATE_DEFAULT, 255);
	lv_style_set_bg_color(&style_cont, LV_STATE_DEFAULT, LV_COLOR_BLACK); //设置屏幕背景
	lv_obj_add_style(contDisp, LV_BTN_PART_MAIN, &style_cont);			  /*Default button style*/
	lv_obj_set_size(contDisp, APP_WIN_WIDTH, APP_WIN_HEIGHT - 60);
	lv_obj_set_pos(contDisp, 0, 60);
	lv_obj_t *img_gif = lv_gif_create_from_data(contDisp, myeye_map);
	lv_obj_set_size(img_gif, 150, 150);
	lv_obj_set_pos(img_gif, APP_WIN_WIDTH / 2 - 75, 0);
	// lv_obj_align(img_gif, contDisp, LV_ALIGN_IN_TOP_MID, 0, 40); //设置图片显示顶端

	lv_obj_t *label_version = lv_label_create(contDisp, NULL);

	static lv_style_t style_label;
	lv_style_init(&style_label);

	//Write style state: LV_STATE_DEFAULT for style_label
	lv_style_set_radius(&style_label, LV_STATE_DEFAULT, 0);
	lv_style_set_bg_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_bg_grad_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_bg_grad_dir(&style_label, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_label, LV_STATE_DEFAULT, 255);
	lv_style_set_text_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_style_local_text_font(label_version, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &myFont);
	lv_style_set_text_letter_space(&style_label, LV_STATE_DEFAULT, 2);
	lv_style_set_pad_left(&style_label, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_right(&style_label, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_top(&style_label, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_bottom(&style_label, LV_STATE_DEFAULT, 0);

	lv_obj_add_style(label_version, LV_LABEL_PART_MAIN, &style_label);
	lv_label_set_long_mode(label_version, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label_version, LV_LABEL_ALIGN_CENTER);
	// lv_obj_set_pos(label_version, 0, 0);
	lv_obj_set_size(label_version, APP_WIN_WIDTH, 100);
	// lv_label_set_recolor(label_version, true);

	/*默认选中的是第二个图标*/
	lv_label_set_static_text(label_version, "KS-ESP32-MASTER\nKevincoooool\n"__DATE__);
	lv_obj_align(label_version, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
	lv_obj_set_auto_realign(label_version, true);
}
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
static void Exit(void)
{

	obj_add_anim(
		appWindow,						   //动画对象
		(lv_anim_exec_xcb_t)lv_obj_set_x,  //动画函数
		lv_anim_speed_to_time(300, 0, 50), //动画速度
		0,								   //起始值
		APP_WIN_WIDTH,						   //结束值
		lv_anim_path_ease_out			   //动画特效:模拟弹性物体下落
	);
	ANIEND
	lv_obj_del(appWindow);
	printf("%s !Dram: %d bytes\r\n", __func__, heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
}

static void Setup(void)
{
	//获取芯片可用内存
	printf(" %s    esp_get_free_heap_size : %d  \n", __func__,esp_get_free_heap_size());
	//获取从未使用过的最小内存
	printf(" %s    esp_get_minaboutm_free_heap_size : %d  \n", __func__,esp_get_minimum_free_heap_size());
	printf("%s !Dram: %d bytes\r\n", __func__, heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
		Title_Create();
	Cont_create();
	obj_add_anim(
		appWindow,						   //动画对象
		(lv_anim_exec_xcb_t)lv_obj_set_x,  //动画函数
		lv_anim_speed_to_time(300, 0, 50), //动画速度
		APP_WIN_WIDTH,						   //起始值
		0,								   //结束值
		lv_anim_path_ease_out			   //动画特效:模拟弹性物体下落
	);
	ANIEND
		lv_obj_set_click(lv_layer_top(), true);
	lv_obj_set_event_cb(lv_layer_top(), event_handler_touch);
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
void PageRegister_About(uint8_t pageID)
{
	/*获取分配给此页面的窗口*/
	// appWindow = AppWindow_GetCont(pageID);

	/*注册至页面调度器*/
	page.PageRegister(pageID, Setup, NULL, Exit, NULL);
	printf("/*注册About至页面调度器*/\r\n");
}
