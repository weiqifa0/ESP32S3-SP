/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-07-05 09:04:44
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-19 15:46:53
 * @FilePath: \s3_lvgl_v7\main\page\PageManager.c
 */

#include "PageManager.h"
#include "esp_log.h"
#include "page_menu.h"
#include "page_home.h"
#include "page_wakeup.h"
#include "page_cam.h"
#include "page_fft.h"
#include "page_start.h"
#include "page_calendar.h"
#include "page_baiduai.h"
#include "page_game_2048.h"
#include "page_game_snake.h"

#include "page_daily.h"
#include "page_imu.h"
#include "page_about.h"

#include "page_fft.h"
#include "updata.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"

#include "esp_err.h"
#include "cJSON.h"
#include "mqtt_client.h"
#include "app_camera.h"
#include "web_server.h"
#include "app_wifi.h"
extern struct _ksdiy_sys_t ksdiy_sys_t;

extern lv_obj_t *cont_head;

void save_clock(uint8_t clock_index, uint8_t s, uint8_t h, uint8_t m)
{
	char index[10];
	char info[30];
	if (h > 23 || m > 60 || s > 2)
	{
		ESP_LOGI("CLOCK", "保存时间格式错误1 s:%d,h:%d,m:%d", s, h, m);
		return;
	}
	sprintf(info, "{\"s\":%d,\"h\":%d,\"m\":%d}", s, h, m);
	sprintf(index, "ck%d", clock_index);
	save_nvs((const char *)index, info);
}
void lv_page_clock_init() //闹钟信息获取
{
	//char info[15];
	// if(read_nvs("t_all",info))//获取哔哩哔哩uid
	// {
	//     sprintf(str,"%s%s%s",BILIBILI_REQUEST1,uid,BILIBILI_REQUEST2);
	//     ESP_LOGI(TAG_HTTP,"获取到uid");
	// }
}
void lv_ico_web_show(uint8_t state)
{
	if (state)
	{
		ksdiy_sys_t.ico.lv_web = lv_label_create(cont_head, NULL);
		lv_label_set_recolor(ksdiy_sys_t.ico.lv_web, true);
		lv_obj_set_style_local_text_color(ksdiy_sys_t.ico.lv_web, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
		lv_label_set_text(ksdiy_sys_t.ico.lv_web, LV_SYMBOL_DIRECTORY);
	}
	else
	{
		if (ksdiy_sys_t.ico.lv_web != NULL)
			lv_obj_del(ksdiy_sys_t.ico.lv_web);
	}
}
void lv_ico_clock_show(uint8_t state)
{
	if (state)
	{
		ksdiy_sys_t.ico.lv_clock = lv_label_create(cont_head, NULL);
		lv_obj_set_style_local_text_color(ksdiy_sys_t.ico.lv_clock, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
		lv_label_set_recolor(ksdiy_sys_t.ico.lv_clock, true);
		lv_label_set_text(ksdiy_sys_t.ico.lv_clock, LV_SYMBOL_BELL);
	}
	else
	{
		if (ksdiy_sys_t.ico.lv_clock != NULL)
			lv_obj_del(ksdiy_sys_t.ico.lv_clock);
	}
}
void lv_ico_temp_show(uint8_t state)
{
	if (state)
	{
		ksdiy_sys_t.ico.lv_temp = lv_label_create(cont_head, NULL);
		lv_label_set_recolor(ksdiy_sys_t.ico.lv_temp, true);
		lv_obj_set_style_local_text_color(ksdiy_sys_t.ico.lv_temp, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
		lv_label_set_text(ksdiy_sys_t.ico.lv_temp, "#3CB371 " LV_SYMBOL_REFRESH);
	}
	else
	{
		if (ksdiy_sys_t.ico.lv_temp != NULL)
			lv_obj_del(ksdiy_sys_t.ico.lv_temp);
	}
}
void web_temp_save(uint8_t i)
{
	if (i)
	{
		save_nvs("temp", "y");
		lv_message("温湿度上传开启", 1000);
	}
	else
	{
		save_nvs("temp", "n");
		lv_message("温湿度上传关闭", 1000);
	}
}
void web_switch_save(uint8_t i)
{
	if (i)
	{
		save_nvs("web_set", "y");
		lv_message("WEB开启", 1000);
	}
	else
	{
		save_nvs("web_set", "n");
		lv_message("WEB关闭", 1000);
	}
}
void cam_switch_save(uint8_t i)
{
	if (i)
	{
		save_nvs("cam_set", "y");
		lv_message("CAM WEB开启", 1000);
	}
	else
	{
		save_nvs("cam_set", "n");
		lv_message("CAM WEB关闭", 1000);
	}
}
void cam_vflip_switch_save(uint8_t i)
{
	if (i)
	{
		save_nvs("cam_vflip", "y");
		lv_message("vflip开启", 1000);
	}
	else
	{
		save_nvs("cam_vflip", "n");
		lv_message("vflip关闭", 1000);
	}
}
void cam_hmirror_switch_save(uint8_t i)
{
	if (i)
	{
		save_nvs("cam_hmirror", "y");
		lv_message("hmirror开启", 1000);
	}
	else
	{
		save_nvs("cam_hmirror", "n");
		lv_message("hmirror关闭", 1000);
	}
}
void clock_switch_save(uint8_t i)
{
	if (i)
	{
		save_nvs("clock", "y");
		lv_message("闹钟开启", 1000);
	}
	else
	{
		save_nvs("clock", "n");
		lv_message("闹钟关闭", 1000);
	}
}
void lv_ksdiy_web_init()
{
	char info[5];
	if (read_nvs("web_set", info)) //读取web状态
	{
		if (!strcmp("y", info)) //开启web
		{
			if (start_file_server("/spiffs") != ESP_OK) //启动文件系统
			{
				ksdiy_sys_t.state.web_set = 0;
				ESP_LOGI("web_set", "web_set启动失败");
			}
			ESP_LOGI("web_set", "打开web_set服务");
			ksdiy_sys_t.state.web_set = 1;
		}
		else //关闭
		{
			save_nvs("web_set", "n");
			ksdiy_sys_t.state.web_set = 0;
		}
	}
	else //第一次使用未选择
	{
		ESP_LOGI("web_set", "web_set没启动");
		ksdiy_sys_t.state.web_set = 0;
	}
	if (ksdiy_sys_t.state.web_set) //显示图标
	{
		lv_ico_web_show(1);
	}
}
void lv_ksdiy_cam_init()
{
	// app_camera_init();
	char info[5];
	if (read_nvs("cam_vflip", info)) //读取web状态
	{
		if (!strcmp("y", info)) //开启web
		{

			ksdiy_sys_t.state.cam_vflip = 1;
			sensor_t *s = esp_camera_sensor_get();
			s->set_vflip(s, ksdiy_sys_t.state.cam_vflip);
			ESP_LOGI("web_set", "cam_vflip打开");
		}

		else //关闭
		{
			ksdiy_sys_t.state.cam_vflip = 0;
			sensor_t *s = esp_camera_sensor_get();
			s->set_vflip(s, ksdiy_sys_t.state.cam_vflip);
			ESP_LOGI("web_set", "cam_vflip关闭");
		}
	}
	else //第一次使用未选择
	{
		ESP_LOGI("web_set", "cam_vflip没设置过");
		ksdiy_sys_t.state.cam_vflip = 0;
		ksdiy_sys_t.state.cam_hmirror = 0;
	}
	if (read_nvs("cam_hmirror", info)) //读取web状态
	{
		if (!strcmp("y", info)) //开启web
		{

			ksdiy_sys_t.state.cam_hmirror = 1;
			sensor_t *s = esp_camera_sensor_get();
			s->set_hmirror(s, ksdiy_sys_t.state.cam_hmirror);
			ESP_LOGI("web_set", "cam_hmirror打开");
		}

		else //关闭
		{
			ksdiy_sys_t.state.cam_hmirror = 0;
			sensor_t *s = esp_camera_sensor_get();
			s->set_hmirror(s, ksdiy_sys_t.state.cam_hmirror);
			ESP_LOGI("web_set", "cam_hmirror关闭");
		}
	}

	else //第一次使用未选择
	{
		ESP_LOGI("web_set", "cam_hmirror没设置过");
		ksdiy_sys_t.state.cam_vflip = 0;
		ksdiy_sys_t.state.cam_hmirror = 0;
	}
	// esp_camera_deinit();
}
void ksdiy_sys_info_init()
{

	ksdiy_sys_t.state.sys_button = 0;
	ksdiy_sys_t.user_data = 0;
	ksdiy_sys_t.wp.follow = get_value_ksdiyfs(1);
	ksdiy_sys_t.wp.following = get_value_ksdiyfs(0);
	lv_ksdiy_web_init();
	// lv_ksdiy_cam_init();

}

PageList_TypeDef PageList[Disp_Max_Page];
// PageList_TypeDef *PageList;
uint8_t PageStack[Disp_Max_Page];
static uint8_t PageStackSize;
static uint8_t PageStackTop;
static uint8_t MaxPage;
static bool IsPageBusy;

#define IS_PAGE(page) ((page) < (MaxPage))

void PageManager(uint8_t pageMax, uint8_t pageStackSize);
bool PageClear(uint8_t pageID);
bool PageRegister(
	uint8_t pageID,
	CallbackFunction_t setupCallback,
	CallbackFunction_t loopCallback,
	CallbackFunction_t exitCallback,
	EventFunction_t eventCallback);
void PageEventTransmit(void *obj, int event);
void PageChangeTo(uint8_t pageID);
bool PagePush(uint8_t pageID);
bool PagePop(void);
void PageStackClear(void);
void Running(void);

_PageManager page = {
	PageManager,
	0,
	0,
	0,
	0,
	0,
	PageRegister,
	PageClear,
	PagePush,
	PagePop,
	PageChangeTo,
	PageEventTransmit,
	PageStackClear,
	Running,
};

/**
  * @brief  初始化页面调度器
  * @param  pageMax: 页面最大数量
  * @param  eventMax: 事件最大数量
  * @retval 无
  */
void PageManager(uint8_t pageMax, uint8_t pageStackSize)
{
	MaxPage = pageMax;
	page.NewPage = 0;
	page.OldPage = 0;
	IsPageBusy = false;

	/* 申请内存，清空列表 */
	// PageList = (PageList_TypeDef *)malloc(sizeof(PageList_TypeDef));
	for (uint8_t page = 0; page < MaxPage; page++)
	{
		PageClear(page);
	}
	/*页面栈配置*/
	PageStackSize = pageStackSize;
	// PageStack = (uint8_t *)heap_caps_malloc(pageStackSize * sizeof(uint8_t), MALLOC_CAP_SPIRAM);
	// PageStack = (uint8_t *)malloc(pageStackSize);
	PageStackClear();
}

/**
  * @brief  页面调度器析构
  * @param  无
  * @retval 无
  */
// ~PageManager()
// {
//     delete[] PageList;
//     delete[] PageStack;
// }

/**
  * @brief  清除一个页面
  * @param  pageID: 页面编号
  * @retval true:成功 false:失败
  */
bool PageClear(uint8_t pageID)
{
	if (!IS_PAGE(pageID))
		return false;

	PageList[pageID].SetupCallback = NULL;
	PageList[pageID].LoopCallback = NULL;
	PageList[pageID].ExitCallback = NULL;
	PageList[pageID].EventCallback = NULL;

	return true;
}

/**
  * @brief  注册一个基本页面，包含一个初始化函数，循环函数，退出函数，事件函数
  * @param  pageID: 页面编号
  * @param  setupCallback: 初始化函数回调
  * @param  loopCallback: 循环函数回调
  * @param  exitCallback: 退出函数回调
  * @param  eventCallback: 事件函数回调
  * @retval true:成功 false:失败
  */
bool PageRegister(
	uint8_t pageID,
	CallbackFunction_t setupCallback,
	CallbackFunction_t loopCallback,
	CallbackFunction_t exitCallback,
	EventFunction_t eventCallback)
{
	if (!IS_PAGE(pageID))
		return false;

	PageList[pageID].SetupCallback = setupCallback;
	PageList[pageID].LoopCallback = loopCallback;
	PageList[pageID].ExitCallback = exitCallback;
	PageList[pageID].EventCallback = eventCallback;
	return true;
}

/**
  * @brief  页面事件传递
  * @param  obj: 发生事件的对象
  * @param  event: 事件编号
  * @retval 无
  */
void PageEventTransmit(void *obj, int event)
{
	/*将事件传递到当前页面*/
	if (PageList[page.NowPage].EventCallback != NULL)
		PageList[page.NowPage].EventCallback(obj, event);
}

/**
  * @brief  切换到指定页面
  * @param  pageID: 页面编号
  * @retval 无
  */
void PageChangeTo(uint8_t pageID)
{
	if (!IS_PAGE(pageID))
		return;
	/*检查页面是否忙碌*/
	if (!IsPageBusy)
	{

		/*新页面ID*/
		page.NextPage = page.NewPage = pageID;

		/*标记为忙碌状态*/
		IsPageBusy = true;
	}
}

/**
  * @brief  页面压栈，跳转至该页面
  * @param  pageID: 页面编号
  * @retval true:成功 false:失败
  */
bool PagePush(uint8_t pageID)
{
	if (!IS_PAGE(pageID))
		return false;

	/*检查页面是否忙碌*/
	if (IsPageBusy)
		return false;
	/*防止栈溢出*/
	if (PageStackTop >= PageStackSize - 1)
		return false;
	/*防止重复页面压栈*/
	if (pageID == PageStack[PageStackTop])
		return false;
	/*栈顶指针上移*/
	PageStackTop++;

	/*页面压栈*/
	PageStack[PageStackTop] = pageID;

	/*页面跳转*/
	PageChangeTo(PageStack[PageStackTop]);

	return true;
}

/**
  * @brief  页面弹栈，跳转至上一个页面
  * @param  无
  * @retval true:成功 false:失败
  */
bool PagePop()
{
	/*检查页面是否忙碌*/
	if (IsPageBusy)
		return false;

	/*防止栈溢出*/
	if (PageStackTop == 0)
		return false;

	/*清空当前页面*/
	PageStack[PageStackTop] = 0;

	/*弹栈，栈顶指针下移*/
	PageStackTop--;

	/*页面跳转*/
	PageChangeTo(PageStack[PageStackTop]);

	return true;
}

/**
  * @brief  清空页面栈
  * @param  无
  * @retval 无
  */
void PageStackClear()
{
	/*检查页面是否忙碌*/
	if (IsPageBusy)
		return;

	/*清除栈中左右数据*/
	for (uint8_t i = 0; i < PageStackSize; i++)
	{
		PageStack[i] = 0;
	}
	/*栈顶指针复位*/
	PageStackTop = 0;
}

/**
  * @brief  页面调度器状态机
  * @param  无
  * @retval 无
  */
void Running()
{
	/*页面切换事件*/
	if (page.NewPage != page.OldPage)
	{
		/*标记为忙碌状态*/
		IsPageBusy = true;

		/*触发旧页面退出事件*/
		if (PageList[page.OldPage].ExitCallback != NULL && IS_PAGE(page.OldPage))
			PageList[page.OldPage].ExitCallback();

		/*标记旧页面*/
		page.LastPage = page.OldPage;

		/*标记新页面为当前页面*/
		page.NowPage = page.NewPage;

		/*触发新页面初始化事件*/
		if (PageList[page.NewPage].SetupCallback != NULL && IS_PAGE(page.NewPage))
			PageList[page.NewPage].SetupCallback();
		/*新页面初始化完成，标记为旧页面*/
		page.OldPage = page.NewPage;
	}
	else
	{
		/*标记页面不忙碌，处于循环状态*/
		IsPageBusy = false;

		/*页面循环事件*/
		if (PageList[page.NowPage].LoopCallback != NULL && IS_PAGE(page.NowPage))
			PageList[page.NowPage].LoopCallback();
	}
}
