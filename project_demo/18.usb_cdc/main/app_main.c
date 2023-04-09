/*
 * @Descripttion :
 * @version      :
 * @Author       : Kevincoooool
 * @Date         : 2021-09-23 16:50:40
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-01-12 14:48:49
 * @FilePath: \18.usb_cdc\main\app_main.c
 */
/* USB Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdlib.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "sdkconfig.h"
#include "tusb_msc.h"
#include "tusb.h"

#include "nvs_flash.h"

static const char *TAG = "example";
#if CFG_TUD_CDC
#include "tusb_cdc_acm.h"
#define USB_RX_BUF_SIZE 1*1024
#define USB_TX_BUF_SIZE 1*1024

#define UART_RX_BUF_SIZE 1*1024
#define UART_TX_BUF_SIZE 1*1024
static uint32_t s_baud_rate_active = 115200;
static uint8_t s_stop_bits_active = 0;
static uint8_t s_parity_active = 0;
static uint8_t s_data_bits_active = 8;
void tinyusb_cdc_line_coding_changed_callback(int itf, cdcacm_event_t *event)
{
    uint32_t bit_rate = event->line_coding_changed_data.p_line_coding->bit_rate;
    uint8_t stop_bits = event->line_coding_changed_data.p_line_coding->stop_bits;
    uint8_t parity = event->line_coding_changed_data.p_line_coding->parity;
    uint8_t data_bits = event->line_coding_changed_data.p_line_coding->data_bits;
    ESP_LOGI(TAG, "host require bit_rate=%u stop_bits=%u parity=%u data_bits=%u", bit_rate, stop_bits, parity, data_bits);

    // if (s_baud_rate_active != bit_rate) {
    //     if (ESP_OK == uart_set_baudrate(BOARD_UART_PORT, CONFIG_BAUD_RATE(bit_rate))) {
    //         s_baud_rate_active = bit_rate;
    //         ESP_LOGW(TAG, "set bit_rate=%d", bit_rate);
    //     }
    // }

    // if (s_stop_bits_active != stop_bits) {
    //     if (ESP_OK == uart_set_stop_bits(BOARD_UART_PORT, CONFIG_STOP_BITS(stop_bits))) {
    //         s_stop_bits_active = stop_bits;
    //         ESP_LOGW(TAG, "set stop_bits=%s", STR_STOP_BITS(stop_bits));
    //     }
    // }

    // if (s_parity_active != parity) {
    //     if (ESP_OK == uart_set_parity(BOARD_UART_PORT, CONFIG_PARITY(parity))) {
    //         s_parity_active = parity;
    //         ESP_LOGW(TAG, "set parity=%s", STR_PARITY(parity));
    //     }
    // }

    // if (s_data_bits_active != data_bits) {
    //     if (ESP_OK == uart_set_word_length(BOARD_UART_PORT, CONFIG_DATA_BITS(data_bits))) {
    //         s_data_bits_active = data_bits;
    //         ESP_LOGW(TAG, "set data_bits=%s", STR_DATA_BITS(data_bits));
    //     }
    // }
}
void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;
    ESP_LOGI(TAG, "Line state changed! dtr:%d, rts:%d ", dtr, rts);
}
void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    /* initialization */
    size_t rx_size = 0;
    uint8_t rx_buf[USB_RX_BUF_SIZE];
    /* read from usb */
    esp_err_t ret = tinyusb_cdcacm_read(itf, rx_buf, USB_RX_BUF_SIZE, &rx_size);

    if (ret == ESP_OK)
    { // TODO:may time out
        // size_t xfer_size = uart_write_bytes(BOARD_UART_PORT, rx_buf, rx_size);
        // assert(xfer_size == rx_size);
        ESP_LOGI(TAG, "uart write data (%d bytes): %s", rx_size, rx_buf);
        for (uint32_t i = 0; i < rx_size; i++)
        {
            tud_cdc_write_char(rx_buf[i]);

            if (rx_buf[i] == '\r')
            {
                tud_cdc_write_str("\n > ");
            }
        }
        tud_cdc_write_flush();
    }
    else
    {
        ESP_LOGE(TAG, "usb read error");
    }
}
#endif


void app_main(void)
{


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "USB initialization");

    tinyusb_config_t tusb_cfg = {
        .descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false // In the most cases you need to use a `false` value
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

#if CFG_TUD_CDC
    // // xTaskCreate(msc_task, "msc", 4096, NULL, 8, NULL);
    tinyusb_config_cdcacm_t amc_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = USB_RX_BUF_SIZE,
        .callback_rx = &tinyusb_cdc_rx_callback, // the first way to register a callback
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = &tinyusb_cdc_line_state_changed_callback,
        .callback_line_coding_changed = &tinyusb_cdc_line_coding_changed_callback};

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&amc_cfg));
#endif

}
