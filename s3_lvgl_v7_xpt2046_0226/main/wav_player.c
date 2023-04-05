/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-05-25 09:20:06
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-11-11 16:05:27
 * @FilePath     : \s3_lvgl_v7\main\app_camera.c
 */
#include "wav_player.h"
#include "file_manager.h"
#include "driver/i2s.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "app_speech_srcif.h"

#define TAG "WAV_PLAYER"

char path_buf[256] = {0};
char **g_file_list = NULL;
uint16_t g_file_num = 0;
bool playing = false;
typedef struct
{
	// The "RIFF" chunk descriptor
	uint8_t ChunkID[4];
	int32_t ChunkSize;
	uint8_t Format[4];
	// The "fmt" sub-chunk
	uint8_t Subchunk1ID[4];
	int32_t Subchunk1Size;
	int16_t AudioFormat;
	int16_t NumChannels;
	int32_t SampleRate;
	int32_t ByteRate;
	int16_t BlockAlign;
	int16_t BitsPerSample;
	// The "data" sub-chunk
	uint8_t Subchunk2ID[4];
	int32_t Subchunk2Size;
} wav_header_t;

esp_err_t play_wav(const char *filepath)
{

	all_i2s_deinit();
	play_i2s_init();
	FILE *fd = NULL;
	struct stat file_stat;

	if (stat(filepath, &file_stat) == -1)
	{
		ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
		all_i2s_deinit();
		record_i2s_init();
		return ESP_FAIL;
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
	int len = fread(&wav_head, 1, sizeof(wav_header_t), fd);
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
	i2s_set_clk(1, wav_head.SampleRate, wav_head.BitsPerSample, 1);
	ESP_LOGI(TAG, "write data");
	do
	{
		/* Read file in chunks into the scratch buffer */
		len = fread(buffer, 1, chunk_size, fd);
		if (len <= 0)
		{
			break;
		}
		i2s_write(1, buffer, len, &cnt, 1000 / portTICK_PERIOD_MS);
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
	playing = true;

	fm_sdcard_init();
	fm_print_dir("/sdcard", 2);
	fm_file_table_create(&g_file_list, &g_file_num, ".WAV");
	for (size_t i = 0; i < g_file_num; i++)
	{
		ESP_LOGI(TAG, "have file [%d:%s]", i, g_file_list[i]);
	}
	if (0 == g_file_num)
	{
		ESP_LOGW(TAG, "Can't found any wav file in sdcard!");
		all_i2s_deinit();
		record_i2s_init();
		sd_unmount();
		playing = false;
		return;
	}
	sprintf(path_buf, "%s/%s", "/sdcard", file_name);
	play_wav(path_buf);
	sd_unmount();
	fm_file_table_free(&g_file_list, g_file_num);
    record_i2s_init();
    vTaskDelay(10);
	playing = false;
}
void play_spiffs_name(char *file_name)
{
	playing = true;

	sprintf(path_buf, "%s/%s", "/spiffs/wav", file_name);
	play_wav(path_buf);
	record_i2s_init();
    vTaskDelay(10);
	playing = false;
	vTaskDelay(10);
}