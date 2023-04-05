/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-07-05 09:04:44
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-13 17:08:36
 * @FilePath     : \s3_lvgl_v7\main\page\page_home.c
 */
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "lv_port_indev.h"
#include "esp_err.h"
#include "gui_time.h"

#include "bili_http.h"
#include "page_home.h"
#include "page_menu.h"
#include "page_start.h"
#include "myFont.h"
#include "number_temp.h"
#include "number.h"
#include "myimg.h"
#include "weather.h"
#include "button.h"
#include "lv_gif.h"

LV_IMG_DECLARE(taikong001);
LV_IMG_DECLARE(taikong002);
LV_IMG_DECLARE(taikong003);
LV_IMG_DECLARE(taikong004);
LV_IMG_DECLARE(taikong005);
LV_IMG_DECLARE(taikong006);
LV_IMG_DECLARE(taikong007);
LV_IMG_DECLARE(taikong008);
LV_IMG_DECLARE(taikong009);
LV_IMG_DECLARE(taikong010);

extern struct DATA_WEATHER weather_data[3];
extern const struct LV_WEATHER lv_weather[40];
extern struct _ksdiy_sys_t ksdiy_sys_t;

LV_IMG_DECLARE(bili_half);
LV_IMG_DECLARE(bili_half);
LV_IMG_DECLARE(kevincoooool);
LV_FONT_DECLARE(myFont);
LV_FONT_DECLARE(number);
LV_FONT_DECLARE(number_temp);

/*此页面窗口*/
static lv_obj_t *appWindow;

extern lv_obj_t *scr;
extern lv_obj_t *scr_body;
extern lv_obj_t *cont_head;

static lv_obj_t *img_weather;

lv_style_t style1;
/*****主体****/
lv_obj_t *cont_time, *cont_temp, *cont_fensi, *img_anim;
/*****时间****/
lv_obj_t *label_time1, *label_time1_shi, *label_time1_fen;
lv_obj_t *label_time2;
lv_task_t *task_time, *task_anim;
/*****粉丝****/
lv_obj_t *label_fensi;
lv_obj_t *label_guanzhu;
/*****温湿度****/
lv_obj_t *cont_temp_number;
lv_obj_t *label_temp_number;

static void update_weather(void);
void page_task_time(lv_task_t *task)
{
    static uint8_t i = 0, h = 0;
    if (i % 2 == 0)
        lv_obj_set_hidden(label_time1, 1);
    else
        lv_obj_set_hidden(label_time1, 0);

    if (i % 20 == 0) //10s
    {
        update_temp(); //更新温湿度
        update_time(); //更新时间
    }
    // if (i % 500 == 0) //十分钟
    //     read_fans();
    // if (i % (1000) == 0) //12小时
    // {
    //     read_weather();
    // }
    if (i % 120 == 0) //一分钟
    {
        i = 0;
        h++;
        if (h % 3 == 0) //3分钟，更新粉丝数
            update_fensi();
        if (h == h * 60 * 8) //8小时
        {
            update_weather(); //更新天气
            h = 0;
        }
    }
    i++;
}
void update_time()
{
    char now_time[40];
    sprintf((char *)now_time, "#FFFFFF %02d#", ksdiy_sys_t.timeinfo.tm_hour);
    lv_label_set_text(label_time1_shi, (const char *)now_time);
    sprintf((char *)now_time, "#FFFFFF %02d#", ksdiy_sys_t.timeinfo.tm_min);
    lv_label_set_text(label_time1_fen, (const char *)now_time);
    //星期与日历
    switch (ksdiy_sys_t.timeinfo.tm_wday)
    {
    case 0:
        sprintf((char *)now_time, "星期日\n %02d/%02d", ksdiy_sys_t.timeinfo.tm_mon + 1, ksdiy_sys_t.timeinfo.tm_mday);
        break;
    case 1:
        sprintf((char *)now_time, "星期一\n %02d/%02d", ksdiy_sys_t.timeinfo.tm_mon + 1, ksdiy_sys_t.timeinfo.tm_mday);
        break;
    case 2:
        sprintf((char *)now_time, "星期二\n %02d/%02d", ksdiy_sys_t.timeinfo.tm_mon + 1, ksdiy_sys_t.timeinfo.tm_mday);
        break;
    case 3:
        sprintf((char *)now_time, "星期三\n %02d/%02d", ksdiy_sys_t.timeinfo.tm_mon + 1, ksdiy_sys_t.timeinfo.tm_mday);
        break;
    case 4:
        sprintf((char *)now_time, "星期四\n %02d/%02d", ksdiy_sys_t.timeinfo.tm_mon + 1, ksdiy_sys_t.timeinfo.tm_mday);
        break;
    case 5:
        sprintf((char *)now_time, "星期五\n %02d/%02d", ksdiy_sys_t.timeinfo.tm_mon + 1, ksdiy_sys_t.timeinfo.tm_mday);
        break;
    case 6:
        sprintf((char *)now_time, "星期六\n %02d/%02d", ksdiy_sys_t.timeinfo.tm_mon + 1, ksdiy_sys_t.timeinfo.tm_mday);
        break;

    default:
        sprintf((char *)now_time, "星期日\n %02d/%02d", ksdiy_sys_t.timeinfo.tm_mon + 1, ksdiy_sys_t.timeinfo.tm_mday);
        break;
        break;
    }
    lv_label_set_text(label_time2, (const char *)now_time);
}
void page_home_time(void)
{
    cont_time = lv_cont_create(scr_body, NULL);
    lv_cont_set_fit2(cont_time, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(cont_time, 130, 130);
    lv_obj_set_pos(cont_time, 0, 10);
    lv_obj_add_style(cont_time, LV_BTN_PART_MAIN, &style1); /*Default button style*/
    //lv_cont_set_layout(cont_time, LV_LAYOUT_CENTER);
    //小时
    label_time1_shi = lv_label_create(cont_time, NULL);
    lv_label_set_recolor(label_time1_shi, true);
    lv_obj_set_pos(label_time1_shi, 2, 0);
    lv_label_set_align(label_time1_shi, LV_LABEL_ALIGN_CENTER); /*Center aligned lines*/
    lv_obj_set_style_local_text_font(label_time1_shi, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &number);

    //点
    label_time1 = lv_label_create(cont_time, NULL);
    lv_label_set_recolor(label_time1, true);
    lv_obj_set_pos(label_time1, 58, -5);
    lv_label_set_align(label_time1, LV_LABEL_ALIGN_CENTER); /*Center aligned lines*/
    lv_obj_set_style_local_text_font(label_time1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &number);
    lv_label_set_text(label_time1, "#FFA500 :#");
    //分钟
    label_time1_fen = lv_label_create(cont_time, NULL);
    lv_label_set_recolor(label_time1_fen, true);
    lv_obj_set_pos(label_time1_fen, 68, 0);
    lv_label_set_align(label_time1_fen, LV_LABEL_ALIGN_CENTER); /*Center aligned lines*/
    lv_obj_set_style_local_text_font(label_time1_fen, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &number);

    //星期及日期
    label_time2 = lv_label_create(cont_time, NULL);
    lv_label_set_recolor(label_time2, true);
    lv_obj_set_pos(label_time2, 0, 70);
    lv_label_set_align(label_time2, LV_LABEL_ALIGN_CENTER); /*Center aligned lines*/
    lv_obj_set_style_local_text_font(label_time2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &myFont);
    lv_obj_set_style_local_text_color(label_time2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    update_time(); //更新数据

    //动画

    obj_add_anim(
        cont_time,                        //动画对象
        (lv_anim_exec_xcb_t)lv_obj_set_x, //动画函数
        300,                              //动画速度
        -(APP_WIN_WIDTH / 2),              //起始值
        10,                               //结束值
        lv_anim_path_ease_out             //动画特效:模拟弹性物体下落
    );
}
lv_img_dsc_t bili_face = {
    .header.always_zero = 0,
    .header.w = 300,
    .header.h = 300,
    .data_size = 300 * 300 * 2,
    .header.cf = LV_IMG_CF_RAW,
    .data = NULL,
};
lv_obj_t *img_anim, *img_face, *face_cont;
LV_IMG_DECLARE(png_decoder_test);

extern const uint8_t taikongren_map[];
void page_home_anim(void)
{
    img_anim = lv_img_create(scr_body, NULL);
    //Write style LV_IMG_PART_MAIN for WIN_img0
    static lv_style_t style_img;
    lv_style_init(&style_img);

    //Write style state: LV_STATE_DEFAULT for style_img
    lv_style_set_image_recolor(&style_img, LV_STATE_DEFAULT, lv_color_make(0x00, 0x00, 0x00));
    lv_style_set_image_recolor_opa(&style_img, LV_STATE_DEFAULT, 0);
    lv_style_set_image_opa(&style_img, LV_STATE_DEFAULT, 255);
    lv_obj_add_style(img_anim, LV_IMG_PART_MAIN, &style_img);
    lv_obj_set_pos(img_anim, APP_WIN_WIDTH / 2 + 60, 70);
    lv_img_set_src(img_anim, &taikong001);

        img_face = lv_img_create(scr_body, NULL);
    lv_obj_add_style(img_face, LV_IMG_PART_MAIN, &style_img);
    lv_obj_set_pos(img_face, APP_WIN_WIDTH / 2 - 40, 70);


    bili_face.data = (uint8_t *)face_buffer;//把在线获取的buffer给img

    lv_img_set_src(img_face, &bili_face);    // lv_obj_set_size(img_anim, 100, 100);
    // lv_obj_set_pos(img_anim, 70, 50);
    //动画

    obj_add_anim(
        img_anim,                         //动画对象
        (lv_anim_exec_xcb_t)lv_obj_set_y, //动画函数
        300,                              //动画速度
        APP_WIN_HEIGHT,                     //起始值
        APP_WIN_HEIGHT - 170,               //结束值
        lv_anim_path_ease_out             //动画特效:模拟弹性物体下落
    );
}
void update_fensi()
{
    char now_time[40];
    sprintf((char *)now_time, "粉丝:%d\n关注:%d", ksdiy_sys_t.wp.follow, ksdiy_sys_t.wp.following);
    lv_label_set_text(label_fensi, (const char *)now_time);
}
void page_home_fensi(void)
{

    cont_fensi = lv_cont_create(scr_body, NULL);
    lv_cont_set_fit2(cont_fensi, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(cont_fensi, 100, 48);
    lv_obj_set_pos(cont_fensi, APP_WIN_WIDTH / 2 - 50, APP_WIN_HEIGHT - 80);
    lv_obj_add_style(cont_fensi, LV_BTN_PART_MAIN, &style1); /*Default button style*/
    lv_cont_set_layout(cont_fensi, LV_LAYOUT_CENTER);
    //主题
    static lv_style_t label_fensi_style;
    lv_style_init(&label_fensi_style);
    lv_style_set_bg_opa(&label_fensi_style, LV_STATE_DEFAULT, 0);
    lv_style_set_text_opa(&label_fensi_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&label_fensi_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    //粉丝
    label_fensi = lv_label_create(cont_fensi, NULL);
    lv_label_set_recolor(label_fensi, true);
    lv_label_set_align(label_fensi, LV_LABEL_ALIGN_CENTER); /*Center aligned lines*/
    lv_obj_set_style_local_text_font(label_fensi, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &myFont);
    lv_obj_add_style(label_fensi, LV_LABEL_PART_MAIN, &label_fensi_style);
    update_fensi();

    //动画
    obj_add_anim(
        cont_fensi,                       //动画对象
        (lv_anim_exec_xcb_t)lv_obj_set_y, //动画函数
        300,                              //动画速度
        APP_WIN_HEIGHT,                     //起始值
        APP_WIN_HEIGHT - 80,                //结束值
        lv_anim_path_ease_out             //动画特效:模拟弹性物体下落
    );
}
void update_temp()
{
    //更新温湿度
    char now_time[40];
    sprintf(now_time, "△%d℃\n▽%d℃", weather_data[0].high, weather_data[0].low);
    // printf("now temp:\n%s\n", now_time);
    lv_label_set_text(label_temp_number, (const char *)now_time);
}
static void update_weather()
{
    //天气图片

    ESP_LOGI("WEATHER", "更新天气");

    lv_img_set_src(img_weather, lv_weather[weather_data[0].code_day].img_dsc);
}
/*温度*/
void page_home_temp(void)
{

    //主cont体
    cont_temp = lv_cont_create(scr_body, NULL);
    lv_cont_set_fit2(cont_temp, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(cont_temp, 110, 50);
    lv_obj_set_pos(cont_temp, APP_WIN_WIDTH - 110, 15);

    lv_obj_add_style(cont_temp, LV_BTN_PART_MAIN, &style1); /*Default button style*/

    img_weather = lv_img_create(cont_temp, NULL);
    uint8_t i = weather_data[0].code_day;
    lv_img_dsc_t *p = (lv_img_dsc_t *)lv_weather[i].img_dsc;
    lv_img_set_src(img_weather, p);
    lv_obj_set_pos(img_weather, 60, 0);
    //风格
    static lv_style_t label_temp_style;
    lv_style_init(&label_temp_style);
    lv_style_set_text_opa(&label_temp_style, LV_STATE_DEFAULT, 255);
    lv_style_set_text_color(&label_temp_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    //百分比
    label_temp_number = lv_label_create(cont_temp, NULL);
    // lv_obj_set_size(label_temp_number, 70, 50);
    // lv_obj_set_pos(label_temp_number, 130, 15);
    lv_obj_set_size(label_temp_number, 60, 50);
    lv_obj_set_pos(label_temp_number, 8, 0);
    lv_label_set_align(label_temp_number, LV_LABEL_ALIGN_CENTER); /*Center aligned lines*/
    lv_obj_set_style_local_text_font(label_temp_number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &myFont);
    lv_obj_add_style(label_temp_number, LV_LABEL_PART_MAIN, &label_temp_style);

    update_temp();
    update_weather();
    //加载动画
    obj_add_anim(
        cont_temp,                        //动画对象
        (lv_anim_exec_xcb_t)lv_obj_set_x, //动画函数
        500,                              //动画速度
        APP_WIN_WIDTH,                     //起始值
        APP_WIN_WIDTH - 110,               //结束值
        lv_anim_path_ease_out             //动画特效:模拟弹性物体下落
    );
}

void page_home_ksdiy()
{

    static lv_obj_t *img22;
    img22 = lv_img_create(scr_body, NULL);
    lv_img_set_src(img22, &bili_half);
    lv_obj_set_pos(img22, -(APP_WIN_WIDTH / 2), APP_WIN_HEIGHT - 80);

    static lv_obj_t *img33;
    img33 = lv_img_create(scr_body, NULL);
    lv_img_set_src(img33, &bili_half);
    lv_obj_set_pos(img33, APP_WIN_WIDTH, APP_WIN_HEIGHT - 80);

    obj_add_anim(
        img22,                            //动画对象
        (lv_anim_exec_xcb_t)lv_obj_set_x, //动画函数
        500,                              //动画速度
        -(APP_WIN_WIDTH / 2),              //起始值
        10,                               //结束值
        lv_anim_path_ease_out             //动画特效:模拟弹性物体下落
    );
    obj_add_anim(
        img33,                            //动画对象
        (lv_anim_exec_xcb_t)lv_obj_set_x, //动画函数
        500,                              //动画速度
        APP_WIN_WIDTH,                     //起始值
        APP_WIN_WIDTH - 60,                //结束值
        lv_anim_path_ease_out             //动画特效:模拟弹性物体下落
    );
}

static void event_handler_page_home(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_GESTURE)
    {

        switch (lv_indev_get_gesture_dir(lv_indev_get_act()))
        {
        case LV_GESTURE_DIR_TOP:

            printf("LV_GESTURE_DIR_TOP.\n\r");
            page.PagePush(Disp_Menu);
            break;
        case LV_GESTURE_DIR_BOTTOM:

            printf("LV_GESTURE_DIR_BOTTOM.\n\r");
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
    case LV_EVENT_PRESSED: /* 按下 */
        printf("Pressed\n");
        break;

    case LV_EVENT_SHORT_CLICKED: /* 短点击 */
        printf("Short clicked\n");
        break;

    case LV_EVENT_CLICKED: /* 点击 */

        printf("Clicked\n");
        break;

    case LV_EVENT_LONG_PRESSED: /* 长按 */
    
        printf("Long press\n");
        break;

    case LV_EVENT_LONG_PRESSED_REPEAT: /* 一直长按 */
        printf("Long press repeat\n");
        break;

    case LV_EVENT_RELEASED: /* 松开 */
        printf("Released\n");
        break;
    }
}

void page_home_load()
{

    lv_style_set_bg_color(&style1, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_border_color(&style1, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_border_width(&style1, LV_STATE_DEFAULT, 0);
    lv_style_set_border_opa(&style1, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_opa(&style1, LV_STATE_DEFAULT, 0);
    page_home_ksdiy();
    page_home_temp();
    page_home_fensi();
    page_home_time();
    page_home_anim();
    ANIEND
    /* 分配屏幕触摸事件处理 */
    lv_obj_set_click(lv_layer_top(), true);
    lv_obj_set_event_cb(lv_layer_top(), event_handler_page_home);
}
static void Exit(void)
{
    lv_task_del(task_time);
    lv_task_del(task_anim);
    lv_task_handler();

    ANIEND
    lv_obj_clean(scr_body); //清空页面scr_body
    lv_obj_set_click(lv_layer_top(), false);
    lv_obj_clean(lv_layer_top());
    lv_obj_set_event_cb(lv_layer_top(), NULL); /* 分配事件处理 */

    ESP_LOGI("SYSTEM", "esp_get_free_heap_size : %d  ", esp_get_free_heap_size());
}

void page_anim_time(lv_task_t *task)
{
    static uint8_t i = 1;
    switch (i)
    {
    case 1:
        lv_img_set_src(img_anim, &taikong001);
        break;
    case 2:
        lv_img_set_src(img_anim, &taikong002);
        break;
    case 3:
        lv_img_set_src(img_anim, &taikong003);
        break;
    case 4:
        lv_img_set_src(img_anim, &taikong004);
        break;
    case 5:
        lv_img_set_src(img_anim, &taikong005);
        break;
    case 6:
        lv_img_set_src(img_anim, &taikong006);
        break;
    case 7:
        lv_img_set_src(img_anim, &taikong007);
        break;
    case 8:
        lv_img_set_src(img_anim, &taikong008);
        break;
    case 9:
        lv_img_set_src(img_anim, &taikong009);
        break;
    case 10:
        lv_img_set_src(img_anim, &taikong010);
        break;
    default:
        break;
    }

    if (i == 10)
        i = 1;
    i++;
}
static void Setup(void)
{
    page_home_load();
    task_time = lv_task_create(page_task_time, 500, LV_TASK_PRIO_MID, NULL); //创建任务
    task_anim = lv_task_create(page_anim_time, 100, LV_TASK_PRIO_MID, NULL); //创建任务

    ESP_LOGI("SYSTEM", "esp_get_free_heap_size : %d ", esp_get_free_heap_size());
}
extern struct _ksdiy_sys_t ksdiy_sys_t;
void move_task_home(uint8_t move)
{
    switch (move)
    {

    case BT1_DOWN: //往上移动

        break;
    case BT1_DOUBLE: //往上移动

        break;
    case BT1_LONG: //往上移动

        break;
    case BT1_LONGFREE: //往上移动

        break;
    case BT2_DOWN:

        break;
    case BT3_LONG: //往下移动

        break;
    case BT3_LONGFREE: //往上移动

        break;

    case BT3_DOWN: //往下移动

        break;

    default:
        break;
    }
}

/**
  * @brief  页面事件
  * @param  btn:发出事件的按键
  * @param  event:事件编号
  * @retval 无
  */
static void Event(void *btn, int event)
{
    // if(btn == &btOK)
    // {
    //     if(event == ButtonEvent::EVENT_ButtonLongPressed)
    //     {
    //         /*长按OK，退出上一个页面*/
    //         page.PagePop();
    //     }
    //     else if(event == ButtonEvent::EVENT_ButtonClick)
    //     {
    //         /*单击OK，进入对应页面*/
    //         uint8_t pageID = ICON_Grp[ICON_NowSelIndex].pageID;
    //         if(pageID != PAGE_NONE)
    //         {
    //             page.PagePush(pageID);
    //         }
    //     }
    // }

    // if(event == ButtonEvent::EVENT_ButtonPress || event == ButtonEvent::EVENT_ButtonLongPressRepeat)
    // {
    //     /*按下或长按上下键，图标上下选择*/
    //     if(btn == &btUP)
    //     {
    //         ICON_Grp_Move(-1);
    //     }
    //     if(btn == &btDOWN)
    //     {
    //         ICON_Grp_Move(+1);
    //     }
    // }
}

/**
  * @brief  页面注册
  * @param  pageID:为此页面分配的ID号
  * @retval 无
  */
void PageRegister_Home(uint8_t pageID)
{
    /*获取分配给此页面的窗口*/
    // appWindow = AppWindow_GetCont(pageID);

    /*注册至页面调度器*/
    page.PageRegister(pageID, Setup, NULL, Exit, NULL);
    printf("/*注册Home至页面调度器*/\r\n");
}
