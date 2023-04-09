/*
 * @Author: your name
 * @Date: 2021-11-13 14:29:15
 * @LastEditTime: 2021-11-15 15:24:56
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \s3_lvgl_v7\main\page\DisplayPrivate.h
 */

#ifndef __DISPLAYPRIVATE_H
#define __DISPLAYPRIVATE_H
/*Page*/
#include "PageManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*LittleVGL*/
#include "lvgl.h"
void DisplayError_Init();
void Display_Init();
void Display_Update();



void PageDelay(uint32_t ms);
#define PageWaitUntil(condition)\
while(!(condition)){\
    lv_task_handler();\
}


#define LV_ANIM_TIME_DEFAULT 200
#define LV_SYMBOL_DEGREE_SIGN   "\xC2\xB0"

/**
  * @brief  让一个变量增加或者减去一个值，在大于最大值后从最小值开始，小于最小值后从最大值开始
  * @param  src:被控变量
  * @param  plus:增加的值
  * @param  min:最小值
  * @param  max:最大值
  * @retval 无
  */
#define __ValuePlus(src, plus, min, max) \
	do                                   \
	{                                    \
		int __value_temp = (src);        \
		__value_temp += (plus);          \
		if (__value_temp < (min))        \
			__value_temp = (max);        \
		else if (__value_temp > (max))   \
			__value_temp = (min);        \
		(src) = __value_temp;            \
	} while (0)

#define __Sizeof(arr) (sizeof(arr) / sizeof(arr[0]))


typedef enum
{
	Disp_Home = 1,	 //主页
	Disp_Menu,		 //菜单选择界面
	Disp_Cam,		 //摄像头显示
	Disp_Color,		 //颜色识别
	Disp_Face,		 //人脸识别
	Disp_Baidu,		 //百度AI 物体和文字识别
	Disp_IMU,		 //陀螺仪
	Disp_FFT,		 //音乐频谱
	Disp_Daily,		 //每日一句
	Disp_Game_2048,	 //2048游戏
	Disp_Game_Snake, //贪吃蛇游戏
	Disp_Music,		 //音乐播放界面 采用lvgl的music demo
	Disp_Calendar,	 //日历
	Disp_Setting,	 //设置界面 可设置摄像头翻转   空调类型选择
	Disp_About,		 //关于介绍
	Disp_Max_Page,
} Display_Page; //当前界面枚举


/*AppWindow*/
void AppWindow_Create();
lv_obj_t * AppWindow_GetCont(uint8_t pageID);
lv_coord_t AppWindow_GetHeight();
lv_coord_t AppWindow_GetWidth();
#define APP_WIN_HEIGHT AppWindow_GetHeight()
#define APP_WIN_WIDTH  AppWindow_GetWidth()
void Display_Init();
#endif
