/*
 * SPDX发牌文本：2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX许可证标识符：CC0-1.0
 * 
 * Coze智能代理默认配置头文件
 * 
 * 本文件定义了Coze AI平台的默认配置参数映射，主要功能：
 * - 将menuconfig中的配置选项映射为预编译宏定义
 * - 提供Coze平台集成的默认参数模板
 * - 支持多个AI机器人的条件编译配置
 * - 简化开发过程中的参数管理
 * 
 * 配置层次结构：
 * 1. menuconfig配置选项 (CONFIG_EXAMPLE_*)
 * 2. 本文件的宏定义映射 (COZE_AGENT_*)
 * 3. 运行时配置加载 (coze_agent_config.c)
 * 
 * 使用说明：
 * - 通过menuconfig启用/禁用默认配置
 * - 支持最多2个机器人的独立配置
 * - 每个机器人可单独启用/禁用
 */
#pragma once

#include "sdkconfig.h"

// 启用默认配置的主开关（从menuconfig读取）
#define COZE_AGENT_ENABLE_DEFAULT_CONFIG CONFIG_EXAMPLE_COZE_AGENT_ENABLE_DEFAULT_CONFIG

#if COZE_AGENT_ENABLE_DEFAULT_CONFIG

// Coze平台基础配置参数
#define COZE_AGENT_APP_ID                  CONFIG_EXAMPLE_COZE_AGENT_APP_ID        // 应用程序ID
#define COZE_AGENT_DEVICE_PUBLIC_KEY       CONFIG_EXAMPLE_COZE_AGENT_PUBLIC_KEY    // 设备公钥
#define COZE_AGENT_CUSTOM_CONSUMER         CONFIG_EXAMPLE_COZE_AGENT_CUSTOM_CONSUMER  // 自定义消费者标识

// 机器人启用开关
#define COZE_AGENT_BOT1_ENABLE             CONFIG_EXAMPLE_COZE_AGENT_BOT1_ENABLE   // 机器人1启用标志
#define COZE_AGENT_BOT2_ENABLE             CONFIG_EXAMPLE_COZE_AGENT_BOT2_ENABLE   // 机器人2启用标志

// 机器人1配置参数（条件编译）
#if COZE_AGENT_BOT1_ENABLE
#   define COZE_AGENT_BOT1_NAME            CONFIG_EXAMPLE_COZE_AGENT_BOT1_NAME        // 机器人1显示名称
#   define COZE_AGENT_BOT1_ID              CONFIG_EXAMPLE_COZE_AGENT_BOT1_ID          // 机器人1唯一标识符
#   define COZE_AGENT_BOT1_VOICE_ID        CONFIG_EXAMPLE_COZE_AGENT_BOT1_VOICE_ID    // 机器人1语音合成ID
#   define COZE_AGENT_BOT1_DESCRIPTION     CONFIG_EXAMPLE_COZE_AGENT_BOT1_DESCRIPTION // 机器人1功能描述
#endif // COZE_AGENT_BOT1_ENABLE

// 机器人2配置参数（条件编译）
#if COZE_AGENT_BOT2_ENABLE
#   define COZE_AGENT_BOT2_NAME            CONFIG_EXAMPLE_COZE_AGENT_BOT2_NAME        // 机器人2显示名称
#   define COZE_AGENT_BOT2_ID              CONFIG_EXAMPLE_COZE_AGENT_BOT2_ID          // 机器人2唯一标识符
#   define COZE_AGENT_BOT2_VOICE_ID        CONFIG_EXAMPLE_COZE_AGENT_BOT2_VOICE_ID    // 机器人2语音合成ID
#   define COZE_AGENT_BOT2_DESCRIPTION     CONFIG_EXAMPLE_COZE_AGENT_BOT2_DESCRIPTION // 机器人2功能描述
#endif // COZE_AGENT_BOT2_ENABLE

#endif // COZE_AGENT_ENABLE_DEFAULT_CONFIG
