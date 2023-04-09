/*
 * @Descripttion :
 * @version      :
 * @Author       : Kevincoooool
 * @Date         : 2021-05-25 16:03:28
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-03-18 17:54:07
 * @FilePath: \S3_DEMO\21.new_speech_rec\main\app_speech_wakeup.c
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
#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"

#include "page_wakeup.h"
#include "wav_player.h"

#include "dl_lib_coefgetter_if.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "model_path.h"

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

esp_afe_sr_iface_t *afe_handle = NULL;

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

            ESP_LOGI(TAG, "ID0_TIAODAOERSHIDU");
            lv_label_set_text(label_speech, "已调到20度");
            break;
        case ID1_TIAODAOERSHIYIDU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID1_TIAODAOERSHIYIDU");
            lv_label_set_text(label_speech, "已调到21度");
            break;
        case ID2_TIAODAOERSHIERDU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID2_TIAODAOERSHIERDU");
            lv_label_set_text(label_speech, "已调到22度");
            break;
        case ID3_TIAODAOERSHISANDU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID3_TIAODAOERSHISANDU");
            lv_label_set_text(label_speech, "已调到23度");
            break;
        case ID4_TIAODAOERSHISIDU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID4_TIAODAOERSHISIDU");
            lv_label_set_text(label_speech, "已调到24度");
            break;
        case ID5_TIAODAOERSHIWUDU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID5_TIAODAOERSHIWUDU");
            lv_label_set_text(label_speech, "已调到25度");
            break;
        case ID6_TIAODAOERSHLIUIDU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID6_TIAODAOERSHLIUIDU");
            lv_label_set_text(label_speech, "已调到26度");
            break;
        case ID7_TIAODAOERSHIQIDU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID7_TIAODAOERSHIQIDU");
            lv_label_set_text(label_speech, "已调到27度");
            break;
        case ID8_TIAODAOERSHIBADU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID8_TIAODAOERSHIBADU");
            lv_label_set_text(label_speech, "已调到28度");
            break;
        case ID9_QIDONGKONGTIAOSAOFENG:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID9_QIDONGKONGTIAOSAOFENG");
            lv_label_set_text(label_speech, "已打开扫风");
            break;
        case ID10_TINGZHIKONGTIAOSAOFENG:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID10_TINGZHIKONGTIAOSAOFENG");
            lv_label_set_text(label_speech, "已关闭扫风");
            break;
        case ID11_TIAODAOZIDONGFENGSU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID11_TIAODAOZIDONGFENGSU");
            break;
        case ID12_TIAODAOYIJIFENGSU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID12_TIAODAOYIJIFENGSU");
            break;
        case ID13_TIAODAOERJIFENGSU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID13_TIAODAOERJIFENGSU");
            break;
        case ID14_TIAODAOSANJIFENGSU:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID14_TIAODAOSANJIFENGSU");
            break;
        case ID15_DAKAIKONGTIAO:
            play_spiffs_name("airon.wav");

            ESP_LOGI(TAG, "ID15_DAKAIKONGTIAO");
            lv_label_set_text(label_speech, "已打开空调");

            break;
        case ID16_GUANBIKONGTIAO:
            play_spiffs_name("airoff.wav");

            ESP_LOGI(TAG, "ID16_GUANBIKONGTIAO");
            lv_label_set_text(label_speech, "已关闭空调");

            break;
        case ID17_GELIKONGTIAO:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID17_GELIKONGTIAO");
            lv_label_set_text(label_speech, "已设置为格力空调");
            break;
        case ID18_MEIDIKONGTIAO:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID18_MEIDIKONGTIAO");
            lv_label_set_text(label_speech, "已设置为美的空调");
            break;
        case ID19_AOKESIKONGTIAO:
            play_spiffs_name("haode.wav");

            ESP_LOGI(TAG, "ID19_AOKESIKONGTIAO");
            lv_label_set_text(label_speech, "已设置为奥克斯空调");
            break;
        case ID20_HAIERKONGTIAO:
            play_spiffs_name("haode.wav");

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

    esp_afe_sr_data_t *afe_data = arg;
    int afe_chunksize = afe_handle->get_fetch_chunksize(afe_data);
    int nch = afe_handle->get_channel_num(afe_data);
    int16_t *buff = (int16_t *)malloc(afe_chunksize * sizeof(int16_t));
    assert(buff);

    static const esp_mn_iface_t *multinet = &MULTINET_MODEL;
    model_iface_data_t *model_data = multinet->create((const model_coeff_getter_t *)&MULTINET_COEFF, 5760);
    int mu_chunksize = multinet->get_samp_chunksize(model_data);
    int chunk_num = multinet->get_samp_chunknum(model_data);
    assert(mu_chunksize == afe_chunksize);

    printf("------------detect start------------\n");
    ESP_LOGI(TAG, "Ready");

    int chunks = 0;
    int mn_chunks = 0;
    bool detect_flag = 0;

    while (1)
    {

        int res = afe_handle->fetch(afe_data, buff);
        if (res == AFE_FETCH_WWE_DETECTED)
        {
            ESP_LOGI(TAG, ">>> Say your command <<<");
            detect_flag = true;
            afe_handle->disable_wakenet(afe_data);
            page_wakeup_start();
            // play_spiffs_name("wozai.wav");
            detect_flag = 1;
            lv_label_set_text(label_speech, "我在听呢！\n请说命令词");
            printf("-----------------LISTENING-----------------\n\n");
            // self->notify();
        }
        if (detect_flag)
        {
            // Detect command
            int command = (command_word_t)multinet->detect(model_data, buff);
            // FatfsComboWrite(buff, afe_chunksize * sizeof(int16_t), 1, fp);

            if (command == COMMAND_NOT_DETECTED)
                continue;
            else if (command == COMMAND_TIMEOUT)
            {
                afe_handle->enable_wakenet(afe_data);
                // self->afe_handle->enable_aec(afe_data);

                detect_flag = false;
                ESP_LOGI(TAG, ">>> Waiting to be waken up <<<");
                playing = true;
                vTaskDelay(100);
                lv_label_set_text(label_speech, "刚刚没听清...");
                // play_spiffs_name("what.wav");
                printf("\n-----------awaits to be waken up-----------\n");
                page_wakeup_end();
                // self->notify();
            }
            else
            {
                // self->notify();
                ESP_LOGI(TAG, "Command: %d", command);
                playing = true;
                lv_label_set_text(label_speech, "好的");
                // play_spiffs_name("what.wav");
                printf("\n-----------awaits to be waken up-----------\n");
                vTaskDelay(500);
                page_wakeup_end();
#ifndef CONFIG_SR_MN_CN_MULTINET3_CONTINUOUS_RECOGNITION
                afe_handle->enable_wakenet(afe_data);
                // self->afe_handle->enable_aec(afe_data);
                detect_flag = false;
                ESP_LOGI(TAG, ">>> Waiting to be waken up <<<");
#endif
                command = COMMAND_TIMEOUT;
                // self->notify();
            }
        }
    }

    // free(buffer);
    vTaskDelete(NULL);
}

#include "esp_spiffs.h"
static void spiffs_init(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/srmodel",
        .partition_label = "model",
        .max_files = 5,
        .format_if_mount_failed = true};
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    size_t total = 0, used = 0;
    ret = esp_spiffs_info("model", &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}
#include "esp_vfs.h"
#include "esp_spiffs.h"
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
/*
    初始化语音识别任务
*/
void app_speech_wakeup_init()
{

    spiffs_init();
    SPIFFS_Directory("/srmodel/");
    afe_config_t afe_config = {
        .aec_init = true,
        .se_init = true,
        .vad_init = true,
        .wakenet_init = true,
        .vad_mode = 3,
        .wakenet_model = &WAKENET_MODEL,
        .wakenet_coeff = (const model_coeff_getter_t *)&WAKENET_COEFF,
        .wakenet_mode = DET_MODE_2CH_90,
        .afe_mode = SR_MODE_LOW_COST,
        .afe_perferred_core = 0,
        .afe_perferred_priority = 5,
        .afe_ringbuf_size = 50,
        .alloc_from_psram = AFE_PSRAM_MEDIA_COST,
        .agc_mode = 2,
    };
    afe_handle = &esp_afe_sr_1mic;

    afe_config.aec_init = false;
    afe_config.se_init = false;
    afe_config.vad_init = false;
    afe_config.afe_ringbuf_size = 10;
    esp_afe_sr_data_t *afe_data = afe_handle->create_from_config(&afe_config);

    xTaskCreatePinnedToCore(&recsrcTask, "recsrcTask", 4 * 1024, (void *)afe_data, 13, NULL, 0); // 创建录音任务

    xTaskCreatePinnedToCore(&nnTask, "nnTask", 5 * 1024, (void *)afe_data, 14, NULL, 1); // 创建识别任务
}
