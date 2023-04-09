/*
 * @Descripttion :
 * @version      :
 * @Author       : Kevincoooool
 * @Date         : 2021-05-25 16:03:28
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-03-09 14:11:19
 * @FilePath: \S3_DEMO\8.speech_rec\main\app_speech_recsrc.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "xtensa/core-macros.h"
#include "esp_partition.h"
#include "app_speech_srcif.h"
#include "driver/i2s.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "app_main.h"

#include "dl_lib_coefgetter_if.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"

extern esp_afe_sr_iface_t *afe_handle;

// #include "esp_ns.h"
// #include "esp_agc.h"

#define I2S_NUM 1
/*
    录音I2S初始化
*/
void record_i2s_init(void)
{
    // i2s_config_t i2s_config = {
    //     .mode = I2S_MODE_MASTER | I2S_MODE_RX,        // the mode must be set according to DSP configuration
    //     .sample_rate = 16000,                         // must be the same as DSP configuration
    //     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // must be the same as DSP configuration
    //     .bits_per_sample = 32,                        // must be the same as DSP configuration
    //     .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    //     .dma_buf_count = 2,
    //     .dma_buf_len = 300,
    //     .intr_alloc_flags = ESP_INTR_FLAG_LOWMED,
    //     .bits_per_chan = I2S_BITS_PER_SAMPLE_16BIT};
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // the mode must be set according to DSP configuration
        .sample_rate = 16000,                                // must be the same as DSP configuration
        .bits_per_sample = (i2s_bits_per_sample_t)32,        // must be the same as DSP configuration
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,         // must be the same as DSP configuration
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = 3,
        .dma_buf_len = 300,
    };
    i2s_pin_config_t pin_config = {
        .mck_io_num = -1,
        .bck_io_num = IIS_SCLK, // IIS_SCLK
        .ws_io_num = IIS_LCLK,  // IIS_LCLK
        .data_out_num = -1,     // IIS_DSIN
        .data_in_num = IIS_DOUT // IIS_DOUT
    };
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM);
    printf("i2s init success...\r\n");
}
/*
    播放I2S初始化
*/
void play_i2s_init(void)
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 36000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LOWMED,
        .dma_buf_count = 2,
        .dma_buf_len = 300,
        .bits_per_chan = I2S_BITS_PER_SAMPLE_16BIT};
    i2s_pin_config_t pin_config = {
        .mck_io_num = -1,
        .bck_io_num = IIS_SCLK,   // IIS_SCLK
        .ws_io_num = IIS_LCLK,    // IIS_LCLK
        .data_out_num = IIS_DSIN, // IIS_DSIN
        .data_in_num = -1         // IIS_DOUT
    };
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM);
}
/*
    卸载I2S驱动
*/
void all_i2s_deinit(void)
{
    i2s_driver_uninstall(I2S_NUM);
    vTaskDelay(10);
}
extern bool playing;

static void tips_i2s_read(void)
{
    static uint8_t i = 0;
    i++;
    if (i > 50)
    {
        i = 0;
        printf("i2s_reading...\r\n");
    }
}

static void tips_sleep(void)
{
    static uint8_t i = 0;
    i++;
    if (i > 20)
    {
        i = 0;
        printf("sleeping...\r\n");
    }
}
/*
    语音识别中的录音任务
*/
void recsrcTask(void *arg)
{
    // play_i2s_init();
    record_i2s_init(); //初始化I2S用于录音
    esp_afe_sr_data_t *afe_data = arg;
    int audio_chunksize = afe_handle->get_feed_chunksize(afe_data);
    int nch = afe_handle->get_channel_num(afe_data);
    size_t samp_len = audio_chunksize * 1;
    size_t samp_len_bytes = samp_len * sizeof(int32_t);
    int32_t *i2s_buff = (int32_t *)malloc(samp_len_bytes);
    assert(i2s_buff);
    size_t bytes_read;

    while (1)
    {

        // if (playing != true)
        // {
        // tips_i2s_read();
        // i2s_read(I2S_NUM, samp, samp_len, &read_len, portMAX_DELAY); //从I2S读取原始数据
        // for (int x = 0; x < cfg->item_size / 4; x++)                 //对原始数据进行处理只取其中一部分
        // {
        //     int s1 = ((samp[x * 4] + samp[x * 4 + 1]) >> 13) & 0x0000FFFF;
        //     int s2 = ((samp[x * 4 + 2] + samp[x * 4 + 3]) << 3) & 0xFFFF0000;
        //     samp[x] = s1 | s2;
        // }

        i2s_read(I2S_NUM, i2s_buff, samp_len_bytes, &bytes_read, portMAX_DELAY);

        for (int i = 0; i < samp_len; ++i)
        {
            i2s_buff[i] = i2s_buff[i] >> 14; // 32:8为有效位， 8:0为低8位， 全为0， AFE的输入为16位语音数据，拿29：13位是为了对语音信号放大。

            // int s1 = ((i2s_buff[i * 4] + i2s_buff[i * 4 + 1]) >> 13) & 0x0000FFFF;
            // int s2 = ((i2s_buff[i * 4 + 2] + i2s_buff[i * 4 + 3]) << 3) & 0xFFFF0000;
            // i2s_buff[i] = s1 | s2;

        }

        // esp_agc_process(agc_inst, samp, agc_out,samp_len, 16000);
        // xQueueSend(*cfg->queue, i2s_buff, portMAX_DELAY); //把处理完成的数据发送到识别任务的队列
        afe_handle->feed(afe_data, (int16_t *)i2s_buff);

        // }
        // else
        // {
        //     // tips_sleep();
        //     // memset(samp, 0, samp_len);
        //     memset(i2s_buff, 0, samp_len);
        //     vTaskDelay(10);
        // }
    }
    afe_handle->destroy(afe_data);
    vTaskDelete(NULL);
}
