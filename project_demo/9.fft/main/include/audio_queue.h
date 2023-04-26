/*
** Copyright (c) 2023 The Roomix project. All rights reserved.
** Created by crisqifawei 2023
*/
#pragma once

#include "queue.h"

#define AUDIO_QUEUE_LENGTH 100
#define AUDIO_CH 2
#define AUDIO_CAPTURE_TIME_MS 20
#define AUDIO_LRCLK 16000
#define AUDIO_BITS (16/8)
#define AUDIO_PACKET_SIZE (AUDIO_CAPTURE_TIME_MS * AUDIO_LRCLK * AUDIO_BITS * AUDIO_CH / 1000)
typedef struct {
	uint8_t data[AUDIO_PACKET_SIZE];
	uint32_t size;
} AudioPacket;
extern QueueHandle_t g_audioQueue;
