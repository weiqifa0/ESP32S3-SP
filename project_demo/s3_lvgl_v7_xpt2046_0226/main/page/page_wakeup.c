/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-06-05 10:13:51
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-03 16:04:59
 * @FilePath     : \esp-idf\pro\KSDIY_ESPCAM\main\page\page_wakeup.c
 */
#include "page_wakeup.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include "app_anim.h"
#include <esp_system.h>
#include "esp_log.h"
#include "lv_port_indev.h"

LV_FONT_DECLARE(myFont);
extern lv_obj_t *scr;
extern lv_obj_t *scr_body;
extern lv_obj_t *label_speech;

void wakeup_init(void)
{

    label_speech = lv_label_create(scr, NULL);
	/*Modify the Label's text*/
	
	static lv_style_t style_label_speech;
	lv_style_init(&style_label_speech);

	//Write style state: LV_STATE_DEFAULT for style_label_speech
	lv_style_set_radius(&style_label_speech, LV_STATE_DEFAULT, 0);
	lv_style_set_bg_color(&style_label_speech, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_style_set_bg_grad_color(&style_label_speech, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_style_set_bg_grad_dir(&style_label_speech, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_label_speech, LV_STATE_DEFAULT, 255);
	lv_style_set_text_color(&style_label_speech, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    lv_obj_set_style_local_text_font(label_speech, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &myFont);
	lv_style_set_text_letter_space(&style_label_speech, LV_STATE_DEFAULT, 2);
	lv_style_set_pad_left(&style_label_speech, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_right(&style_label_speech, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_top(&style_label_speech, LV_STATE_DEFAULT, 0);
	lv_style_set_pad_bottom(&style_label_speech, LV_STATE_DEFAULT, 0);

	lv_obj_add_style(label_speech, LV_LABEL_PART_MAIN, &style_label_speech);
    lv_label_set_long_mode(label_speech,LV_LABEL_LONG_BREAK);
	lv_obj_set_pos(label_speech, 0, LV_VER_RES);
	lv_obj_set_size(label_speech, LV_HOR_RES, 50);
    lv_label_set_recolor(label_speech, true);
    lv_label_set_text(label_speech, "#0000ff 你好# \n我是酷世DIY制作的ESP32-CAM");
}

void page_wakeup_load()
{
    wakeup_init();

    obj_add_anim(
		label_speech,								  //动画对象
		(lv_anim_exec_xcb_t)lv_obj_set_y,		  //动画函数
		lv_anim_speed_to_time(800, 0, 50), //动画速度
		APP_WIN_HEIGHT,					  //起始值
		APP_WIN_HEIGHT-60,  //结束值
		lv_anim_path_linear						  //动画特效:模拟弹性物体下落
	);
    ANIEND
}
void page_wakeup_end()
{

    obj_add_anim(
		label_speech,								  //动画对象
		(lv_anim_exec_xcb_t)lv_obj_set_y,		  //动画函数
		lv_anim_speed_to_time(800, 0, 50), //动画速度
		APP_WIN_HEIGHT-60,					  //起始值
		APP_WIN_HEIGHT,  //结束值
		lv_anim_path_linear						  //动画特效:模拟弹性物体下落
	);
    ANIEND
    lv_obj_del(label_speech);
}
void page_wakeup_start()
{
    //获取芯片可用内存
    printf("     esp_get_free_heap_size : %d  \n", esp_get_free_heap_size());
    //获取从未使用过的最小内存
    printf("     esp_get_minimum_free_heap_size : %d  \n", esp_get_minimum_free_heap_size());
    page_wakeup_load();
}
