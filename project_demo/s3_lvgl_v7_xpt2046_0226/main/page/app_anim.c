/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-07-05 10:25:59
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-02 10:47:31
 * @FilePath     : \esp-idf\pro\KSDIY_ESPCAM\main\page\app_anim.c
 */
#include "app_anim.h"

/**
 * @descripttion: 1
 * @param {lv_obj_t} *动画对象
 * @param {lv_anim_exec_xcb_t} 动画函数
 * @param {uint16_t} 动画速度
 * @param {lv_coord_t} 开始位置
 * @param {lv_coord_t} 结束位置
 * @param {lv_anim_path_cb_t} 动画形式
 * @return {*}
 */
void obj_add_anim(lv_obj_t *obj, lv_anim_exec_xcb_t exec_cb, uint16_t time, lv_coord_t start, lv_coord_t end, lv_anim_path_cb_t path_cb)
{

#if (LVGL_VERSION_MAJOR >= 8)
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_values(&a, start, end);
	lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)exec_cb);
	lv_anim_set_var(&a, obj);
	lv_anim_set_time(&a, time);
	lv_anim_set_playback_time(&a, 0);
	lv_anim_set_path_cb(&a, path_cb);
	// lv_anim_set_ready_cb(&a, spectrum_end_cb);
	lv_anim_start(&a);
#else /* Before v7 */
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, obj);							  //动画对象
	lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)exec_cb); //动画函数
	lv_anim_set_time(&a, time);
	a.start = start;	 //起始值
	a.end = end;		 //结束值
	a.path.cb = path_cb; //动画计算方法
	a.playback_time = 0; //回放时间设为0不执行动画回放
	lv_anim_start(&a);	 //开启动画

#endif
}
