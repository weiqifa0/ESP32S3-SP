/*
** Copyright (c) 2023 The tinyuac project. All rights reserved.
** Created by crisqifawei 2023
*/
#pragma once

#include "queue.h"
#include "platform_i2s_interface.h"

#define AUDIO_QUEUE_LENGTH 100
#define AUDIO_CAPTURE_TIME_MS 1
#define AUDIO_PACKET_SIZE (AUDIO_CAPTURE_TIME_MS * AUDIO_LRCLK * AUDIO_CH / 1000)

typedef struct {
	uint16_t data[AUDIO_PACKET_SIZE];
	size_t size;
} AudioPacket;
extern QueueHandle_t g_audioQueue;
