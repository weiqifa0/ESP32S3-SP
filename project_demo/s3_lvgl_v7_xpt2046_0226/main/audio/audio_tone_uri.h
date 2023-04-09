#ifndef __AUDIO_TONEURI_H__
#define __AUDIO_TONEURI_H__

extern const char* tone_uri[];

typedef enum {
    TONE_TYPE_AIR_CLOSE,
    TONE_TYPE_AIR_SET_OK,
    TONE_TYPE_BLE_CLOSE,
    TONE_TYPE_BLE_CONN_SUCCE,
    TONE_TYPE_BLE_DISCONN,
    TONE_TYPE_BLE_OPEN,
    TONE_TYPE_NO,
    TONE_TYPE_SET_TIMER,
    TONE_TYPE_WHATAREYOUTAKING,
    TONE_TYPE_WIFI_CON,
    TONE_TYPE_WIFI_DISC,
    TONE_TYPE_WOZAI,
    TONE_TYPE_MAX,
} tone_type_t;

#endif 
