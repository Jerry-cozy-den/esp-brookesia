/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "speaker_camera.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "camera_example";

/**
 * @brief 摄像头拍照示例
 * 
 * 演示如何初始化摄像头、拍照并保存到文件系统
 */
void camera_example_capture_photo(void)
{
    esp_err_t ret;
    camera_fb_t *fb = NULL;
    
    ESP_LOGI(TAG, "Starting camera capture example");
    
    // 1. 初始化摄像头 (使用默认配置)
    ret = speaker_camera_init(NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize camera: %s", esp_err_to_name(ret));
        return;
    }
    
    // 等待摄像头稳定
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 2. 拍照
    ESP_LOGI(TAG, "Capturing photo...");
    ret = speaker_camera_capture(&fb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to capture photo: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    
    // 3. 保存照片到文件 (假设SPIFFS已挂载到/spiffs)
    FILE *file = fopen("/spiffs/photo.jpg", "wb");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        goto cleanup_fb;
    }
    
    size_t written = fwrite(fb->buf, 1, fb->len, file);
    fclose(file);
    
    if (written == fb->len) {
        ESP_LOGI(TAG, "Photo saved successfully: %zu bytes", fb->len);
    } else {
        ESP_LOGE(TAG, "Failed to write complete photo data");
    }
    
cleanup_fb:
    // 4. 释放帧缓冲区
    speaker_camera_fb_return(fb);
    
cleanup:
    // 5. 反初始化摄像头
    speaker_camera_deinit();
    ESP_LOGI(TAG, "Camera example completed");
}

/**
 * @brief 摄像头设置示例
 * 
 * 演示如何调整摄像头参数
 */
void camera_example_settings(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Starting camera settings example");
    
    // 自定义配置
    speaker_camera_config_t config = {
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_VGA,    // 640x480
        .jpeg_quality = 8,              // 高质量
        .fb_count = 2,                  // 双缓冲
        .enable_psram = true,
    };
    
    // 初始化摄像头
    ret = speaker_camera_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize camera: %s", esp_err_to_name(ret));
        return;
    }
    
    // 动态调整设置
    ESP_LOGI(TAG, "Adjusting camera settings...");
    
    // 设置高质量
    speaker_camera_set_quality(5);
    
    // 设置帧尺寸
    speaker_camera_set_framesize(FRAMESIZE_CIF);  // 352x288
    
    // 连续拍摄几张照片测试
    for (int i = 0; i < 3; i++) {
        camera_fb_t *fb = NULL;
        
        ESP_LOGI(TAG, "Capturing photo %d...", i + 1);
        ret = speaker_camera_capture(&fb);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Photo %d: %zu bytes", i + 1, fb->len);
            speaker_camera_fb_return(fb);
        } else {
            ESP_LOGE(TAG, "Failed to capture photo %d", i + 1);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // 清理
    speaker_camera_deinit();
    ESP_LOGI(TAG, "Camera settings example completed");
}

/**
 * @brief 摄像头任务示例
 * 
 * 创建一个任务来连续捕获图像
 */
void camera_task(void *pvParameters)
{
    esp_err_t ret;
    camera_fb_t *fb = NULL;
    int photo_count = 0;
    
    ESP_LOGI(TAG, "Starting camera task");
    
    // 初始化摄像头
    ret = speaker_camera_init(NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize camera in task: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);
        return;
    }
    
    while (photo_count < 10) {  // 拍摄10张照片后退出
        // 拍照
        ret = speaker_camera_capture(&fb);
        if (ret == ESP_OK) {
            photo_count++;
            ESP_LOGI(TAG, "Task photo %d: %zu bytes", photo_count, fb->len);
            
            // 这里可以处理图像数据，比如：
            // - 图像识别
            // - 网络传输
            // - 显示到屏幕
            
            speaker_camera_fb_return(fb);
        } else {
            ESP_LOGE(TAG, "Task failed to capture photo: %s", esp_err_to_name(ret));
        }
        
        // 等待2秒
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    // 清理
    speaker_camera_deinit();
    ESP_LOGI(TAG, "Camera task completed");
    vTaskDelete(NULL);
}

/**
 * @brief 创建摄像头任务
 */
void camera_example_create_task(void)
{
    xTaskCreate(camera_task, "camera_task", 4096, NULL, 5, NULL);
}
