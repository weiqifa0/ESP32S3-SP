/*
 * @Descripttion :
 * @version      :
 * @Author       : Kevincoooool
 * @Date         : 2021-05-25 16:03:28
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-04 18:27:08
 * @FilePath     : \s3_lvgl_v7\main\app_speech_recsrc.c
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
// #include "esp_ns.h"
// #include "esp_agc.h"

#define I2S_NUM 1

void record_i2s_init(void)
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,        // the mode must be set according to DSP configuration
        .sample_rate = 16000,                         // must be the same as DSP configuration
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // must be the same as DSP configuration
        .bits_per_sample = 32,                        // must be the same as DSP configuration
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 2,
        .dma_buf_len = 300,
        .intr_alloc_flags = ESP_INTR_FLAG_LOWMED,
        .bits_per_chan = I2S_BITS_PER_SAMPLE_16BIT};
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
}
void all_i2s_deinit(void)
{
    i2s_driver_uninstall(I2S_NUM);
    vTaskDelay(10);
}
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

void recsrcTask(void *arg)
{
    // play_i2s_init();
    record_i2s_init();

    src_cfg_t *cfg = (src_cfg_t *)arg;
    size_t samp_len = cfg->item_size * 2 * sizeof(int) / sizeof(int16_t);

    int *samp = malloc(samp_len);

    size_t read_len = 0;


    while (1)
    {

        if (playing != true)
        {
            // tips_i2s_read();
            i2s_read(I2S_NUM, samp, samp_len, &read_len, portMAX_DELAY);
            for (int x = 0; x < cfg->item_size / 4; x++)
            {
                int s1 = ((samp[x * 4] + samp[x * 4 + 1]) >> 13) & 0x0000FFFF;
                int s2 = ((samp[x * 4 + 2] + samp[x * 4 + 3]) << 3) & 0xFFFF0000;
                samp[x] = s1 | s2;
            }
            // esp_agc_process(agc_inst, samp, agc_out,samp_len, 16000);
            xQueueSend(*cfg->queue, samp, portMAX_DELAY);
        }
        else
        {
            // tips_sleep();
            memset(samp, 0, samp_len);
            vTaskDelay(10);
        }
    }

    vTaskDelete(NULL);
}
