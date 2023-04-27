/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "esp_private/usb_phy.h"
#include "fft_convert.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "usb_headset.h"
#include "driver/i2s_std.h"
#include "audio_queue.h"
#include "platform_i2s_interface.h"
//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
//--------------------------------------------------------------------+
// List of supported sample rates
#if defined(__RX__)
const uint32_t sample_rates[] = {44100, 48000};
#else
const uint32_t sample_rates[] = {44100, 48000, 88200, 96000};
#endif

uint32_t current_sample_rate  = 48000;

// extern i2s_chan_handle_t i2s_tx_chan;
// extern i2s_chan_handle_t i2s_rx_chan;

#define N_SAMPLE_RATES  TU_ARRAY_SIZE(sample_rates)

#define AUDIO_LENGTH 640

/* Blink pattern
 * - 25 ms   : streaming data
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
	BLINK_STREAMING = 25,
	BLINK_NOT_MOUNTED = 250,
	BLINK_MOUNTED = 1000,
	BLINK_SUSPENDED = 2500,
};

enum {
	VOLUME_CTRL_0_DB = 0,
	VOLUME_CTRL_10_DB = 2560,
	VOLUME_CTRL_20_DB = 5120,
	VOLUME_CTRL_30_DB = 7680,
	VOLUME_CTRL_40_DB = 10240,
	VOLUME_CTRL_50_DB = 12800,
	VOLUME_CTRL_60_DB = 15360,
	VOLUME_CTRL_70_DB = 17920,
	VOLUME_CTRL_80_DB = 20480,
	VOLUME_CTRL_90_DB = 23040,
	VOLUME_CTRL_100_DB = 25600,
	VOLUME_CTRL_SILENCE = 0x8000,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

// Audio controls
// Current states
int8_t mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];       // +1 for master channel 0
int16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];    // +1 for master channel 0

// Buffer for microphone data
int16_t mic_buf[CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ / 2];
// Buffer for speaker data
int16_t spk_buf[CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ / 4];
// Speaker data size received in the last frame
int spk_data_size;
// Resolution per format
const uint8_t resolutions_per_format[CFG_TUD_AUDIO_FUNC_1_N_FORMATS] = {CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX,
                                                                        CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_RX
                                                                       };
// Current resolution, update on format change
uint8_t current_resolution;
AudioPacket g_audio_packet;

// void led_blinking_task(void);


static usb_phy_handle_t phy_hdl;
static void usb_phy_init(void)
{
	// Configure USB PHY
	usb_phy_config_t phy_conf = {
			.controller = USB_PHY_CTRL_OTG,
			.otg_mode = USB_OTG_MODE_DEVICE,
	};
	phy_conf.target = USB_PHY_TARGET_INT;
	usb_new_phy(&phy_conf, &phy_hdl);
}

static void usb_task(void *pvParam)
{
	(void) pvParam;

	do {
			// TinyUSB device task
			tud_task();
	} while (true);

	vTaskDelete(NULL);
}

static void send_audio_task(void *pvParam)
{
	(void) pvParam;
	do {
		if (g_audioQueue) {
			xQueueSend(g_audioQueue, &g_audio_packet, portTICK_PERIOD_MS);
		}
		vTaskDelay(pdMS_TO_TICKS(600));
	} while (true);

	vTaskDelete(NULL);
}

/*------------- MAIN -------------*/
esp_err_t usb_headset_init(void)
{
	usb_phy_init();
	// init device stack on configured roothub port
	tud_init(BOARD_TUD_RHPORT);
	platform_i2s_init();
	TU_LOG1("Headset running\r\n");
	BaseType_t ret_val = xTaskCreatePinnedToCore(usb_task, "usb_task", 8 * 1024, NULL, 9, NULL, 1);
	ret_val = xTaskCreatePinnedToCore(send_audio_task, "send_audio_task", 8 * 1024, NULL, 6, NULL, 0);
	return (pdPASS == ret_val) ? ESP_OK : ESP_FAIL;
}

// Invoked when device is mounted
void tud_mount_cb(void)
{
	blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
	blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
	(void)remote_wakeup_en;
	blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
	blink_interval_ms = BLINK_MOUNTED;
}

// Helper for clock get requests
static bool tud_audio_clock_get_request(uint8_t rhport, audio_control_request_t const *request)
{
	TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);

	if (request->bControlSelector == AUDIO_CS_CTRL_SAM_FREQ) {
			if (request->bRequest == AUDIO_CS_REQ_CUR) {
					TU_LOG1("Clock get current freq %lu\r\n", current_sample_rate);

					audio_control_cur_4_t curf = { (int32_t) tu_htole32(current_sample_rate) };
					return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &curf, sizeof(curf));
			} else if (request->bRequest == AUDIO_CS_REQ_RANGE) {
					audio_control_range_4_n_t(N_SAMPLE_RATES) rangef = {
							.wNumSubRanges = tu_htole16(N_SAMPLE_RATES)
					};
					TU_LOG1("Clock get %d freq ranges\r\n", N_SAMPLE_RATES);
					for (uint8_t i = 0; i < N_SAMPLE_RATES; i++) {
							rangef.subrange[i].bMin = (int32_t) sample_rates[i];
							rangef.subrange[i].bMax = (int32_t) sample_rates[i];
							rangef.subrange[i].bRes = 0;
							TU_LOG1("Range %d (%d, %d, %d)\r\n", i, (int)rangef.subrange[i].bMin, (int)rangef.subrange[i].bMax, (int)rangef.subrange[i].bRes);
					}

					return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &rangef, sizeof(rangef));
			}
	} else if (request->bControlSelector == AUDIO_CS_CTRL_CLK_VALID &&
							request->bRequest == AUDIO_CS_REQ_CUR) {
			audio_control_cur_1_t cur_valid = { .bCur = 1 };
			TU_LOG1("Clock get is valid %u\r\n", cur_valid.bCur);
			return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &cur_valid, sizeof(cur_valid));
	}
	TU_LOG1("Clock get request not supported, entity = %u, selector = %u, request = %u\r\n",
					request->bEntityID, request->bControlSelector, request->bRequest);
	return false;
}

// Helper for clock set requests
static bool tud_audio_clock_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf)
{
	(void)rhport;

	TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);
	TU_VERIFY(request->bRequest == AUDIO_CS_REQ_CUR);

	if (request->bControlSelector == AUDIO_CS_CTRL_SAM_FREQ) {
			TU_VERIFY(request->wLength == sizeof(audio_control_cur_4_t));

			current_sample_rate = (uint32_t) ((audio_control_cur_4_t const *)buf)->bCur;

			TU_LOG1("Clock set current freq: %ld\r\n", current_sample_rate);

			return true;
	} else {
			TU_LOG1("Clock set request not supported, entity = %u, selector = %u, request = %u\r\n",
							request->bEntityID, request->bControlSelector, request->bRequest);
			return false;
	}
}

// Helper for feature unit get requests
static bool tud_audio_feature_unit_get_request(uint8_t rhport, audio_control_request_t const *request)
{
	TU_ASSERT(request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT);

	if (request->bControlSelector == AUDIO_FU_CTRL_MUTE && request->bRequest == AUDIO_CS_REQ_CUR) {
		audio_control_cur_1_t mute1 = { .bCur = mute[request->bChannelNumber] };
		TU_LOG1("Get channel %u mute %d\r\n", request->bChannelNumber, mute1.bCur);
		return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &mute1, sizeof(mute1));
	} else if (UAC2_ENTITY_SPK_FEATURE_UNIT && request->bControlSelector == AUDIO_FU_CTRL_VOLUME) {
		if (request->bRequest == AUDIO_CS_REQ_RANGE) {
				audio_control_range_2_n_t(1) range_vol = {
						.wNumSubRanges = tu_htole16(1),
						.subrange[0] = { .bMin = tu_htole16(-VOLUME_CTRL_50_DB), tu_htole16(VOLUME_CTRL_0_DB), tu_htole16(256) }
				};
				TU_LOG1("Get channel %u volume range (%d, %d, %u) dB\r\n", request->bChannelNumber,
								range_vol.subrange[0].bMin / 256, range_vol.subrange[0].bMax / 256, range_vol.subrange[0].bRes / 256);
				return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &range_vol, sizeof(range_vol));
		} else if (request->bRequest == AUDIO_CS_REQ_CUR) {
				audio_control_cur_2_t cur_vol = { .bCur = tu_htole16(volume[request->bChannelNumber]) };
				TU_LOG1("Get channel %u volume %d dB\r\n", request->bChannelNumber, cur_vol.bCur / 256);
				return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &cur_vol, sizeof(cur_vol));
		}
	}
	TU_LOG1("Feature unit get request not supported, entity = %u, selector = %u, request = %u\r\n",
					request->bEntityID, request->bControlSelector, request->bRequest);

	return false;
}

// Helper for feature unit set requests
static bool tud_audio_feature_unit_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf)
{
	(void)rhport;

	TU_ASSERT(request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT);
	TU_VERIFY(request->bRequest == AUDIO_CS_REQ_CUR);

	if (request->bControlSelector == AUDIO_FU_CTRL_MUTE) {
		TU_VERIFY(request->wLength == sizeof(audio_control_cur_1_t));

		mute[request->bChannelNumber] = ((audio_control_cur_1_t const *)buf)->bCur;

		TU_LOG1("Set channel %d Mute: %d\r\n", request->bChannelNumber, mute[request->bChannelNumber]);

		return true;
	} else if (request->bControlSelector == AUDIO_FU_CTRL_VOLUME) {
		TU_VERIFY(request->wLength == sizeof(audio_control_cur_2_t));

		volume[request->bChannelNumber] = ((audio_control_cur_2_t const *)buf)->bCur;

		TU_LOG1("Set channel %d volume: %d dB\r\n", request->bChannelNumber, volume[request->bChannelNumber] / 256);

		return true;
	} else {
		TU_LOG1("Feature unit set request not supported, entity = %u, selector = %u, request = %u\r\n",
						request->bEntityID, request->bControlSelector, request->bRequest);
		return false;
	}
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
	audio_control_request_t const *request = (audio_control_request_t const *)p_request;

	if (request->bEntityID == UAC2_ENTITY_CLOCK) {
			return tud_audio_clock_get_request(rhport, request);
	}
	if (request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT) {
			return tud_audio_feature_unit_get_request(rhport, request);
	} else {
			TU_LOG1("Get request not handled, entity = %d, selector = %d, request = %d\r\n",
							request->bEntityID, request->bControlSelector, request->bRequest);
	}
	return false;
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *buf)
{
	audio_control_request_t const *request = (audio_control_request_t const *)p_request;

	if (request->bEntityID == UAC2_ENTITY_SPK_FEATURE_UNIT) {
			return tud_audio_feature_unit_set_request(rhport, request, buf);
	}
	if (request->bEntityID == UAC2_ENTITY_CLOCK) {
			return tud_audio_clock_set_request(rhport, request, buf);
	}
	TU_LOG1("Set request not handled, entity = %d, selector = %d, request = %d\r\n",
					request->bEntityID, request->bControlSelector, request->bRequest);

	return false;
}

bool tud_audio_set_itf_close_EP_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
	(void)rhport;

	uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
	uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

	if (ITF_NUM_AUDIO_STREAMING_SPK == itf && alt == 0) {
			blink_interval_ms = BLINK_MOUNTED;
	}

	return true;
}

bool tud_audio_set_itf_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
	(void)rhport;
	uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
	uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

	TU_LOG2("Set interface %d alt %d\r\n", itf, alt);
	if (ITF_NUM_AUDIO_STREAMING_SPK == itf && alt != 0) {
			blink_interval_ms = BLINK_STREAMING;
	}

	// Clear buffer when streaming format is changed
	spk_data_size = 0;
	if (alt != 0) {
			current_resolution = resolutions_per_format[alt - 1];
	}

	return true;
}

bool tud_audio_rx_done_pre_read_cb(uint8_t rhport, uint16_t n_bytes_received, uint8_t func_id, uint8_t ep_out, uint8_t cur_alt_setting)
{
	(void)rhport;
	(void)func_id;
	(void)ep_out;
	(void)cur_alt_setting;

	spk_data_size = tud_audio_read(spk_buf, n_bytes_received);
	return true;
}

bool tud_audio_rx_done_post_read_cb(uint8_t rhport, uint16_t n_bytes_received, uint8_t func_id, uint8_t ep_out, uint8_t cur_alt_setting)
{
	(void)rhport;
	(void)func_id;
	(void)ep_out;
	(void)cur_alt_setting;
	esp_err_t ret = ESP_OK;

	// size_t bytes_written = 0;
	// esp_err_t ret = i2s_channel_write(i2s_tx_chan, &spk_buf, spk_data_size, &bytes_written, 0);

	// for (int i = 0; i < AUDIO_LENGTH ; i += 2) {
	//     //rb_write(spk_buf + i, 2);
	// }
	return ret == ESP_OK ? true : false;
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
	(void)rhport;
	(void)itf;
	(void)ep_in;
	(void)cur_alt_setting;

	tud_audio_write(mic_buf, g_audio_packet.size);
	// This callback could be used to fill microphone data separately
	return true;
}
extern int8_t i2s_read_rawdata[1024];
bool tud_audio_tx_done_post_load_cb(uint8_t rhport, uint16_t n_bytes_copied, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
	(void) rhport;
	(void) n_bytes_copied;
	(void) itf;
	(void) ep_in;
	(void) cur_alt_setting;
	esp_err_t ret = ESP_OK;
	i2s_read(I2S_NUM_0, &g_audio_packet.data, AUDIO_LENGTH, &g_audio_packet.size, portTICK_PERIOD_MS);

	for (int i = 0, j = 0; j < g_audio_packet.size/2; i++, j += 2) {
		mic_buf[i] = g_audio_packet.data[j];
	}

	// memcpy(&g_audio_packet.data, mic_buf, g_audio_packet.size);
	// if (g_audioQueue) {
	// 	xQueueSend(g_audioQueue, &g_audio_packet, portTICK_PERIOD_MS);
	// }
	return ret == ESP_OK ? true : false;
}
