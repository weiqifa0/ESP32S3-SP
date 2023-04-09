/*
 * @Descripttion :
 * @version      :
 * @Author       : Kevincoooool
 * @Date         : 2021-09-04 16:11:59
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-18 10:46:20
 * @FilePath     : \s3_lvgl_v7\main\app_main.c
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"
#include "lv_examples/src/lv_demo_music/lv_demo_music.h"
#include "lv_examples/src/lv_demo_benchmark/lv_demo_benchmark.h"
#include "lvgl_helpers.h"
#include "esp_freertos_hooks.h"
#include "button.h"
#include "bsp_adc.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "app_camera.h"
#include "app_wifi.h"
#include "app_httpd.h"
#include "irTask.h"
#include "lv_gif.h"
// #include "lv_port_fs.h"
#include "lv_png.h"
#include "app_main.h"
#include "app_face.h"
#include "app_face.h"
#include "page_cam.h"
#include "page_wakeup.h"
#include "page_start.h"
#include "page_menu.h"
#include "page_baiduai.h"
#include "page_menu.h"
#include "page_daily.h"
#include "page_calendar.h"
#include "page_start.h"
#include "page_game_2048.h"
#include "page_game_snake.h"
#include "page_imu.h"
#include "page_cam.h"
#include "page_home.h"
#include "page_fft.h"

#include "app_speech_srcif.h"
#include "file_manager.h"
#include "app_main.h"
#include "app_led.h"
#include "DisplayPrivate.h"

#define TAG "ESP32S3"

camera_fb_t *fb = NULL;

lv_obj_t *label_speech;
uint8_t cam_en = 0, color_en = 0, face_en = 0, baiduai_en = 0;
/*根据屏幕尺寸决定显示大小*/
#if LV_HOR_RES_MAX == 320
lv_img_dsc_t img_dsc = {
	.header.always_zero = 0,
	.header.w = 320,
	.header.h = 240,
	.data_size = 320 * 240 * 2,
	.header.cf = LV_IMG_CF_TRUE_COLOR,
	.data = NULL,
};
/*
	读取画面中的每一个像素值
*/
uint16_t RGB_ReadBit16Point(unsigned short x, unsigned short y)
{
	if (fb != NULL)
		return fb->buf[2 * x + 320 * 2 * y] << 8 | fb->buf[2 * x + 240 * 2 * y + 1];
	else
		return 0;
}
#elif LV_HOR_RES_MAX == 240
lv_img_dsc_t img_dsc = {
	.header.always_zero = 0,
	.header.w = 240,
	.header.h = 240,
	.data_size = 240 * 240 * 2,
	.header.cf = LV_IMG_CF_TRUE_COLOR,
	.data = NULL,
};
/*
	读取画面中的每一个像素值
*/
uint16_t RGB_ReadBit16Point(unsigned short x, unsigned short y)
{
	if (fb != NULL)
		return fb->buf[2 * x + 240 * 2 * y] << 8 | fb->buf[2 * x + 240 * 2 * y + 1];
	else
		return 0;
}
#endif

extern struct _ksdiy_sys_t ksdiy_sys_t;

extern uint8_t fft_en;
void page_switch()
{
	uint32_t user_data;
	user_data = Button_Value;

	if (user_data)
	{
		if (Button_Value == BT1_LONG || Button_Value == BT3_LONG)
		{
		}
		else
			Button_Value = 0;

		if (user_data == BT1_DOWN || user_data == BT3_DOWN)
		{
			switch (page.NowPage)
			{
			case Disp_Home:
				// move_task_home(user_data);
				break;
			case Disp_Menu:
				move_task_menu(user_data);
				break;
			case Disp_Cam:
				// move_task_cam(user_data);
				break;
			case Disp_Color:
				move_task_color(user_data);
				break;
			case Disp_Face:
				((user_data == BT1_DOWN) ? (g_state = START_DELETE) : (g_state = START_RECOGNITION));
				// move_task_cam(user_data);
				break;
			case Disp_Baidu:
				move_task_baiduai(user_data);
				break;
			case Disp_FFT:
				move_task_fft(user_data);
				break;
			case Disp_Daily:
				move_task_daily(user_data);
				break;
			case Disp_Game_2048:
				move_task_game_2048(user_data);
				break;
			case Disp_Game_Snake:
				move_task_game_snake(user_data);
				break;
			case Disp_Calendar:
				// move_task_calendar(user_data);
				encoder_handler((user_data == BT1_DOWN) ? 2 : 3);
				break;
			case Disp_Setting:
				encoder_handler((user_data == BT1_DOWN) ? 2 : 3);
				// move_task_calendar(user_data);
				break;
			default:
				break;
			}
		}
		else if (user_data == BT1_LONG || user_data == BT3_LONG)
		{
			switch (page.NowPage)
			{
			case Disp_Home:
				// move_task_home(user_data);
				break;
			case Disp_Menu:

				move_task_menu(user_data);
				break;
			case Disp_Cam:
				// move_task_cam(user_data);
				break;
			case Disp_Color:
				// move_task_color(user_data);
				break;
			case Disp_Face:
				// ((user_data == BT1_DOWN) ? (g_state = START_DELETE) : (g_state = START_RECOGNITION));
				// move_task_cam(user_data);
				break;
			case Disp_Baidu:

				// move_task_baiduai(user_data);
				break;
			case Disp_Daily:
				move_task_daily(user_data);
				break;

			case Disp_Calendar:
				// move_task_calendar(user_data);
				// encoder_handler((user_data == BT1_DOWN) ? 2 : 3);
				break;
			case Disp_Setting:
				encoder_handler((user_data == BT1_LONG) ? 2 : 3);
				// move_task_calendar(user_data);
				break;
			default:
				break;
			}
		}
		else if (user_data == BT2_DOWN)
		{
			switch (page.NowPage)
			{
			case Disp_Home:
				// move_task_home(user_data);

				break;
			case Disp_Menu:

				page.PagePush(Menu_Choose());

				switch (page.NowPage)
				{
				case Disp_Cam:
					cam_en = 1;
					break;
				case Disp_Color:
					cam_en = 1;
					color_en = 1;
					break;
				case Disp_Face:
					face_en = 1;
					g_state = START_RECOGNITION;
					xTaskCreatePinnedToCore(&Face_DEC, "Face_DEC", 1024 * 4, NULL, 5, NULL, 0);
					break;
				case Disp_Baidu:
					baiduai_en = 1;
					break;
				case Disp_FFT:
					// xTaskCreatePinnedToCore(&FFT_Task, "FFT_Task", 1024 * 8, NULL, 6, NULL, 0);
					break;
				case Disp_About:

					break;

				default:
					break;
				}
				break;
			case Disp_Face:
				// move_task_cam(user_data);
				g_state = START_ENROLL;
				break;
			case Disp_Daily:
				move_task_daily(user_data);
				break;
			case Disp_Calendar:
				// move_task_calendar(user_data);
				encoder_handler(1);
				break;
			case Disp_Setting:
				encoder_handler(1);
				// move_task_calendar(user_data);
				break;
			case Disp_Baidu:
				move_task_baiduai(user_data);
				break;
			default:
				break;
			}
		}
		else if (user_data == BT1_DOUBLE || user_data == BT3_DOUBLE)
		{
			switch (page.NowPage)
			{
			case Disp_Game_2048:
				move_task_game_2048(user_data);
				break;
			case Disp_Game_Snake:
				move_task_game_snake(user_data);
				break;
			default:
				break;
			}
		}
		else if (user_data == BT2_LONG)
		{
			if (page.NowPage == Disp_Menu || page.NowPage == Disp_Home)
			{
				static uint8_t set = 1;
				if (page.NowPage == Disp_Home)
				{
					ksdiy_sys_t.state.sys_button = 1; //使用lvgl按键机制
					page.PagePush(Disp_Menu);
					set = 0;
				}
				else if (page.NowPage == Disp_Menu)
				{
					ksdiy_sys_t.state.sys_button = 0; //退出lvgl按键机制
					page.PagePop();
					set = 1;
				}
			}
			else
			{
				cam_en = 0, color_en = 0, face_en = 0;
				page.PagePop();
			}
		}
		printf("page.NowPage: %d\n", page.NowPage);
	}
}
static void lv_tick_task(void *arg)
{
	(void)arg;
	lv_tick_inc(10);
}
void button_task(void *arg)
{
	Button_Init();
	while (1)
	{
		Button_Process();
		vTaskDelay(20 / portTICK_PERIOD_MS);
	}
}
SemaphoreHandle_t xGuiSemaphore;

static void gui_task(void *arg)
{
	xGuiSemaphore = xSemaphoreCreateMutex();
	lv_init(); // lvgl内核初始化

	lvgl_driver_init(); // lvgl显示接口初始化
	//申请两个buffer给刷屏用
	/*外部PSRAM方式*/
	lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
	lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

	/*内部DMA方式*/
	// lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
	// lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);

	// static lv_color_t buf1[DISP_BUF_SIZE];
	// static lv_color_t buf2[DISP_BUF_SIZE];
	static lv_disp_buf_t disp_buf;
	uint32_t size_in_px = DISP_BUF_SIZE;//KV_CONF.H 改 LVGL  尺寸。
	lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);//初始化显示缓冲区

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = disp_driver_flush;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);
    //初始化触摸板
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	// indev_drv.read_cb = touch_driver_read;
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	lv_indev_drv_register(&indev_drv);

	// esp_register_freertos_tick_hook(lv_tick_task);
	/* Create and start a periodic timer interrupt to call lv_tick_inc */
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &lv_tick_task,
		.name = "periodic_gui"};
	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));
	//按键初始化
	lv_port_indev_init();

	// lv_port_fs_init();
	lv_png_init();
	lv_split_jpeg_init();

	// LV_IMG_DECLARE(png_decoder_test);
	// lv_obj_t * img = lv_img_create(lv_scr_act(),NULL);
	// lv_img_set_src(img, &png_decoder_test);

	/* Set src of image with file name */
	// lv_img_set_src(img, "/spiffs/imgbg.png");

	// lv_obj_t * img = lv_gif_create_from_data(lv_scr_act(), example_gif_map);
	// lv_demo_widgets();
	// lv_demo_music();
	// lv_demo_benchmark();
	// page_wakeup_start();
	// vTaskDelay(pdMS_TO_TICKS(1000));
	// page_wakeup_end();
	// page_cam_start();
	page_init();
	// Display_Init();
	while (1)
	{
		/* Delay 1 tick (assumes FreeRTOS tick is 10ms */
		vTaskDelay(pdMS_TO_TICKS(10));
		page_switch();
		/* Try to take the semaphore, call lvgl related function on success */
		if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
		{
		 	 page.Running();
			lv_task_handler();
			xSemaphoreGive(xGuiSemaphore);
		}
	}
}
/*显示spiffs的所有文件名*/
static void SPIFFS_Directory(char *path)
{
	DIR *dir = opendir(path);
	assert(dir != NULL);
	while (true)
	{
		struct dirent *pe = readdir(dir);
		if (!pe)
			break;
		ESP_LOGI(__FUNCTION__, "d_name=%s d_ino=%d d_type=%x", pe->d_name, pe->d_ino, pe->d_type);
	}
	closedir(dir);
}

void app_main(void)
{
	/*初始化spiffs用于存放字体文件或者图片文件或者网页文件*/
	ESP_LOGI(TAG, "Initializing SPIFFS");
	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 8,
		.format_if_mount_failed = true};
	esp_err_t ret = esp_vfs_spiffs_register(&conf);
	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		else if (ret == ESP_ERR_NOT_FOUND)
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		else
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		return;
	}
	/*显示spiffs里的文件列表*/
	SPIFFS_Directory("/spiffs/");

	// 初始化nvs用于存放wifi或者其他需要掉电保存的东西
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	s_wifi_event_group = xEventGroupCreate();
	/*初始化ADC引脚  用于ADC按键*/
	// adc_init();
	/*初始化WS2812 */
	app_led_init(GPIO_RMT_LED);

	/*关掉48引脚的IR输出 不然控制红外的MOS管会发烫*/
	gpio_pad_select_gpio(0);
	gpio_set_direction(0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(0, GPIO_PULLUP_ONLY);


	/*创建lvgl任务显示*/
	xTaskCreatePinnedToCore(&gui_task, "gui task", 1024 * 5, NULL, 5, NULL, 1);
	/*创建按键任务 定时扫描按键值*/
	xTaskCreatePinnedToCore(&button_task, "button_task", 1024 * 3, NULL, 8, NULL, 0);

	/*初始化红外遥控引脚*/
	IR_init();
	xTaskCreatePinnedToCore(&rmt_ir_txTask, "ir_tx", 1024 * 2, NULL, 5, &ir_tx_handle, 0);
	/*进入阻塞态等待连接*/
	EventBits_t uxBits = xEventGroupWaitBits(s_wifi_event_group, BIT0, false, false, portMAX_DELAY);
	/*初始化语音唤醒识别任务*/
	app_speech_wakeup_init();
	g_state = WAIT_FOR_WAKEUP;
	// printf("adc_value: %d\n", get_adc());
	//while (1)
	//{

		// get_adc_vol();
	//	page.Running();
		// lv_task_handler();
	//	vTaskDelay(10 / portTICK_PERIOD_MS);
	//}
}
