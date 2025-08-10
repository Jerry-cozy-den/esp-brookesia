/*
 * SPDX发牌文本：2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX许可证标识符：CC0-1.0
 * 
 * USB大容量存储设备(MSC)管理头文件
 * 
 * 本文件定义了USB大容量存储设备的挂载接口，用于：
 * - 将ESP32设备模拟为USB存储设备
 * - 通过USB接口向电脑暴露内部存储（如SD卡、SPI Flash等）
 * - 支持开发者模式下的文件传输和配置更新
 * 
 * USB MSC功能说明：
 * - 当设备连接到电脑时，可以像U盘一样被识别和访问
 * - 主要用于开发和调试阶段的文件管理
 * - 可以方便地更新音频文件、配置文件等资源
 */
#pragma once

#include "esp_err.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief 挂载USB大容量存储设备
 * 
 * 该函数初始化并启动USB MSC功能，使ESP32设备能够：
 * - 作为USB存储设备被电脑识别
 * - 允许电脑访问设备的存储分区（通常是SD卡）
 * - 支持文件的读写操作
 * 
 * 功能特点：
 * - 实现标准的USB Mass Storage Class协议
 * - 支持热插拔检测
 * - 与现有文件系统兼容
 * 
 * 使用场景：
 * - 开发者模式下的文件传输
 * - 音频资源更新
 * - 配置文件修改
 * 
 * @return esp_err_t 
 *         - ESP_OK: USB MSC挂载成功
 *         - ESP_FAIL: 挂载失败（硬件不支持或初始化错误）
 */
esp_err_t usb_msc_mount(void);

#if defined(__cplusplus)
}
#endif
