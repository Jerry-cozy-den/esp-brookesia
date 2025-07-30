/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

/**
 * @file audio_sys.h
 * @brief ESP-Brookesia 智能音箱音频系统工具头文件
 * 
 * 该文件定义了音频系统相关的工具函数和宏定义，包括：
 * - 系统时间和定时器相关函数
 * - CPU使用率统计函数
 * - 通用工具宏定义
 * 
 * 这些工具函数主要用于音频系统的性能监控、时间管理和系统调试。
 */

#ifndef _AUDIO_SYS_H_
#define _AUDIO_SYS_H_

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 工具宏定义 */
#define ___STR___(x) #x                   /*!< 字符串化宏（内部使用） */
#define STR_AUDIO(x) ___STR___(x)         /*!< 将参数转换为字符串的宏 */

#define AUDIO_UNUSED(x) (void)(x)         /*!< 标记未使用变量的宏，避免编译器警告 */

/* GCC编译器相关宏定义 */
#if defined(__GNUC__) && (__GNUC__ >= 7)
#define FALL_THROUGH __attribute__ ((fallthrough))  /*!< GCC 7+ switch语句穿透标记 */
#else
#define FALL_THROUGH ((void)0)            /*!< 其他编译器的空操作宏 */
#endif /* __GNUC__ >= 7 */

/**
 * @brief 根据给定的毫秒数获取系统时钟周期数
 * 
 * 将毫秒时间转换为系统时钟周期数（ticks），用于FreeRTOS任务延时等操作。
 * 转换基于当前系统的时钟频率配置。
 *
 * @param[in] ms 毫秒数
 *
 * @return 对应的系统时钟周期数
 */
int audio_sys_get_tick_by_time_ms(int ms);

/**
 * @brief 获取系统当前时间（毫秒精度）
 * 
 * 返回系统启动以来的毫秒数，用于时间戳、性能测量等场景。
 * 提供毫秒级精度的时间测量。
 *
 * @return 系统时间（毫秒）
 */
int64_t audio_sys_get_time_ms(void);

/**
 * @brief 获取任务CPU使用率实时统计信息
 * 
 * 该函数用于测量和打印指定时间段内各个任务的CPU使用率统计信息。
 * 实现原理是通过两次调用uxTaskGetSystemState()函数（中间间隔一定时间），
 * 然后计算任务运行时间的差值来统计CPU使用率。
 * 
 * 功能特点：
 * - 提供系统中所有任务的CPU使用率统计
 * - 用于性能调优和系统监控
 * - 支持双核模式下的CPU使用率分析
 * 
 * @note 如果在测量期间有任务被添加或删除，这些任务的统计信息将不会被打印
 * @note 该函数应该从高优先级任务中调用，以最小化延时带来的测量误差
 * @note 在双核模式下运行时，每个核心对应50%的运行时间
 *
 * @return
 *      - ESP_OK: 统计成功
 *      - ESP_FAIL: 统计失败
 */
esp_err_t audio_sys_get_real_time_stats(void);

#ifdef __cplusplus
}
#endif

#endif
