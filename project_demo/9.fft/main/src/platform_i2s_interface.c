/*
** Copyright (c) 2023 The tinyuac project. All rights reserved.
** Created by crisqifawei 2023
*/

#include "platform_i2s_interface.h"

esp_err_t platform_i2s_init(void)
{
	i2s_config_t i2s_config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate = AUDIO_LRCLK,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format = I2S_COMM_FORMAT_STAND_I2S,
		.intr_alloc_flags = 0,
		.dma_desc_num = 2,
		.dma_frame_num = 1024,
		.use_apll = 0,
	};
	i2s_pin_config_t pin_config = {
		.bck_io_num = IIS_SCLK,	  // IIS_SCLK
		.ws_io_num = IIS_LCLK,	  // IIS_LCLK
		.data_out_num = IIS_DOUT, // IIS_DOUT
		.data_in_num = IIS_DSIN	  // IIS_DSIN
	};
	i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
	i2s_set_pin(I2S_NUM_0, &pin_config);
	i2s_zero_dma_buffer(I2S_NUM_0);

	return ESP_OK;
}