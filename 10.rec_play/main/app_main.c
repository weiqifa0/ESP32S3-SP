

/* I2S Example
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wav_player.h"
#include "file_manager.h"
TaskHandle_t Task1Task_Handler; //任务句柄
static const char *TAG = "i2s_recorder";
#define NUM_CH (2)
#define REC_SECONDS (10)
#define SAMPLE_RATE (48000)
#define I2S_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define FILE_BITS_PER_SAMPLE (16)
#define RECBUF_SIZE (SAMPLE_RATE * (FILE_BITS_PER_SAMPLE / 8) * NUM_CH) // 1 second
#define NUM_RECBUFS (2)
#define MAX_SAMPLES (256)
// #define I2S_NUM (1)
#define I2S_BCK_IO (GPIO_NUM_16)
#define I2S_WS_IO (GPIO_NUM_7)
#define I2S_DO_IO (I2S_PIN_NO_CHANGE)
#define I2S_DI_IO (GPIO_NUM_15)
#define MOUNT_POINT "/sdcard"

// DMA channel to be used by the SPI peripheral
#ifndef SPI_DMA_CHAN
#define SPI_DMA_CHAN SPI_DMA_CH_AUTO
#endif // SPI_DMA_CHAN
// Pin mapping when using SPI mode.
// With this mapping, SD card can be used both in SPI and 1-line SD mode.
// Note that a pull-up on CS line is required in SD mode.
#define PIN_NUM_MISO 14
#define PIN_NUM_MOSI 4
#define PIN_NUM_CLK 3
#define PIN_NUM_CS 8

#define PRO_CPU 0
#define APP_CPU 1
uint8_t *wav_mem_play = NULL;
xTaskHandle TaskHandle_Task1;
uint8_t rec_ok = 0;
void i2s_init(void);
void sd_init(void);
void sd_deinit(void);
void i2s_task(void *pvParameters);
void read_task(void *pvParameters);
void sd_task(void *pvParameters);
void get_timestamps(int *seconds, char *datetime, size_t datetime_size);

sdmmc_card_t *card;
const char mount_point[] = MOUNT_POINT;
QueueHandle_t queue;
QueueHandle_t read_queue;
void *buffer[NUM_RECBUFS];

// structure of a command on the msg q
typedef struct qm
{
    /* data */
    char filename[128]; // this is just the timestamp part
    int seqno;
    void *buffer;
    size_t len;
} q_msg;

// structure of a WAV file header
// WAV header spec information:
// https://web.archive.org/web/20140327141505/https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
// http://www.topherlee.com/software/pcm-tut-wavformat.html

typedef struct wav_header
{
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    uint32_t wav_size;   // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4];      // Contains "fmt " (includes trailing space)
    uint32_t fmt_chunk_size; // Should be 16 for PCM
    uint16_t audio_format;   // Should be 1 for PCM. 3 for IEEE Float
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;        // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    uint16_t sample_alignment; // num_channels * Bytes Per Sample
    uint16_t bit_depth;        // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    uint32_t data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} wav_header;

wav_header wav_hdr = {
    "RIFF",
    0,
    "WAVE",
    "fmt ",
    16,
    1,
    2,
    SAMPLE_RATE,
    SAMPLE_RATE * 2 * 2,
    2 * 2,
    16,
    "data",
    0};

void app_main(void)
{
    // Allocate from PSRAM the buffer pages we will use to grab record data
    for (int i = 0; i < NUM_RECBUFS; i++)
    {
        if ((buffer[i] = malloc(RECBUF_SIZE)) == NULL)
        {
            ESP_LOGE(TAG, "Failed to allocate a record buffer.");
        }
        ESP_LOGI(TAG, "Allocated %d bytes at 0x%08X", RECBUF_SIZE, (uint32_t)buffer[i]);
    }

    // Allocate a queue, with max depth corresponding to the number of buffers
    queue = xQueueCreate(NUM_RECBUFS, sizeof(q_msg));
    if (queue == 0)
    {
        // Queue was not created and must not be used.
        ESP_LOGE(TAG, "Failed to allocate a queue.");
    }

    // Create two tasks on different cores:
    // 1. Dedicated to reading data from I2S, higher priority
    // 2. Dedicated to writing data to SD card, lower priority
    xTaskCreatePinnedToCore(i2s_task, "i2s_task", 8192, NULL, 2, &Task1Task_Handler, APP_CPU);
    xTaskCreatePinnedToCore(sd_task, "sd_task", 8192, NULL, 1, NULL, PRO_CPU);
}

void sd_task(void *pvParameters)
{
    ESP_LOGI(TAG, "sd_task, starting up.");
    static char prev_filename[256];
    static uint32_t audio_bytes = 0;
    int rc;
    char filename[256];
    sd_init();
    // fm_sdcard_init();
    BaseType_t qrc;
    q_msg m;

    size_t written;
    FILE *f;
    while (true)
    {

        // Read a command from the queue, wait for up to 2 seconds
        while ((qrc = xQueueReceive(queue, (void *)&m, 2000 / portTICK_PERIOD_MS)) != pdTRUE)
        {
            // There's nothing on the queue, log an info, and try again
            ESP_LOGI(TAG, "sd_task: nothing on queue.");
        }

        // Now we have got a queue element, write the buffer to disk
        // The filename is in the Q msg, less the .raw suffix
        sprintf(filename, "%s/%s.wav", MOUNT_POINT, m.filename);

        // If the seqno indicates this is the first buffer of a new file
        // and we wrote something to the previous file, finish it off by adding
        // number of bytes in the file.
        //录音最后一帧
        if (m.seqno == REC_SECONDS && audio_bytes > 0)
        {
            wav_header new_hdr = wav_hdr;
            new_hdr.wav_size = audio_bytes + 36;
            new_hdr.data_bytes = audio_bytes;
            ESP_LOGI(
                TAG,
                "sd_task: file: %s, chunk: %d, subchunk2: %d",
                prev_filename,
                new_hdr.wav_size,
                new_hdr.data_bytes);
            FILE *prev = fopen(prev_filename, "r+");
            if (prev == NULL)
            {
                ESP_LOGE(TAG, "sd_task: Failed to open file, %s", prev_filename);
            }
            else if ((rc = fseek(prev, 0, SEEK_SET)) != 0)
            {
                ESP_LOGE(TAG, "sd_task: Failed to fseek(), rc: %d", rc);
            }
            else if ((written = fwrite(
                          (const void *)&new_hdr,
                          sizeof new_hdr,
                          1,
                          prev)) < 1)
            {
                ESP_LOGE(
                    TAG,
                    "sd_task: Failed to rewrite WAV header, tried: %d, wrote: %d, %s",
                    sizeof new_hdr,
                    written,
                    strerror(errno));
            }
            else
            {
                ESP_LOGI(TAG, "sd_task: rewrote WAV header");
            }
            fclose(prev);
            break;
        }

        // Save the filename written in this pass, so we can later update it
        strncpy(prev_filename, filename, sizeof prev_filename);

        // If the seqno indicates this is the first buffer of a new file then
        // write a WAV file header.
        //第一帧
        if (m.seqno == 0 || audio_bytes == 0)
        {

            // New file, truncate it 创建一个新文件然后写入wav头
            f = fopen(filename, "w");
            if (f == NULL)
            {
                ESP_LOGE(TAG, "sd_task: Failed to open new file, %s", filename);
                continue;
            }

            audio_bytes = 0;
            if (fwrite((void *)&wav_hdr, 1, sizeof wav_hdr, f) < sizeof wav_hdr)
            {
                ESP_LOGE(TAG, "sd_task: Failed to write WAV header");
                fclose(f);
                continue;
            }
            else
            {
                ESP_LOGI(TAG, "sd_task: Wrote WAV header");
            }
        }
        else
        {

            // Not a new file, open it for append 不是新文件，继续打开添加数据
            f = fopen(filename, "a");
            if (f == NULL)
            {
                ESP_LOGE(TAG, "sd_task: Failed to reopen file, %s", filename);
                continue;
            }
        }

        if ((written = fwrite(m.buffer, 1, m.len, f)) < m.len)
        {
            ESP_LOGE(
                TAG,
                "sd_task: Failed to write all samples, len=%d, written=%d",
                m.len,
                written);
            audio_bytes += written;
            fclose(f);
            continue;
        }
        else
        {
            ESP_LOGI(
                TAG,
                "sd_task: Wrote file: %s, seqno: %d, bytes: %d",
                filename,
                m.seqno,
                written);
            audio_bytes += written;
            fclose(f);
        }
    }
    rec_ok = 1;
    vTaskSuspend(Task1Task_Handler);
    printf("rec over!,ready to play\n");

    i2s_driver_uninstall(I2S_NUM);

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

    FILE *fd = NULL;
    struct stat file_stat;

    if (stat(filename, &file_stat) == -1) //先找找这个文件是否存在
    {
        ESP_LOGE(TAG, "Failed to stat file : %s", filename);
        return ESP_FAIL; //如果不存在就继续录音
    }

    ESP_LOGI(TAG, "file stat info: %s (%ld bytes)...", filename, file_stat.st_size);
    wav_mem_play = malloc(file_stat.st_size);
    fd = fopen(filename, "r");

    if (NULL == fd)
    {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filename);

        return ESP_FAIL;
    }
    // const size_t chunk_size = 4096 * 2;
    // uint8_t *buffer = malloc(chunk_size);

    // if (NULL == buffer)
    // {
    //     ESP_LOGE(TAG, "audio data buffer malloc failed");
    //     fclose(fd);
    //     return ESP_FAIL;
    // }

    /**
     * read head of WAV file
     */
    wav_header_t wav_head;
    int len = fread(&wav_head, 1, sizeof(wav_header_t), fd); //读取wav文件的文件头
    if (len <= 0)
    {
        ESP_LOGE(TAG, "Read wav header failed");
        fclose(fd);
        return ESP_FAIL;
    }
    if (NULL == strstr((char *)wav_head.Subchunk1ID, "fmt") &&
        NULL == strstr((char *)wav_head.Subchunk2ID, "data"))
    {
        ESP_LOGE(TAG, "Header of wav format error");
        fclose(fd);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "frame_rate=%d, ch=%d, width=%d", wav_head.SampleRate, wav_head.NumChannels, wav_head.BitsPerSample);
    /**
     * read wave data of WAV file
     */
    size_t write_num = 0;
    size_t cnt;
    ESP_LOGI(TAG, "set clock");
    i2s_set_clk(I2S_NUM, wav_head.SampleRate, wav_head.BitsPerSample, NUM_CH); //根据该wav文件的各种参数来配置一下i2S的clk 采样率等等
    ESP_LOGI(TAG, "write data");
    // xTaskCreatePinnedToCore(read_task, "read_task", 8192, NULL, 3, NULL, APP_CPU);
    fread(wav_mem_play, 1, file_stat.st_size, fd);
    for (size_t i = 0; i < file_stat.st_size; i++)
    {
        wav_mem_play[i]*=5;
    }
    
    len = 4096*2;
    do
    {
        /* Read file in chunks into the scratch buffer */
        // len = fread(buffer, 1, chunk_size, fd);//边读边写太耗时
        // if (len <= 0)
        if(write_num >= file_stat.st_size)
        {
            break;
        }
        i2s_write(I2S_NUM, wav_mem_play+write_num, len, &cnt, 1000 / portTICK_PERIOD_MS); //输出数据到I2S  就实现了播放
        write_num += len;
    } while (1);
    fclose(fd);
    ESP_LOGI(TAG, "File reading complete, total: %d bytes", write_num);
    free(wav_mem_play);
        i2s_zero_dma_buffer(I2S_NUM);
    i2s_driver_uninstall(I2S_NUM);
    sd_unmount();
    while (1)
    {

        vTaskDelay(1000);
    }
}

void i2s_task(void *pvParameters)
{
    ESP_LOGI(TAG, "i2s_task, starting up.");

    // Initialise the I2S bus
    i2s_init();

    uint8_t buf_index = 0;
    size_t bytesRead = 0;

    esp_err_t rc;

    while (true)
    {
        // Loop reading from I2S and writing to a buffer

        // Request 1 second of data from the I2S bus, timeout after 1.5 seconds
        rc = i2s_read(I2S_NUM, buffer[buf_index], RECBUF_SIZE, &bytesRead, 1500 / portTICK_PERIOD_MS);

        if (rc != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "i2s_read(): rc=%d  bytes=%d, buf_index=%d\n",
                rc,
                bytesRead,
                buf_index);
        }
        else
        {
            ESP_LOGI(
                TAG,
                "i2s_read(): rc=%d  bytes=%d, buf_index=%d\n",
                rc,
                bytesRead,
                buf_index);
        }

        // Now enqueue a request for this to be written to the SD card
        q_msg m;
        BaseType_t qrc;
        get_timestamps(&m.seqno, m.filename, sizeof m.filename);

        m.seqno++;
        m.buffer = buffer[buf_index];
        m.len = bytesRead;

        if ((qrc = xQueueSend(queue, (void *)&m, (TickType_t)100)) != pdTRUE)
        {
            ESP_LOGE(TAG, "i2s: xQueueSend() failed: rc=%d", qrc);
        }

        // Move onto the next receive buffer
        buf_index = (buf_index + 1) % NUM_RECBUFS;
    }
}

void read_task(void *pvParameters)
{
    ESP_LOGI(TAG, "i2s_task, starting up.");

    uint8_t buf_index = 0;
    size_t bytesRead = 0;

    esp_err_t rc;

    while (true)
    {

        // if (rec_ok == 0)
        // {
        // Loop reading from I2S and writing to a buffer

        // Request 1 second of data from the I2S bus, timeout after 1.5 seconds
        rc = i2s_read(I2S_NUM, buffer[buf_index], RECBUF_SIZE, &bytesRead, 1500 / portTICK_PERIOD_MS);

        if (rc != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "i2s_read(): rc=%d  bytes=%d, buf_index=%d\n",
                rc,
                bytesRead,
                buf_index);
        }
        else
        {
            ESP_LOGI(
                TAG,
                "i2s_read(): rc=%d  bytes=%d, buf_index=%d\n",
                rc,
                bytesRead,
                buf_index);
        }

        // Now enqueue a request for this to be written to the SD card
        q_msg m;
        BaseType_t qrc;
        // if(m.seqno == 10)
        // m.seqno = 0;
        // m.seqno++;
        get_timestamps(&m.seqno, m.filename, sizeof m.filename);
        //  m.seqno = 0;
        m.seqno++;
        if (m.seqno > 10)
            m.seqno = 0;
        m.buffer = buffer[buf_index];
        m.len = bytesRead;

        if ((qrc = xQueueSend(queue, (void *)&m, (TickType_t)100)) != pdTRUE)
        {
            ESP_LOGE(TAG, "i2s: xQueueSend() failed: rc=%d", qrc);
        }

        // Move onto the next receive buffer
        buf_index = (buf_index + 1) % NUM_RECBUFS;
        // }
        // else
        //     vTaskDelete(NULL);
        // vTaskDelay(100);
    }
}
void sd_init(void)
{
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    ESP_LOGI(TAG, "Initializing SD card");

    // ESP_LOGI(TAG, "Using SPI peripheral");

    // sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    // spi_bus_config_t bus_cfg = {
    //     .mosi_io_num = PIN_NUM_MOSI,
    //     .miso_io_num = PIN_NUM_MISO,
    //     .sclk_io_num = PIN_NUM_CLK,
    //     .quadwp_io_num = -1,
    //     .quadhd_io_num = -1,
    //     .max_transfer_sz = 4000,
    // };
    // ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to initialize bus.");
    //     return;
    // }

    // // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    // sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    // slot_config.gpio_cs = PIN_NUM_CS;
    // slot_config.host_id = host.slot;

    // ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    // // Card has been initialized, print its properties
    // sdmmc_card_print_info(stdout, card);


      ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    slot_config.width = 1;

    // On chips where the GPIOs used for SD card can be configured, set them in
    // the slot_config structure:
    // #ifdef CONFIG_SOC_SDMMC_USE_GPIO_MATRIX
    slot_config.clk = PIN_NUM_CLK;
    slot_config.cmd = PIN_NUM_MOSI;
    slot_config.d0 = PIN_NUM_MISO;
    // #ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
    // slot_config.d1 = PIN_NUM_MOSI;
//     slot_config.d2 = CONFIG_EXAMPLE_PIN_D2;
//     slot_config.d3 = CONFIG_EXAMPLE_PIN_D3;
// #endif  // CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
// #endif  // CONFIG_SOC_SDMMC_USE_GPIO_MATRIX

    // Enable internal pullups on enabled pins. The internal pullups
    // are insufficient however, please make sure 10k external pullups are
    // connected on the bus. This is for debug / example purpose only.
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
}

void i2s_init()
{

    // ESP32 as slave seems to be prone to frame alignment errors eg samples
    // that should start 0xF start 0x7.
    // Switching instead to ESP32 as master using code from:
    // https://github.com/YetAnotherElectronicsChannel/ESP32_DSP_I2S_SETUP/blob/master/code/main/main.c

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // 2-channels I2S_CHANNEL_FMT_ALL_RIGHT I2S_CHANNEL_FMT_RIGHT_LEFT
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 4,
        .dma_buf_len = MAX_SAMPLES,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
        .tx_desc_auto_clear = true,
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO 
    };
    if (ESP_OK != i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL))
    {
        printf("i2s_driver_install: error");
    }
    if (ESP_OK != i2s_set_pin(I2S_NUM, &pin_config))
    {
        printf("i2s_set_pin: error");
    }

    // enable MCLK on GPIO0
    //  REG_WRITE(PIN_CTRL, 0xFF0);
    //  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
}

// Utility to populate time variables
void get_timestamps(int *seconds, char *datetime, size_t datetime_size)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    // Set timezone to Universal Cooordinated Time
    setenv("TZ", "UTC", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(datetime, datetime_size, "%Y%m%d-%H%M", &timeinfo);
    // static uint16_t time = 0;
    // time+=1;
    // sprintf(datetime,"file-%d",time/10);
    // printf("file_name :%s", datetime);
    // *seconds = timeinfo.tm_sec;
}