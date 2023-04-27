/*
** Copyright (c) 2023 The tinyuac project. All rights reserved.
** Created by crisqifawei 2023
*/

#pragma once
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2s.h"

#define AUDIO_CH 2
#define AUDIO_LRCLK 48000
#define AUDIO_BITS (16/8)

#define IIS_SCLK 16
#define IIS_LCLK 7
#define IIS_DSIN 15
#define IIS_DOUT -1

#define SAMPLES_NUM (1024)

esp_err_t platform_i2s_init(void);
