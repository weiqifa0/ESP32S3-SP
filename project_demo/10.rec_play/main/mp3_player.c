/*
 * @Descripttion :  
 * @version      :  
 * @Author       : Kevincoooool
 * @Date         : 2021-05-25 09:20:06
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-04-26 14:50:33
 * @FilePath: \S3_DEMO\26.rec_and_play\main\mp3_player.c
 */
#include "mp3_player.h"
#include "file_manager.h"
#include "driver/i2s.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "mp3dec.h"

#define TAG "WAV_PLAYER"
#define I2S_NUM 0
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
/*
    卸载I2S驱动
*/
void all_i2s_deinit(void)
{
    i2s_driver_uninstall(I2S_NUM);
    vTaskDelay(10);

}
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

	all_i2s_deinit();//先把i2s的驱动卸载掉 因为在这之前是把i2s初始化为录音了 现在要播放 公用的引脚就不能播放  需要卸载掉重新初始化为播放模式
	play_i2s_init();//初始化播放模式的i2s
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
	i2s_set_clk(1, wav_head.SampleRate, wav_head.BitsPerSample, 1);//根据该wav文件的各种参数来配置一下i2S的clk 采样率等等
	ESP_LOGI(TAG, "write data");
	do
	{
		/* Read file in chunks into the scratch buffer */
		len = fread(buffer, 1, chunk_size, fd);
		if (len <= 0)
		{
			break;
		}
		i2s_write(1, buffer, len, &cnt, 1000 / portTICK_PERIOD_MS);//输出数据到I2S  就实现了播放
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


#define SAMPLE_RATE (44100)
#define I2S_NUM (0)
#define WAVE_FREQ_HZ (100)
#define PI (3.14159265)

#define SAMPLE_PER_CYCLE (SAMPLE_RATE / WAVE_FREQ_HZ)

xQueueHandle play_queue = NULL;

/*!< aduio music list from spiffs*/
const char audio_list[2][64] = {
    "/spiffs/apple-tosk.mp3",
    "/spiffs/distance.mp3",
    // "/spiffs/longest_movie.mp3",
};

enum
{
    AUDIO_STOP = 0,
    AUDIO_PLAY,
    AUDIO_NEXT,
    AUDIO_LAST
};

int play_flag = AUDIO_STOP;
int audio_play_index = 0;

void aplay_mp3(const char *path)
{
    ESP_LOGI(TAG, "start to decode %s", path);
    HMP3Decoder hMP3Decoder;
    MP3FrameInfo mp3FrameInfo;
    unsigned char *readBuf = malloc(MAINBUF_SIZE);

    if (readBuf == NULL)
    {
        ESP_LOGE(TAG, "readBuf malloc failed");
        return;
    }

    short *output = malloc(1153 * 4);

    if (output == NULL)
    {
        free(readBuf);
        ESP_LOGE(TAG, "outBuf malloc failed");
    }

    hMP3Decoder = MP3InitDecoder();

    if (hMP3Decoder == 0)
    {
        free(readBuf);
        free(output);
        ESP_LOGE(TAG, "memory is not enough..");
    }

    int samplerate = 0;
    i2s_zero_dma_buffer(0);
    FILE *mp3File = fopen(path, "rb");

    if (mp3File == NULL)
    {
        MP3FreeDecoder(hMP3Decoder);
        free(readBuf);
        free(output);
        ESP_LOGE(TAG, "open file failed");
    }

    char tag[10];
    int tag_len = 0;
    int read_bytes = fread(tag, 1, 10, mp3File);

    if (read_bytes == 10)
    {
        if (memcmp(tag, "ID3", 3) == 0)
        {
            tag_len = ((tag[6] & 0x7F) << 21) | ((tag[7] & 0x7F) << 14) | ((tag[8] & 0x7F) << 7) | (tag[9] & 0x7F);
            // ESP_LOGI(TAG,"tag_len: %d %x %x %x %x", tag_len,tag[6],tag[7],tag[8],tag[9]);
            fseek(mp3File, tag_len - 10, SEEK_SET);
        }
        else
        {
            fseek(mp3File, 0, SEEK_SET);
        }
    }

    int bytesLeft = 0;
    unsigned char *readPtr = readBuf;
    play_flag = AUDIO_PLAY;

    while (1)
    {
        switch (play_flag)
        {
        case AUDIO_STOP:
        {
            while (!play_flag)
            {
                i2s_zero_dma_buffer(0);
                vTaskDelay(100 / portTICK_RATE_MS);
                goto stop;
            }
            break;
        }
        break;

        case AUDIO_PLAY:
        {
        }
        break;

        case AUDIO_NEXT:
        {
            if (audio_play_index < 3 - 1)
            {
                audio_play_index++;
            }
            else
            {
                audio_play_index = 0;
            }

            goto stop;
        }
        break;

        case AUDIO_LAST:
        {
            if (audio_play_index > 0)
            {
                audio_play_index--;
            }
            else
            {
                audio_play_index = 3 - 1;
            }

            goto stop;
        }
        break;
        }

        if (bytesLeft < MAINBUF_SIZE)
        {
            memmove(readBuf, readPtr, bytesLeft);
            int br = fread(readBuf + bytesLeft, 1, MAINBUF_SIZE - bytesLeft, mp3File);

            if ((br == 0) && (bytesLeft == 0))
            {
                break;
            }

            bytesLeft = bytesLeft + br;
            readPtr = readBuf;
        }

        int offset = MP3FindSyncWord(readPtr, bytesLeft);

        if (offset < 0)
        {
            ESP_LOGE(TAG, "MP3FindSyncWord not find");
            bytesLeft = 0;
            continue;
        }
        else
        {
            readPtr += offset;   /*!< data start point */
            bytesLeft -= offset; /*!< in buffer */
            int errs = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, output, 0);

            if (errs != 0)
            {
                ESP_LOGE(TAG, "MP3Decode failed ,code is %d ", errs);
                break;
            }

            MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);

            if (samplerate != mp3FrameInfo.samprate)
            {
                samplerate = mp3FrameInfo.samprate;
                i2s_set_clk(0, samplerate, 16, mp3FrameInfo.nChans);
                ESP_LOGI(TAG, "mp3file info---bitrate=%d,layer=%d,nChans=%d,samprate=%d,outputSamps=%d", mp3FrameInfo.bitrate, mp3FrameInfo.layer, mp3FrameInfo.nChans, mp3FrameInfo.samprate, mp3FrameInfo.outputSamps);
            }

            size_t bytes_write = 0;
            i2s_write(0, (const char *)output, mp3FrameInfo.outputSamps * 2, &bytes_write, 100 / portTICK_RATE_MS);
            // rmt_write_items(0,(const char*)output,mp3FrameInfo.outputSamps*2, 1000 / portTICK_RATE_MS);
        }
    }

stop:
    i2s_zero_dma_buffer(0);
    MP3FreeDecoder(hMP3Decoder);
    free(readBuf);
    free(output);
    fclose(mp3File);

    ESP_LOGI(TAG, "end mp3 decode ..");
}

// static void audio_task(void *arg)
// {
//     /*!<  for 36Khz sample rates, we create 100Hz sine wave, every cycle need 36000/100 = 360 samples (4-bytes or 8-bytes each sample) */
//     /*!<  depend on bits_per_sample */
//     /*!<  using 6 buffers, we need 60-samples per buffer */
//     /*!<  if 2-channels, 16-bit each channel, total buffer is 360*4 = 1440 bytes */
//     /*!<  if 2-channels, 24/32-bit each channel, total buffer is 360*8 = 2880 bytes */
//     i2s_config_t i2s_config = {
//         .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX, /*!<  Only TX */
//         .sample_rate = SAMPLE_RATE,
//         .bits_per_sample = 16,
//         .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, /*!< 1-channels */
//         .communication_format = I2S_COMM_FORMAT_I2S,
//         .dma_buf_count = 6,
//         .dma_buf_len = 256,
//         .use_apll = true,
//         .tx_desc_auto_clear = true, /*!< I2S auto clear tx descriptor if there is underflow condition (helps in avoiding noise in case of data unavailability) */
//         .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2 | ESP_INTR_FLAG_IRAM,
//     };
//     i2s_pin_config_t pin_config = {
//         .bck_io_num = I2S_SCLK,
//         .ws_io_num = I2S_LCLK,
//         .data_out_num = I2S_DOUT,
//         .data_in_num = I2S_DSIN /*!< Not used */
//     };

//     i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
//     i2s_set_pin(I2S_NUM, &pin_config);

//     unsigned int mp3_index = 0;
//     while (1)
//     {
//         if (xQueueReceive(play_queue, &mp3_index, portTICK_RATE_MS))
//         {
//             printf("play_queue: mp3_index = %u\n", mp3_index);

//             if (mp3_index > (AUDIO_MAX_PLAY_LIST - 1))
//                 printf("mp3_index: error\n");
//             else
//             {
//                 aplay_mp3(audio_list[mp3_index]);
//                 vTaskDelay(1000 / portTICK_RATE_MS);
//             }
//         }
//     }
// }
void audio_play(uint8_t index)
{
	printf("play_queue: mp3_index = %d\n", index);
	aplay_mp3(audio_list[index]);
}
int audio_init(uint32_t vol)
{
    // es8311_init(SAMPLE_RATE);
    
    // if (vol > 100)
    //     vol = 100;

    ESP_LOGI(TAG, "设置音量(%d)", vol);
    // es8311_set_voice_volume(vol);
    // xTaskCreate(audio_task, "audio_task", 4096, NULL, 5, NULL);
    return 0;
}

// void audio_setting_vol(uint32_t vol)
// {
//     if (vol > 100)
//         vol = 100;

//     es8311_set_voice_volume(vol);
// }
