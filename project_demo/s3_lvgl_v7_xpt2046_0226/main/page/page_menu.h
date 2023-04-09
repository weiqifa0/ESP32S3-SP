/*** 
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-07-02 09:59:54
 * @LastEditors  : Kevincoooool
 * @LastEditTime : 2021-09-18 10:40:29
 * @FilePath     : \esp-idf\pro\ESP_MASTER\Software\main\page\page_menu.h
 */
/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-07-02 09:59:37
 * @LastEditors: Please set LastEditors
 * @LastEditTime : 2021-07-02 09:59:37
 * @FilePath     : \esp-idf\KSDIY_ESPCAM\main\page_menu.c
 */
#ifndef _PAGEMENU_
#define _PAGEMENU_

#ifdef __cplusplus
extern "C"
{
#endif
/*********************
* INCLUDES
*********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
#include "lv_ex_conf.h"
#else
#include "../../lvgl/lvgl.h"
#include "../../lv_ex_conf.h"
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif


typedef struct
{
	const void *img_buffer; //图标图像数据
	const char *text;		//文字描述
	lv_obj_t *img;			//图标img对象
	bool focus;				//图标当前是否被聚焦
	uint8_t pageID;			//对应的页面ID
} ICON_TypeDef;


uint8_t Menu_Choose(void);
void move_task_menu(uint8_t dir);
#endif
