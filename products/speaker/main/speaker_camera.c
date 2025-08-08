/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "speaker_camera.h"
#include "esp_system.h"
#include "esp_heap_caps.h"

static const char *TAG = "speaker_camera";

// 摄像头状态
static bool camera_initialized = false;

esp_err_t speaker_camera_init(const speaker_camera_config_t *config)
{
    esp_err_t ret = ESP_OK;
    
    if (camera_initialized) {
        ESP_LOGW(TAG, "Camera already initialized");
        return ESP_OK;
    }

    // 使用默认配置或用户配置
    speaker_camera_config_t cam_config;
    if (config == NULL) {
        cam_config = speaker_camera_get_default_config();
    } else {
        cam_config = *config;
    }

    // ESP32-Camera配置
    camera_config_t esp_cam_config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sscb_sda = CAM_PIN_SIOD,
        .pin_sscb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        // XCLK 20MHz或10MHz用于OV2640，对于OV3660建议使用camera_xclk_freq_20M
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = cam_config.pixel_format,
        .frame_size = cam_config.frame_size,

        .jpeg_quality = cam_config.jpeg_quality,
        .fb_count = cam_config.fb_count,
        .fb_location = cam_config.enable_psram ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    };

    // 初始化摄像头
    ret = esp_camera_init(&esp_cam_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", ret);
        return ret;
    }

    camera_initialized = true;
    ESP_LOGI(TAG, "Camera initialized successfully");
    ESP_LOGI(TAG, "Frame size: %dx%d, Format: %s, Quality: %d", 
             esp_cam_config.frame_size == FRAMESIZE_QVGA ? 320 : 
             esp_cam_config.frame_size == FRAMESIZE_CIF ? 352 :
             esp_cam_config.frame_size == FRAMESIZE_VGA ? 640 : 0,
             esp_cam_config.frame_size == FRAMESIZE_QVGA ? 240 :
             esp_cam_config.frame_size == FRAMESIZE_CIF ? 288 :
             esp_cam_config.frame_size == FRAMESIZE_VGA ? 480 : 0,
             esp_cam_config.pixel_format == PIXFORMAT_JPEG ? "JPEG" :
             esp_cam_config.pixel_format == PIXFORMAT_RGB565 ? "RGB565" : "Unknown",
             esp_cam_config.jpeg_quality);

    return ESP_OK;
}

esp_err_t speaker_camera_deinit(void)
{
    if (!camera_initialized) {
        ESP_LOGW(TAG, "Camera not initialized");
        return ESP_OK;
    }

    esp_err_t ret = esp_camera_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera deinit failed with error 0x%x", ret);
        return ret;
    }

    camera_initialized = false;
    ESP_LOGI(TAG, "Camera deinitialized successfully");
    return ESP_OK;
}

esp_err_t speaker_camera_capture(camera_fb_t **fb)
{
    if (!camera_initialized) {
        ESP_LOGE(TAG, "Camera not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (fb == NULL) {
        ESP_LOGE(TAG, "Invalid frame buffer pointer");
        return ESP_ERR_INVALID_ARG;
    }

    *fb = esp_camera_fb_get();
    if (*fb == NULL) {
        ESP_LOGE(TAG, "Camera capture failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Camera captured %zu bytes", (*fb)->len);
    return ESP_OK;
}

esp_err_t speaker_camera_fb_return(camera_fb_t *fb)
{
    if (fb == NULL) {
        ESP_LOGE(TAG, "Invalid frame buffer");
        return ESP_ERR_INVALID_ARG;
    }

    esp_camera_fb_return(fb);
    return ESP_OK;
}

esp_err_t speaker_camera_set_quality(uint8_t quality)
{
    if (!camera_initialized) {
        ESP_LOGE(TAG, "Camera not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (quality > 63) {
        ESP_LOGE(TAG, "Invalid quality value: %d (must be 0-63)", quality);
        return ESP_ERR_INVALID_ARG;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return ESP_FAIL;
    }

    s->set_quality(s, quality);
    ESP_LOGI(TAG, "Camera quality set to %d", quality);
    return ESP_OK;
}

esp_err_t speaker_camera_set_framesize(framesize_t framesize)
{
    if (!camera_initialized) {
        ESP_LOGE(TAG, "Camera not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return ESP_FAIL;
    }

    s->set_framesize(s, framesize);
    ESP_LOGI(TAG, "Camera framesize set to %d", framesize);
    return ESP_OK;
}

esp_err_t speaker_camera_set_pixformat(pixformat_t format)
{
    if (!camera_initialized) {
        ESP_LOGE(TAG, "Camera not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return ESP_FAIL;
    }

    s->set_pixformat(s, format);
    ESP_LOGI(TAG, "Camera pixel format set to %d", format);
    return ESP_OK;
}

speaker_camera_config_t speaker_camera_get_default_config(void)
{
    speaker_camera_config_t config = {
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_QVGA,  // 320x240
        .jpeg_quality = 12,             // 中等质量
        .fb_count = 1,                  // 单缓冲
        .enable_psram = true,           // 使用PSRAM
    };
    return config;
}

bool speaker_camera_is_initialized(void)
{
    return camera_initialized;
}
