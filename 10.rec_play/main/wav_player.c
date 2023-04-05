/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-05-25 09:20:06
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-06-15 15:39:29
 * @FilePath: \S3_DEMO\26.rec_and_play\main\wav_player.c
 */
#include "wav_player.h"
#include "file_manager.h"
#include "driver/i2s.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#define TAG "WAV_PLAYER"
/*
    卸载I2S驱动
*/
void all_i2s_deinit(void)
{
    i2s_driver_uninstall(I2S_NUM);
    vTaskDelay(10);
}
/*
    录音I2S初始化
*/
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
char path_buf[256] = {0};
char **g_file_list = NULL;
uint16_t g_file_num = 0;
bool playing = false;


esp_err_t play_wav(const char *filepath)
{

	// all_i2s_deinit();//先把i2s的驱动卸载掉 因为在这之前是把i2s初始化为录音了 现在要播放 公用的引脚就不能播放  需要卸载掉重新初始化为播放模式

	// play_i2s_init();//初始化播放模式的i2s

	FILE *fd = NULL;
	struct stat file_stat;

	if (stat(filepath, &file_stat) == -1)//先找找这个文件是否存在
	{
		ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
		all_i2s_deinit();
		record_i2s_init();
		return ESP_FAIL;//如果不存在就继续录音
	}

	ESP_LOGI(TAG, "file stat info: %s (%ld bytes)...", filepath, file_stat.st_size);
	fd = fopen(filepath, "r");

	if (NULL == fd)
	{
		ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
		all_i2s_deinit();
		record_i2s_init();
		return ESP_FAIL;
	}
	const size_t chunk_size = 4096;
	uint8_t *buffer = malloc(chunk_size);

	if (NULL == buffer)
	{
		ESP_LOGE(TAG, "audio data buffer malloc failed");
		all_i2s_deinit();
		record_i2s_init();
		fclose(fd);
		return ESP_FAIL;
	}

	/**
	 * read head of WAV file
	 */
	wav_header_t wav_head;
	int len = fread(&wav_head, 1, sizeof(wav_header_t), fd);//读取wav文件的文件头
	if (len <= 0)
	{
		ESP_LOGE(TAG, "Read wav header failed");
		all_i2s_deinit();
		record_i2s_init();
		fclose(fd);
		return ESP_FAIL;
	}
	if (NULL == strstr((char *)wav_head.Subchunk1ID, "fmt") &&
		NULL == strstr((char *)wav_head.Subchunk2ID, "data"))
	{
		ESP_LOGE(TAG, "Header of wav format error");
		all_i2s_deinit();
		record_i2s_init();
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
	i2s_set_clk(I2S_NUM, wav_head.SampleRate, wav_head.BitsPerSample, 1);//根据该wav文件的各种参数来配置一下i2S的clk 采样率等等
	ESP_LOGI(TAG, "write data");
	do
	{
		/* Read file in chunks into the scratch buffer */
		len = fread(buffer, 1, chunk_size, fd);
		if (len <= 0)
		{
			break;
		}
		i2s_write(I2S_NUM, buffer, len, &cnt, 1000 / portTICK_PERIOD_MS);//输出数据到I2S  就实现了播放
		write_num += len;
	} while (1);
	fclose(fd);
	ESP_LOGI(TAG, "File reading complete, total: %d bytes", write_num);

	all_i2s_deinit();
	// record_i2s_init();

	return ESP_OK;
}
void play_sdfile_name(char *file_name)
{

	fm_sdcard_init();
	fm_print_dir("/sdcard", 2);

	play_wav(file_name);
	sd_unmount();
}
