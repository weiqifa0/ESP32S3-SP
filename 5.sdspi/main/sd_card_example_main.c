/* SD card and FAT filesystem example.
   This example uses SPI peripheral to communicate with SD card.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_timer.h"
static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO GPIO_NUM_14
#define PIN_NUM_MOSI GPIO_NUM_4
#define PIN_NUM_CLK GPIO_NUM_3
#define PIN_NUM_CS GPIO_NUM_8
uint8_t write_data[100 * 1024] = {0x01};
uint8_t read_data[100 * 1024] = {0};

void app_main(void)
{
	esp_err_t ret;

	// Options for mounting the filesystem.
	// If format_if_mount_failed is set to true, SD card will be partitioned and
	// formatted in case when mounting fails.
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
		.format_if_mount_failed = true,
#else
		.format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
		.max_files = 5,
		.allocation_unit_size = 16 * 1024};
	sdmmc_card_t *card;
	const char mount_point[] = MOUNT_POINT;
	ESP_LOGI(TAG, "Initializing SD card");

	// Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	// Please check its source code and implement error recovery when developing
	// production applications.
	ESP_LOGI(TAG, "Using SPI peripheral");

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = PIN_NUM_MOSI,
		.miso_io_num = PIN_NUM_MISO,
		.sclk_io_num = PIN_NUM_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4000,
	};
	ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to initialize bus.");
		return;
	}

	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = PIN_NUM_CS;
	slot_config.host_id = host.slot;

	ESP_LOGI(TAG, "Mounting filesystem");
	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(TAG, "Failed to mount filesystem. "
						  "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		}
		else
		{
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
						  "Make sure SD card lines have pull-up resistors in place.",
					 esp_err_to_name(ret));
		}
		return;
	}
	ESP_LOGI(TAG, "Filesystem mounted");

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

	// Use POSIX and C standard library functions to work with files.

	// First create a file.
	const char *file_hello = MOUNT_POINT "/hello.txt";

	ESP_LOGI(TAG, "Opening file %s", file_hello);
	FILE *f = fopen(file_hello, "w");
	if (f == NULL)
	{
		ESP_LOGE(TAG, "Failed to open file for writing");
		return;
	}
	fprintf(f, "Hello %s!\n", card->cid.name);

	int64_t last_frame = esp_timer_get_time();
	ESP_LOGI(TAG, "开始写入100KB *100 开始时间：%u", (uint32_t)last_frame);

	for (size_t i = 0; i < 100; i++)
	{
		fwrite(write_data, sizeof(uint8_t), sizeof(write_data) / sizeof(uint8_t), f);
	}
	int64_t fr_end = esp_timer_get_time();
	int64_t frame_time = fr_end - last_frame;
	ESP_LOGI(TAG, "结束时间：%u 耗费时间：%u ms", (uint32_t)fr_end, (uint32_t)frame_time / 1000);

	ESP_LOGI(TAG, "写入完成，写入速度：%u KB/S", (100 * 1024 * 100) / ((uint32_t)frame_time / 1000));
	ESP_LOGI(TAG, "File written");
	fclose(f);
	const char *file_foo = MOUNT_POINT "/foo.txt";

	// Check if destination file exists before renaming
	struct stat st;
	if (stat(file_foo, &st) == 0)
	{
		// Delete it if it exists
		unlink(file_foo);
	}

	// Rename original file
	ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
	if (rename(file_hello, file_foo) != 0)
	{
		ESP_LOGE(TAG, "Rename failed");
		return;
	}

	// Open renamed file for reading
	ESP_LOGI(TAG, "Reading file %s", file_foo);
	f = fopen(file_foo, "r");
	if (f == NULL)
	{
		ESP_LOGE(TAG, "Failed to open file for reading");
		return;
	}
	// Read a line from file
	// char line[64];
	// fgets(line, sizeof(line), f);
	int64_t last_frame1 = esp_timer_get_time();
	ESP_LOGI(TAG, "开始读取 100KB *100 开始时间：%u", (uint32_t)last_frame1);

	for (size_t i = 0; i < 100; i++)
	{
		fread(read_data, sizeof(uint8_t), 100 * 1024, f);
	}
	int64_t fr_end1 = esp_timer_get_time();
	int64_t frame_time1 = fr_end1 - last_frame1;
	ESP_LOGI(TAG, "结束时间：%u 耗费时间：%u ms", (uint32_t)fr_end1, (uint32_t)frame_time1 / 1000);

	ESP_LOGI(TAG, "读取完成，读取速度：%u KB/S", (100 * 1024 * 100) / ((uint32_t)frame_time1 / 1000));
	// Strip newline
	// char *pos = strchr(line, '\n');
	// if (pos)
	// {
	// 	*pos = '\0';
	// }
	// ESP_LOGI(TAG, "Read from file: '%s'", line);

	fclose(f);

	// All done, unmount partition and disable SPI peripheral
	esp_vfs_fat_sdcard_unmount(mount_point, card);
	ESP_LOGI(TAG, "Card unmounted");

	// deinitialize the bus after all devices are removed
	spi_bus_free(host.slot);
}
