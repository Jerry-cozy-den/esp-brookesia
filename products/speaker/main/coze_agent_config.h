/*
 * SPDX发牌文本：2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX许可证标识符：CC0-1.0
 * 
 * Coze智能代理配置管理头文件
 * 
 * 本文件定义了Coze AI平台智能代理的配置结构体和管理接口，用于：
 * - 管理Coze平台的认证信息（公钥、私钥、应用ID）
 * - 配置多个AI机器人的参数（机器人ID、语音ID、名称、描述）
 * - 提供配置文件读取和资源释放的接口
 * 
 * Coze平台集成说明：
 * - 支持多达MAX_BOT_NUM个机器人同时配置
 * - 每个机器人可配置独立的语音合成参数
 * - 配置信息从NVS存储或默认配置文件加载
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// 支持的最大机器人数量
#define MAX_BOT_NUM 2

/**
 * @brief Coze智能代理配置结构体
 * 
 * 该结构体包含了连接和使用Coze AI平台所需的全部配置信息：
 * - 平台认证信息：用于与Coze服务器建立安全连接
 * - 机器人配置数组：支持多个AI机器人的个性化设置
 * 
 * 每个机器人可以有不同的：
 * - 对话能力和知识库
 * - 语音合成特性
 * - 用户界面显示信息
 */
typedef struct {
    char *public_key;           // Coze平台公钥，用于API认证
    char *private_key;          // Coze平台私钥，用于安全通信
    char *appid;                // 应用程序ID，标识当前应用
    char *custom_consumer;      // 自定义消费者标识符
    int bot_num;                // 实际配置的机器人数量
    
    /**
     * @brief 单个机器人配置信息
     */
    struct bot_config {
        char *bot_id;           // 机器人唯一标识符
        char *voice_id;         // 语音合成ID，决定机器人说话声音
        char *bot_name;         // 机器人显示名称
        char *bot_description;  // 机器人功能描述
    } bot[MAX_BOT_NUM];         // 机器人配置数组
} coze_agent_config_t;

/**
 * @brief 读取Coze智能代理配置信息
 * 
 * 该函数从存储中加载Coze AI平台的配置信息，包括：
 * - 首先尝试从NVS（非易失性存储）读取用户自定义配置
 * - 如果NVS中没有配置，则使用默认配置文件
 * - 解析JSON格式的配置数据并填充配置结构体
 * 
 * @param config 指向配置结构体的指针，用于存储读取的配置信息
 * @return esp_err_t 
 *         - ESP_OK: 配置读取成功
 *         - ESP_FAIL: 配置读取失败（存储错误或格式错误）
 */
esp_err_t coze_agent_config_read(coze_agent_config_t *config);

/**
 * @brief 释放Coze智能代理配置资源
 * 
 * 该函数用于释放coze_agent_config_read()分配的所有内存资源，
 * 包括字符串缓冲区和动态分配的数据结构。
 * 
 * 注意：调用此函数后，config结构体中的所有指针将变为无效，
 * 不应再访问这些指针指向的内容。
 * 
 * @param config 指向需要释放资源的配置结构体的指针
 * @return esp_err_t 
 *         - ESP_OK: 资源释放成功
 *         - ESP_FAIL: 资源释放失败
 */
esp_err_t coze_agent_config_release(coze_agent_config_t *config);

#ifdef __cplusplus
}
#endif
