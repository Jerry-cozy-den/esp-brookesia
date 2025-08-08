/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_camera.h"
#include "esp_err.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32-Camera配置和初始化
 * 
 * 为Speaker项目配置摄像头功能，支持拍照、录像等功能
 */

// 摄像头引脚配置 (适用于ESP32-S3-LCD-EV-Board)
#define CAM_PIN_PWDN    -1  // Power down引脚 (不使用)
#define CAM_PIN_RESET   -1  // Reset引脚 (不使用)
#define CAM_PIN_XCLK    GPIO_NUM_15  // 时钟引脚
#define CAM_PIN_SIOD    GPIO_NUM_4   // SDA引脚
#define CAM_PIN_SIOC    GPIO_NUM_5   // SCL引脚

#define CAM_PIN_D7      GPIO_NUM_16  // 数据引脚
#define CAM_PIN_D6      GPIO_NUM_17
#define CAM_PIN_D5      GPIO_NUM_18
#define CAM_PIN_D4      GPIO_NUM_12
#define CAM_PIN_D3      GPIO_NUM_10
#define CAM_PIN_D2      GPIO_NUM_8
#define CAM_PIN_D1      GPIO_NUM_9
#define CAM_PIN_D0      GPIO_NUM_11

#define CAM_PIN_VSYNC   GPIO_NUM_6   // 垂直同步引脚
#define CAM_PIN_HREF    GPIO_NUM_7   // 水平参考引脚
#define CAM_PIN_PCLK    GPIO_NUM_13  // 像素时钟引脚

/**
 * @brief 摄像头配置结构体
 */
typedef struct {
    pixformat_t pixel_format;     // 像素格式 (JPEG/RGB565/YUV422等)
    framesize_t frame_size;       // 图像尺寸
    uint8_t jpeg_quality;         // JPEG质量 (0-63, 数值越小质量越高)
    uint8_t fb_count;            // 帧缓冲区数量
    bool enable_psram;           // 是否使用PSRAM
} speaker_camera_config_t;

/**
 * @brief 初始化摄像头
 * 
 * @param config 摄像头配置参数，如果为NULL则使用默认配置
 * @return
 *    - ESP_OK: 成功
 *    - ESP_ERR_INVALID_ARG: 无效参数
 *    - ESP_ERR_NOT_FOUND: 未找到摄像头
 *    - ESP_FAIL: 初始化失败
 */
esp_err_t speaker_camera_init(const speaker_camera_config_t *config);

/**
 * @brief 反初始化摄像头
 * 
 * @return
 *    - ESP_OK: 成功
 *    - ESP_FAIL: 失败
 */
esp_err_t speaker_camera_deinit(void);

/**
 * @brief 拍照获取图像
 * 
 * @param fb 输出的帧缓冲区指针
 * @return
 *    - ESP_OK: 成功
 *    - ESP_ERR_INVALID_STATE: 摄像头未初始化
 *    - ESP_FAIL: 拍照失败
 */
esp_err_t speaker_camera_capture(camera_fb_t **fb);

/**
 * @brief 释放帧缓冲区
 * 
 * @param fb 要释放的帧缓冲区
 * @return
 *    - ESP_OK: 成功
 *    - ESP_ERR_INVALID_ARG: 无效参数
 */
esp_err_t speaker_camera_fb_return(camera_fb_t *fb);

/**
 * @brief 设置图像质量
 * 
 * @param quality JPEG质量 (0-63, 数值越小质量越高)
 * @return
 *    - ESP_OK: 成功
 *    - ESP_ERR_INVALID_ARG: 无效参数
 *    - ESP_ERR_INVALID_STATE: 摄像头未初始化
 */
esp_err_t speaker_camera_set_quality(uint8_t quality);

/**
 * @brief 设置图像尺寸
 * 
 * @param framesize 图像尺寸
 * @return
 *    - ESP_OK: 成功
 *    - ESP_ERR_INVALID_ARG: 无效参数
 *    - ESP_ERR_INVALID_STATE: 摄像头未初始化
 */
esp_err_t speaker_camera_set_framesize(framesize_t framesize);

/**
 * @brief 设置像素格式
 * 
 * @param format 像素格式
 * @return
 *    - ESP_OK: 成功
 *    - ESP_ERR_INVALID_ARG: 无效参数
 *    - ESP_ERR_INVALID_STATE: 摄像头未初始化
 */
esp_err_t speaker_camera_set_pixformat(pixformat_t format);

/**
 * @brief 获取默认摄像头配置
 * 
 * @return 默认配置结构体
 */
speaker_camera_config_t speaker_camera_get_default_config(void);

/**
 * @brief 检查摄像头是否已初始化
 * 
 * @return
 *    - true: 已初始化
 *    - false: 未初始化
 */
bool speaker_camera_is_initialized(void);

#ifdef __cplusplus
}
#endif
