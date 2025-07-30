/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

/**
 * @file main.cpp
 * @brief ESP-Brookesia 手机演示程序主文件
 * 
 * 该文件实现了一个基于 ESP-Brookesia 框架的手机界面演示程序，
 * 包含显示初始化、触摸设备配置、应用安装和内存监控等功能。
 */

// FreeRTOS 相关头文件
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP-IDF 核心功能头文件
#include "esp_check.h"
#include "esp_log.h"

// LVGL 图形库移植层头文件
#include "esp_lvgl_port.h"

// 板级支持包(BSP)头文件
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp/touch.h"

// ESP-Brookesia 框架头文件
#include "esp_brookesia.hpp"
#include "esp_brookesia_app_squareline_demo.hpp"

// 使用 ESP-Brookesia 应用命名空间
// 使用 ESP-Brookesia 应用命名空间
using namespace esp_brookesia::apps;

/**
 * @brief 是否显示内存信息的配置宏
 * 
 * 设置为 1 时，程序会在运行时周期性显示内存使用情况
 */
#define EXAMPLE_SHOW_MEM_INFO             (1)

/**
 * @brief LVGL 移植层初始化配置宏
 * 
 * 定义了 LVGL 任务的优先级、堆栈大小、CPU 亲和性等参数
 */
#define LVGL_PORT_INIT_CONFIG() \
    {                               \
        .task_priority = 4,         /* LVGL 任务优先级 */ \
        .task_stack = 10 * 1024,    /* LVGL 任务堆栈大小 (10KB) */ \
        .task_affinity = -1,        /* CPU 亲和性 (-1 表示不绑定特定核心) */ \
        .task_max_sleep_ms = 500,   /* 任务最大休眠时间 */ \
        .timer_period_ms = 5,       /* 定时器周期 */ \
    }

// 日志标签，用于标识日志输出来源
static const char *TAG = "app_main";

// 时钟更新定时器回调函数声明
// 时钟更新定时器回调函数声明
static void on_clock_update_timer_cb(struct _lv_timer_t *t);

/**
 * @brief 应用程序主入口函数
 * 
 * 该函数是 ESP-IDF 应用程序的主入口点，负责：
 * 1. 初始化显示设备和 LVGL
 * 2. 创建和配置 ESP-Brookesia 手机对象
 * 3. 安装应用程序
 * 4. 启动内存监控（如果启用）
 */
extern "C" void app_main(void)
{
    // 配置显示设备参数
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = LVGL_PORT_INIT_CONFIG()  // 使用预定义的 LVGL 配置
    };
    
    // 启动显示设备并返回 LVGL 显示对象
    lv_display_t *disp = bsp_display_start_with_config(&cfg);

    ESP_LOGI(TAG, "Display ESP-Brookesia phone demo");
    
    /**
     * 为了避免多个任务同时访问 LVGL 导致的错误，
     * 在操作 LVGL 之前需要获取锁
     */
    lvgl_port_lock(0);

    /* 创建手机对象 */
    ESP_Brookesia_Phone *phone = new ESP_Brookesia_Phone(disp);
    assert(phone && "Create phone failed");

    /* 尝试使用与分辨率对应的样式表 */
    ESP_Brookesia_PhoneStylesheet_t *stylesheet = nullptr;
    
    // 根据屏幕分辨率选择对应的样式表
    if ((BSP_LCD_H_RES == 480) && (BSP_LCD_V_RES == 480)) {
        // 480x480 分辨率使用深色样式表
        stylesheet = new ESP_Brookesia_PhoneStylesheet_t(ESP_BROOKESIA_PHONE_480_480_DARK_STYLESHEET());
        assert(stylesheet && "Create stylesheet failed");
    } else if ((BSP_LCD_H_RES == 800) && (BSP_LCD_V_RES == 480)) {
        // 800x480 分辨率使用深色样式表
        stylesheet = new ESP_Brookesia_PhoneStylesheet_t(ESP_BROOKESIA_PHONE_800_480_DARK_STYLESHEET());
        assert(stylesheet && "Create stylesheet failed");
    }
    
    // 如果成功创建样式表，则添加并激活它
    if (stylesheet != nullptr) {
        ESP_LOGI(TAG, "Using stylesheet (%s)", stylesheet->core.name);
        assert(phone->addStylesheet(stylesheet) && "Add stylesheet failed");
        assert(phone->activateStylesheet(stylesheet) && "Activate stylesheet failed");
        delete stylesheet;  // 激活后可以删除样式表对象
    }

    /* 配置并启动手机对象 */
    // 设置触摸设备
    assert(phone->setTouchDevice(bsp_display_get_input_dev()) && "Set touch device failed");
    
    // 注册 LVGL 锁定和解锁回调函数，用于线程安全访问
    phone->registerLvLockCallback((ESP_Brookesia_GUI_LockCallback_t)(lvgl_port_lock), 0);
    phone->registerLvUnlockCallback((ESP_Brookesia_GUI_UnlockCallback_t)(lvgl_port_unlock));
    
    // 启动手机对象
    assert(phone->begin() && "Begin failed");
    
    // 可选：显示容器边框（调试用，已注释）
    // assert(phone->getCoreHome().showContainerBorder(), "Show container border failed");

    /* 安装应用程序 */
    // 创建 SquareLine 演示应用实例
    SquarelineDemo *app_squareline = SquarelineDemo::requestInstance();
    assert(app_squareline && "Create app squareline failed");
    
    // 安装应用到手机系统中
    assert((phone->installApp(app_squareline) >= 0) && "Install app squareline failed");

    /* 创建定时器用于更新时钟显示 */
    // 每秒调用一次时钟更新回调函数
    assert(lv_timer_create(on_clock_update_timer_cb, 1000, phone) && "Create clock update timer failed");

    /* 释放 LVGL 锁 */
    /* 释放 LVGL 锁 */
    lvgl_port_unlock();

#if EXAMPLE_SHOW_MEM_INFO
    /* 内存信息显示功能 */
    char buffer[128];    /* 确保缓冲区足够容纳 sprintf 格式化的字符串 */
    
    // 定义内存统计变量
    size_t internal_free = 0;     // 内部 SRAM 空闲大小
    size_t internal_total = 0;    // 内部 SRAM 总大小
    size_t external_free = 0;     // 外部 PSRAM 空闲大小
    size_t external_total = 0;    // 外部 PSRAM 总大小
    
    // 无限循环，周期性显示内存信息
    while (1) {
        // 获取内部 SRAM 内存信息
        internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
        
        // 获取外部 PSRAM 内存信息
        external_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        external_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        
        // 格式化内存信息字符串
        sprintf(buffer, "   Biggest /     Free /    Total\n"
                "\t  SRAM : [%8d / %8d / %8d]\n"
                "\t PSRAM : [%8d / %8d / %8d]",
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), internal_free, internal_total,
                heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM), external_free, external_total);
        
        // 输出内存信息到日志
        ESP_LOGI(TAG, "%s", buffer);

        /**
         * lockLv() 和 unlockLv() 函数用于锁定和解锁 LVGL 任务。
         * 它们通过 registerLvLockCallback() 和 registerLvUnlockCallback() 函数注册。
         */
        phone->lockLv();
        
        // 更新"最近使用屏幕"上的内存标签显示
        // 将字节转换为 KB 单位显示
        if (!phone->getHome().getRecentsScreen()->setMemoryLabel(internal_free / 1024, internal_total / 1024,
                external_free / 1024, external_total / 1024)) {
            ESP_LOGE(TAG, "Set memory label failed");
        }
        
        phone->unlockLv();

        // 等待 2 秒后再次更新
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
#endif
}

/**
 * @brief 时钟更新定时器回调函数
 * 
 * 该函数每秒被调用一次，用于更新状态栏中的时钟显示。
 * 函数获取当前系统时间，并将小时和分钟信息更新到状态栏。
 * 
 * @param t LVGL 定时器对象指针，其 user_data 字段包含 ESP_Brookesia_Phone 对象指针
 */
static void on_clock_update_timer_cb(struct _lv_timer_t *t)
{
    time_t now;                           // 当前时间戳
    struct tm timeinfo;                   // 时间结构体
    ESP_Brookesia_Phone *phone = (ESP_Brookesia_Phone *)t->user_data;  // 从定时器获取手机对象

    // 获取当前系统时间
    time(&now);
    localtime_r(&now, &timeinfo);

    /* 由于此回调函数从 LVGL 任务中调用，因此操作 LVGL 是安全的 */
    // 更新状态栏上的时钟显示
    assert(phone->getHome().getStatusBar()->setClock(timeinfo.tm_hour, timeinfo.tm_min) && "Refresh status bar failed");
}
