/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

// ESP32手机项目主程序文件
// 基于ESP-Brookesia框架实现智能手机界面

#include "freertos/FreeRTOS.h"         // FreeRTOS实时操作系统
#include "freertos/task.h"             // FreeRTOS任务管理
#include "esp_check.h"                 // ESP错误检查宏
#include "esp_log.h"                   // ESP日志系统
#include "esp_lvgl_port.h"             // LVGL图形库移植层
#include "bsp/esp-bsp.h"               // 板级支持包
#include "bsp/display.h"               // 显示屏驱动
#include "bsp/touch.h"                 // 触摸屏驱动
#include "esp_brookesia.hpp"           // ESP-Brookesia手机框架
#include "esp_brookesia_app_squareline_demo.hpp"  // SquareLine演示应用
#include "esp_brookesia_app_game_2048_phone.hpp"  // 2048游戏应用 (Phone版本)
using namespace esp_brookesia::apps;   // 使用esp_brookesia应用命名空间

#define EXAMPLE_SHOW_MEM_INFO             (1)    // 是否显示内存信息的开关

// LVGL移植层初始化配置宏
#define LVGL_PORT_INIT_CONFIG() \
    {                               \
        .task_priority = 4,       /* LVGL任务优先级 */      \
        .task_stack = 10 * 1024,  /* LVGL任务栈大小(10KB) */ \
        .task_affinity = -1,      /* CPU亲和性(-1表示任意CPU) */ \
        .task_max_sleep_ms = 500, /* 任务最大休眠时间(ms) */ \
        .timer_period_ms = 5,     /* 定时器周期(ms) */ \
    }

static const char *TAG = "app_main";  // 日志标签

// 时钟更新定时器回调函数声明
static void on_clock_update_timer_cb(struct _lv_timer_t *t);

// 应用程序主入口函数
extern "C" void app_main(void)
{
    // 配置显示屏参数
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = LVGL_PORT_INIT_CONFIG()
    };
    // 启动显示屏并获取显示对象
    lv_display_t *disp = bsp_display_start_with_config(&cfg);

    ESP_LOGI(TAG, "Display ESP-Brookesia phone demo");  // 输出启动信息
    
    /**
     * 为了避免多个任务同时访问LVGL导致的错误，
     * 在操作LVGL之前应该获取锁。
     */
    lvgl_port_lock(0);

    /* 创建手机对象 */
    ESP_Brookesia_Phone *phone = new ESP_Brookesia_Phone(disp);
    assert(phone && "Create phone failed");  // 断言检查创建是否成功

    /* 根据屏幕分辨率选择对应的样式表 */
    ESP_Brookesia_PhoneStylesheet_t *stylesheet = nullptr;
    // 判断是否为480x480分辨率
    if ((BSP_LCD_H_RES == 480) && (BSP_LCD_V_RES == 480)) {
        stylesheet = new ESP_Brookesia_PhoneStylesheet_t(ESP_BROOKESIA_PHONE_480_480_DARK_STYLESHEET());
        assert(stylesheet && "Create stylesheet failed");
    } 
    // 判断是否为800x480分辨率
    else if ((BSP_LCD_H_RES == 800) && (BSP_LCD_V_RES == 480)) {
        stylesheet = new ESP_Brookesia_PhoneStylesheet_t(ESP_BROOKESIA_PHONE_800_480_DARK_STYLESHEET());
        assert(stylesheet && "Create stylesheet failed");
    }
    
    // 如果成功创建样式表，则添加并激活它
    if (stylesheet != nullptr) {
        ESP_LOGI(TAG, "Using stylesheet (%s)", stylesheet->core.name);
        assert(phone->addStylesheet(stylesheet) && "Add stylesheet failed");       // 添加样式表
        assert(phone->activateStylesheet(stylesheet) && "Activate stylesheet failed"); // 激活样式表
        delete stylesheet;  // 释放样式表内存
    }

    /* 配置手机对象并启动 */
    // 设置触摸设备
    assert(phone->setTouchDevice(bsp_display_get_input_dev()) && "Set touch device failed");
    // 注册LVGL锁定回调函数
    phone->registerLvLockCallback((ESP_Brookesia_GUI_LockCallback_t)(lvgl_port_lock), 0);
    // 注册LVGL解锁回调函数
    phone->registerLvUnlockCallback((ESP_Brookesia_GUI_UnlockCallback_t)(lvgl_port_unlock));
    // 启动手机系统
    assert(phone->begin() && "Begin failed");
    // 可选：显示容器边框（调试用）
    // assert(phone->getCoreHome().showContainerBorder(), "Show container border failed");

    /* 安装应用程序 */
    // 创建SquareLine演示应用实例
    SquarelineDemo *app_squareline = SquarelineDemo::requestInstance();
    assert(app_squareline && "Create app squareline failed");
    // 将应用安装到手机系统中
    assert((phone->installApp(app_squareline) >= 0) && "Install app squareline failed");

    // 创建2048游戏应用实例（Phone版本）
    Game2048 *app_game2048 = Game2048::requestInstance();
    assert(app_game2048 && "Create Game2048 app failed");
    assert((phone->installApp(app_game2048) >= 0) && "Install Game2048 app failed");
    // 如果有其他应用，可以继续安装
    // 例如：
    /*
    // 创建计算器应用实例（假设存在）
    CalculatorApp *app_calculator = CalculatorApp::requestInstance();
    assert(app_calculator && "Create calculator app failed");
    assert((phone->installApp(app_calculator) >= 0) && "Install calculator app failed");

    // 创建音乐播放器应用实例（假设存在）
    MusicPlayerApp *app_music = MusicPlayerApp::requestInstance();
    assert(app_music && "Create music player app failed");
    assert((phone->installApp(app_music) >= 0) && "Install music player app failed");

    // 创建天气应用实例（假设存在）
    WeatherApp *app_weather = WeatherApp::requestInstance();
    assert(app_weather && "Create weather app failed");
    assert((phone->installApp(app_weather) >= 0) && "Install weather app failed");
    */

    /* 创建时钟更新定时器 */
    // 每1000ms更新一次时钟显示
    assert(lv_timer_create(on_clock_update_timer_cb, 1000, phone) && "Create clock update timer failed");

    /* 释放LVGL锁 */
    lvgl_port_unlock();

#if EXAMPLE_SHOW_MEM_INFO
    // 内存信息显示缓冲区（确保缓冲区足够容纳sprintf的内容）
    char buffer[128];    
    // 内存统计变量
    size_t internal_free = 0;    // SRAM可用内存
    size_t internal_total = 0;   // SRAM总内存
    size_t external_free = 0;    // PSRAM可用内存
    size_t external_total = 0;   // PSRAM总内存
    
    // 内存监控循环
    while (1) {
        // 获取内部SRAM内存信息
        internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
        // 获取外部PSRAM内存信息
        external_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        external_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        
        // 格式化内存信息字符串
        sprintf(buffer, "   Biggest /     Free /    Total\n"
                "\t  SRAM : [%8d / %8d / %8d]\n"
                "\t PSRAM : [%8d / %8d / %8d]",
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), internal_free, internal_total,
                heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM), external_free, external_total);
        ESP_LOGI(TAG, "%s", buffer);  // 输出内存信息到日志

        /**
         * `lockLv()` 和 `unlockLv()` 函数用于锁定和解锁LVGL任务。
         * 它们通过 `registerLvLockCallback()` 和 `registerLvUnlockCallback()` 函数注册。
         */
        phone->lockLv();   // 锁定LVGL任务
        // 更新"最近应用界面"上的内存标签显示（转换为KB单位）
        if (!phone->getHome().getRecentsScreen()->setMemoryLabel(internal_free / 1024, internal_total / 1024,
                external_free / 1024, external_total / 1024)) {
            ESP_LOGE(TAG, "Set memory label failed");  // 设置内存标签失败
        }
        phone->unlockLv(); // 解锁LVGL任务

        vTaskDelay(pdMS_TO_TICKS(2000));  // 延时2秒后继续下一次内存检查
    }
#endif
}

// 时钟更新定时器回调函数
static void on_clock_update_timer_cb(struct _lv_timer_t *t)
{
    time_t now;                         // 当前时间戳
    struct tm timeinfo;                 // 时间信息结构体
    ESP_Brookesia_Phone *phone = (ESP_Brookesia_Phone *)t->user_data;  // 从定时器用户数据获取手机对象

    time(&now);                         // 获取当前时间戳
    localtime_r(&now, &timeinfo);       // 将时间戳转换为本地时间结构

    /* 由于此回调函数从LVGL任务中调用，因此可以安全地操作LVGL */
    // 更新状态栏上的时钟显示
    assert(phone->getHome().getStatusBar()->setClock(timeinfo.tm_hour, timeinfo.tm_min) && "Refresh status bar failed");
}
