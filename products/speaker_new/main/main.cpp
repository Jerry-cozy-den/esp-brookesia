/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

/* 
 * ESP-Brookesia 智能音箱主程序
 * 这是一个基于ESP32-S3的AI智能音箱，支持语音交互、应用管理、动画表情等功能
 * 
 * 系统架构说明：
 * 1. 硬件层：ESP32-S3芯片 + LCD显示屏 + 音频编解码器 + SD卡存储
 * 2. 驱动层：BSP板级支持包、LVGL图形库、音频驱动
 * 3. 服务层：NVS存储服务、Wi-Fi网络服务、USB开发者模式
 * 4. 应用层：设置管理、游戏应用、计算器、定时器等
 * 5. AI层：Coze平台集成、语音识别、语音合成、智能对话
 * 
 * 主要特性：
 * - 语音AI助手 (通过Coze平台实现智能对话)
 * - 可视化UI界面 (基于LVGL轻量级图形库)
 * - 多种内置应用 (设置、计算器、2048游戏、定时器等)
 * - 动画表情系统 (支持机器人表情动画显示)
 * - SD卡支持 (用于存储配置文件和媒体资源)
 * - USB开发者模式 (用于调试和配置文件管理)
 * 
 * 程序执行流程：
 * 1. 初始化显示系统和动画引擎
 * 2. 挂载SD卡存储
 * 3. 检查是否进入开发者模式
 * 4. 初始化音频系统
 * 5. 启动系统服务(NVS存储等)
 * 6. 加载AI代理配置
 * 7. 创建音箱实例并安装所有应用
 * 8. 注册AI函数调用接口
 * 9. 进入主循环处理用户交互
 */

#include <map>    // STL映射容器，用于存储应用名称映射关系

// ==================== FreeRTOS 实时操作系统相关头文件 ====================
#include "freertos/FreeRTOS.h"    // FreeRTOS实时操作系统核心头文件
#include "freertos/task.h"        // 任务管理：创建、删除、挂起、恢复任务
#include "freertos/semphr.h"      // 信号量：用于任务同步和资源保护
#include "esp_check.h"            // ESP错误检查宏：提供统一的错误处理机制
#include "esp_log.h"              // ESP日志系统：分级日志输出(ERROR/WARN/INFO/DEBUG)
#include "esp_event.h"            // ESP事件循环：系统事件分发和处理机制
#include "esp_spiffs.h"           // SPIFFS文件系统：轻量级文件系统，用于存储配置文件

// ==================== USB 开发者模式相关头文件 ====================
#if SOC_USB_SERIAL_JTAG_SUPPORTED
#   include "soc/usb_serial_jtag_reg.h"  // USB串行JTAG寄存器定义
#   include "hal/usb_serial_jtag_ll.h"   // USB串行JTAG底层硬件抽象接口
#endif
#include "esp_private/usb_phy.h"  // USB物理层私有接口：用于USB硬件初始化

// ==================== 图形界面和音频相关头文件 ====================
#include "lvgl.h"                 // LVGL轻量级图形库：嵌入式GUI解决方案
#include "boost/thread.hpp"       // Boost线程库：提供跨平台的线程管理功能
#include "esp_codec_dev.h"        // ESP音频编解码器设备：音频硬件抽象层

// ==================== BSP 板级支持包和显示相关头文件 ====================
#include "bsp/esp-bsp.h"          // 板级支持包：硬件相关的初始化和配置
#include "esp_lvgl_port_disp.h"   // LVGL显示端口：连接LVGL和ESP32显示驱动

// ==================== ESP-Brookesia 框架相关头文件 ====================
#include "esp_brookesia.hpp"      // ESP-Brookesia核心框架：智能音箱主框架
#include "esp_brookesia_app_settings.hpp"    // 设置应用：系统参数配置界面
#include "esp_brookesia_app_ai_profile.hpp"  // AI配置应用：AI助手个性化设置
#include "esp_brookesia_app_game_2048.hpp"   // 2048游戏应用：经典数字游戏
#include "esp_brookesia_app_calculator.hpp"  // 计算器应用：基础数学运算工具
#include "esp_brookesia_app_timer.hpp"       // 定时器应用：倒计时和提醒功能

// ==================== 工具库和自定义组件头文件 ====================
#ifdef ESP_UTILS_LOG_TAG
#   undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "Main"    // 定义日志标签，用于标识主程序的日志输出
#include "esp_lib_utils.h"          // ESP工具库：提供常用的工具函数和宏定义

#include "usb_msc.h"                // USB大容量存储类：实现USB磁盘功能
#include "audio_sys.h"              // 音频系统：音频处理和管理模块
#include "coze_agent_config.h"      // Coze AI代理配置：AI助手配置文件解析
#include "coze_agent_config_default.h"  // Coze AI代理默认配置：内置的默认AI配置

// ==================== 系统配置常量 ====================
// 这些常量定义了整个智能音箱系统的各种参数配置

// 调试和系统监控配置
constexpr bool        EXAMPLE_SHOW_MEM_INFO     = false;  // 是否显示内存使用信息(调试用)

// 存储分区配置
constexpr const char *MUSIC_PARTITION_LABEL     = "spiffs_data";  // 音乐文件存储分区标签
constexpr int         DEVELOPER_MODE_KEY        = 0x655;          // 开发者模式激活密钥

// ==================== LVGL 图形库任务配置 ====================
// LVGL是轻量级嵌入式图形库，这些参数控制其运行行为
constexpr int         LVGL_TASK_PRIORITY        = 4;         // LVGL任务优先级(数字越大优先级越高)
constexpr int         LVGL_TASK_CORE_ID         = 1;         // LVGL任务运行的CPU核心(ESP32-S3双核)
constexpr int         LVGL_TASK_STACK_SIZE      = 20 * 1024; // LVGL任务栈大小(20KB)
constexpr int         LVGL_TASK_MAX_SLEEP_MS    = 500;       // LVGL任务最大休眠时间(毫秒)
constexpr int         LVGL_TASK_TIMER_PERIOD_MS = 5;         // LVGL定时器周期(5毫秒刷新一次)
constexpr bool        LVGL_TASK_STACK_CAPS_EXT  = true;      // 是否使用外部PSRAM作为栈内存

// ==================== 音频系统参数配置 ====================
// 定义音量控制的范围和默认值
constexpr int         PARAM_SOUND_VOLUME_MIN            = 0;   // 最小音量值(静音)
constexpr int         PARAM_SOUND_VOLUME_MAX            = 100; // 最大音量值
constexpr int         PARAM_SOUND_VOLUME_DEFAULT        = 70;  // 系统默认音量

// ==================== 显示系统参数配置 ====================
// 定义屏幕亮度控制的范围和默认值
constexpr int         PARAM_DISPLAY_BRIGHTNESS_MIN      = 10;  // 最小亮度(10%,防止完全黑屏)
constexpr int         PARAM_DISPLAY_BRIGHTNESS_MAX      = 100; // 最大亮度(100%)
constexpr int         PARAM_DISPLAY_BRIGHTNESS_DEFAULT  = 100; // 系统默认亮度

// ==================== AI函数调用 - 应用启动功能配置 ====================
// 当AI助手识别到"打开XX应用"指令时，这些参数控制应用启动过程
constexpr const char *FUNCTION_OPEN_APP_THREAD_NAME               = "open_app";     // 应用启动线程名称
constexpr int         FUNCTION_OPEN_APP_THREAD_STACK_SIZE         = 10 * 1024;     // 线程栈大小(10KB)
constexpr int         FUNCTION_OPEN_APP_WAIT_SPEAKING_PRE_MS      = 2000;          // 等待AI语音播放前的延时(2秒)
constexpr int         FUNCTION_OPEN_APP_WAIT_SPEAKING_INTERVAL_MS = 10;            // 检查AI是否还在说话的间隔(10毫秒)
constexpr int         FUNCTION_OPEN_APP_WAIT_SPEAKING_MAX_MS      = 2000;          // 最长等待AI说完的时间(2秒)
constexpr bool        FUNCTION_OPEN_APP_THREAD_STACK_CAPS_EXT     = true;          // 是否使用外部PSRAM作为栈

// ==================== AI函数调用 - 音量控制功能配置 ====================
// 当AI助手识别到"调大音量"、"调小音量"等指令时，这些参数控制音量调节过程
constexpr const char *FUNCTION_VOLUME_CHANGE_THREAD_NAME           = "volume_change";  // 音量调节线程名称
constexpr size_t      FUNCTION_VOLUME_CHANGE_THREAD_STACK_SIZE     = 6 * 1024;         // 线程栈大小(6KB)
constexpr bool        FUNCTION_VOLUME_CHANGE_THREAD_STACK_CAPS_EXT = true;             // 是否使用外部PSRAM作为栈
constexpr int         FUNCTION_VOLUME_CHANGE_STEP                  = 20;               // 每次音量调节的步长值

// ==================== AI函数调用 - 亮度控制功能配置 ====================
// 当AI助手识别到"调亮屏幕"、"调暗屏幕"等指令时，这些参数控制亮度调节过程
constexpr const char *FUNCTION_BRIGHTNESS_CHANGE_THREAD_NAME           = "brightness_change";  // 亮度调节线程名称
constexpr size_t      FUNCTION_BRIGHTNESS_CHANGE_THREAD_STACK_SIZE     = 6 * 1024;             // 线程栈大小(6KB)
constexpr bool        FUNCTION_BRIGHTNESS_CHANGE_THREAD_STACK_CAPS_EXT = true;                 // 是否使用外部PSRAM作为栈
constexpr int         FUNCTION_BRIGHTNESS_CHANGE_STEP                  = 30;                   // 每次亮度调节的步长值

// ==================== 命名空间使用声明 ====================
// 简化代码中的类型引用，避免每次都写完整的命名空间路径
using namespace esp_brookesia::speaker;        // ESP-Brookesia音箱核心命名空间
using namespace esp_brookesia::gui;            // ESP-Brookesia GUI组件命名空间  
using namespace esp_brookesia::speaker_apps;   // ESP-Brookesia音箱应用命名空间
using namespace esp_brookesia::services;       // ESP-Brookesia服务命名空间
using namespace esp_brookesia::ai_framework;   // ESP-Brookesia AI框架命名空间

// ==================== 静态函数声明 ====================
// 这些函数按照系统初始化的顺序排列，每个函数负责初始化特定的子系统
static bool init_display_and_draw_logic();     // 初始化显示系统和动画绘图逻辑
static bool init_sdcard();                     // 初始化SD卡存储系统
static bool check_whether_enter_developer_mode(); // 检查是否进入USB开发者调试模式
static bool init_media_audio();                // 初始化音频编解码器和媒体播放系统
static bool init_services();                   // 初始化NVS存储等系统服务
static bool load_coze_agent_config();          // 加载AI助手(Coze)的配置参数
static bool create_speaker_and_install_apps(); // 创建音箱主对象并安装所有应用

// ==================== 全局变量定义 ====================
// 音频设备句柄 - 用于音频播放和录音功能
static esp_codec_dev_handle_t play_dev = nullptr;  // 音频播放设备句柄(扬声器)
static esp_codec_dev_handle_t rec_dev = nullptr;   // 音频录音设备句柄(麦克风)

/**
 * @brief 开发者模式密钥变量
 * 
 * 这个变量存储在RTC内存中，即使系统重启也不会被清除。
 * 当用户通过特定操作激活开发者模式时，系统会设置这个密钥值，
 * 然后重启进入开发者模式，此时设备会作为USB存储设备出现，
 * 允许用户直接修改SD卡中的配置文件。
 * 
 * RTC_NOINIT_ATTR: 指示编译器将此变量放在RTC内存的非初始化区域
 */
static RTC_NOINIT_ATTR int developer_mode_key;

#if COZE_AGENT_ENABLE_DEFAULT_CONFIG
// ==================== 嵌入式AI配置数据 ====================
// 当启用默认AI配置时，私钥数据会被编译器嵌入到程序的只读数据段中
// 这些符号由链接器生成，指向嵌入数据的起始和结束位置
extern const char private_key_pem_start[] asm("_binary_private_key_pem_start");
extern const char private_key_pem_end[]   asm("_binary_private_key_pem_end");
#endif

/**
 * @brief 智能音箱系统主函数
 * 
 * 这是整个ESP-Brookesia智能音箱系统的入口点和初始化控制中心。
 * 函数按照严格的顺序初始化各个子系统，确保系统的稳定启动。
 * 
 * 初始化顺序说明：
 * 1. 显示和绘图逻辑 - 初始化LCD屏幕和LVGL图形库，建立UI显示基础
 * 2. SD卡存储系统 - 挂载SD卡，用于存储配置文件、媒体资源等
 * 3. 开发者模式检查 - 判断是否需要进入USB调试模式进行开发调试
 * 4. 媒体音频系统 - 初始化音频编解码器，建立音频播放和录音基础
 * 5. 系统服务层 - 启动NVS存储服务，加载用户设置参数
 * 6. AI代理配置 - 加载Coze AI助手的配置参数和认证信息
 * 7. 音箱主体创建 - 创建音箱核心对象，安装所有应用程序
 * 8. 内存监控(可选) - 启动内存使用监控线程(仅调试模式)
 * 
 * @note 此函数使用assert()确保关键初始化步骤必须成功，
 *       如果任何步骤失败，系统将停止运行并输出错误信息。
 */
extern "C" void app_main()
{
    // 打印项目版本信息，用于调试和版本追踪
    printf("Project version: %s\n", CONFIG_APP_PROJECT_VER);

    // ==================== 系统初始化阶段 ====================
    // 按照依赖关系的顺序初始化各个子系统，使用assert确保每步都成功
    
    // 第1步：初始化显示系统 - LCD屏幕、LVGL图形库、动画引擎
    assert(init_display_and_draw_logic()        && "Initialize display and draw logic failed");
    
    // 第2步：初始化SD卡存储 - 挂载文件系统，准备配置文件和资源访问
    assert(init_sdcard()                        && "Initialize SD card failed");
    
    // 第3步：开发者模式检查 - 如果激活了开发者模式，系统会变成USB磁盘
    assert(check_whether_enter_developer_mode() && "Check whether enter developer mode failed");
    
    // 第4步：初始化音频系统 - 音频编解码器、播放设备、录音设备
    assert(init_media_audio()                   && "Initialize media audio failed");
    
    // 第5步：初始化系统服务 - NVS存储、用户设置参数加载
    assert(init_services()                      && "Initialize services failed");
    
    // 第6步：加载AI助手配置 - Coze平台的认证信息和机器人配置
    // 注意：这一步允许失败，系统会使用内置的默认配置
    if (!load_coze_agent_config()) {
        ESP_UTILS_LOGE("Load coze agent config failed, will use default config");
    }
    
    // 第7步：创建音箱主对象 - 启动完整的UI系统和所有应用程序
    assert(create_speaker_and_install_apps()    && "Create speaker and install apps failed");

    // ==================== 可选的系统监控功能 ====================
    // 仅在调试模式下启用内存使用监控，用于性能分析和内存泄漏检测
    if constexpr (EXAMPLE_SHOW_MEM_INFO) {
        // 配置内存监控线程的参数
        esp_utils::thread_config_guard thread_config({
            .name = "mem_info",           // 线程名称
            .stack_size = 4096,           // 栈大小4KB
        });
        
        // 创建后台监控线程，定期输出系统内存使用信息
        boost::thread([ = ]() {
            char buffer[512];    /* 确保缓冲区足够大以容纳sprintf的内容 */

            while (1) {
                // 可选：堆内存完整性检查(耗时较长，通常注释掉)
                // heap_caps_check_integrity_all(true);

                // 打印ESP32系统内存使用情况
                esp_utils_mem_print_info();

                // 获取并打印LVGL图形库的内存使用情况
                lv_mem_monitor_t mon;
                lv_mem_monitor(&mon);
                sprintf(buffer, "LVGL Memory - used: %zu (%3d %%), frag: %3d %%, biggest free: %zu, total: %zu, free: %zu",
                        mon.total_size - mon.free_size, mon.used_pct, mon.frag_pct,
                        mon.free_biggest_size, mon.total_size, mon.free_size);
                ESP_UTILS_LOGI("%s", buffer);

                // 获取音频系统的实时统计信息
                audio_sys_get_real_time_stats();

                // 每5秒输出一次监控信息
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
        }).detach();    // 分离线程，让其在后台独立运行
    }
}

// ==================== 显示系统底层绘图函数 ====================
/**
 * @brief 线程安全的位图绘制函数
 * 
 * 这个函数是显示系统的核心绘图接口，负责将像素数据直接绘制到LCD屏幕上。
 * 使用互斥锁确保多线程环境下的绘图操作不会冲突。
 * 主要用于动画播放器直接向屏幕输出帧数据，绕过LVGL的渲染管线。
 * 
 * @param disp LVGL显示对象指针，包含显示设备信息
 * @param x_start 绘制区域的起始X坐标
 * @param y_start 绘制区域的起始Y坐标  
 * @param x_end 绘制区域的结束X坐标
 * @param y_end 绘制区域的结束Y坐标
 * @param data 像素数据指针，通常是RGB565格式
 * @return true 绘制成功，false 绘制失败
 * 
 * @note 此函数会等待上一帧传输完成，确保数据完整性
 */
static bool draw_bitmap_with_lock(lv_disp_t *disp, int x_start, int y_start, int x_end, int y_end, const void *data)
{
    // ESP_UTILS_LOG_TRACE_GUARD();  // 调试跟踪(通常注释掉以提高性能)

    // 静态互斥锁，确保同一时间只有一个线程进行绘图操作
    static boost::mutex draw_mutex;

    // 参数有效性检查
    ESP_UTILS_CHECK_NULL_RETURN(disp, false, "Invalid display object");
    ESP_UTILS_CHECK_NULL_RETURN(data, false, "Invalid pixel data");

    // 从LVGL显示对象中获取底层的LCD面板句柄
    auto panel_handle = static_cast<esp_lcd_panel_handle_t>(lv_display_get_user_data(disp));
    ESP_UTILS_CHECK_NULL_RETURN(panel_handle, false, "Failed to get LCD panel handle");

    // 使用RAII风格的锁保护，确保函数退出时自动释放锁
    std::lock_guard<boost::mutex> lock(draw_mutex);

    // 获取LVGL的传输信号量，确保显示驱动就绪
    lvgl_port_disp_take_trans_sem(disp, 0);
    
    // 执行实际的位图绘制操作，将数据传输到LCD控制器
    ESP_UTILS_CHECK_ERROR_RETURN(
        esp_lcd_panel_draw_bitmap(panel_handle, x_start, y_start, x_end, y_end, data), 
        false, "LCD panel draw bitmap operation failed"
    );

    // 等待LCD传输完成，确保数据已完全写入显示缓冲区
    ESP_UTILS_CHECK_ERROR_RETURN(
        lvgl_port_disp_take_trans_sem(disp, portMAX_DELAY), 
        false, "Failed to wait for LCD transmission completion"
    );
    
    // 释放传输信号量，允许其他绘图操作继续
    lvgl_port_disp_give_trans_sem(disp, false);

    return true;
}

/**
 * @brief 清除整个显示屏幕
 * 
 * 此函数用于将整个LCD屏幕清除为黑色(像素值为0)。
 * 主要在系统初始化、模式切换或动画播放前使用，确保屏幕内容干净。
 * 
 * @param disp LVGL显示对象指针
 * @return true 清屏成功，false 清屏失败
 * 
 * @note 函数会创建一个临时的像素缓冲区，大小为整个屏幕的像素数据
 *       对于360x360分辨率的RGB565格式屏幕，需要259.2KB的临时内存
 */
static bool clear_display(lv_disp_t *disp)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 创建全黑的像素缓冲区
    // BSP_LCD_H_RES * BSP_LCD_V_RES * 2: 宽度 x 高度 x 2字节(RGB565)
    std::vector<uint8_t> buffer(BSP_LCD_H_RES * BSP_LCD_V_RES * 2, 0);
    
    // 使用绘图函数将黑色缓冲区绘制到整个屏幕
    ESP_UTILS_CHECK_FALSE_RETURN(
        draw_bitmap_with_lock(disp, 0, 0, BSP_LCD_H_RES, BSP_LCD_V_RES, buffer.data()), 
        false, "Failed to draw black screen buffer"
    );

    return true;
}

/**
 * @brief 初始化显示系统和绘图逻辑
 * 
 * 这是显示系统的核心初始化函数，负责：
 * 1. 初始化BSP(板级支持包)和电源管理
 * 2. 配置并启动LVGL图形库和LCD显示驱动
 * 3. 设置动画播放器的事件处理机制
 * 4. 建立虚拟绘制模式的控制机制
 * 
 * 关键概念说明：
 * - 虚拟绘制模式：允许动画直接绘制到屏幕，暂停LVGL的UI渲染
 * - 双缓冲技术：使用两个显示缓冲区，避免画面撕裂和闪烁
 * - 信号槽机制：使用观察者模式处理动画事件
 * 
 * @return true 初始化成功，false 初始化失败
 */
static bool init_display_and_draw_logic()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 虚拟绘制模式标志：true=动画直接绘制，false=LVGL正常渲染
    static bool is_lvgl_dummy_draw = true;

    // ==================== BSP 和显示驱动初始化 ====================
    
    // 初始化板级电源管理(参数0表示使用默认配置)
    bsp_power_init(0);
    
    // 配置显示系统参数
    bsp_display_cfg_t cfg = {
        // LVGL任务配置
        .lvgl_port_cfg = {
            .task_priority = LVGL_TASK_PRIORITY,        // 任务优先级
            .task_stack = LVGL_TASK_STACK_SIZE,         // 任务栈大小
            .task_affinity = LVGL_TASK_CORE_ID,         // 绑定到指定CPU核心
            .task_max_sleep_ms = LVGL_TASK_MAX_SLEEP_MS, // 任务最大休眠时间
            .task_stack_caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, // 使用外部PSRAM
            .timer_period_ms = LVGL_TASK_TIMER_PERIOD_MS, // 定时器周期
        },
        // 显示缓冲区配置
        .buffer_size = BSP_LCD_H_RES * BSP_LCD_V_RES,  // 缓冲区大小=屏幕像素总数
        .double_buffer = true,                          // 启用双缓冲技术
        .flags = {
            .buff_spiram = true,                        // 显示缓冲区使用外部PSRAM
            // 根据是否处于开发者模式决定是否启用虚拟绘制
            .default_dummy_draw = (developer_mode_key != DEVELOPER_MODE_KEY), 
        },
    };
    
    // 启动显示驱动，返回LVGL显示对象
    auto disp = bsp_display_start_with_config(&cfg);
    ESP_UTILS_CHECK_NULL_RETURN(disp, false, "Failed to start display with configuration");
    
    // 在非开发者模式下清屏，避免启动时的白屏现象
    if (developer_mode_key != DEVELOPER_MODE_KEY) {
        ESP_UTILS_CHECK_FALSE_RETURN(clear_display(disp), false, "Failed to clear display during initialization");
        vTaskDelay(pdMS_TO_TICKS(100)); // 延时100ms避免雪花屏现象
    }
    
    // 打开LCD背光，显示内容变为可见
    bsp_display_backlight_on();

    // ==================== 动画播放器事件处理 ====================
    // 这部分是实现AI机器人表情动画的核心机制
    
    /**
     * 动画帧刷新信号处理
     * 当动画播放器准备好一帧新数据时触发此回调
     * 负责将动画帧数据直接绘制到LCD屏幕上
     */
    AnimPlayer::flush_ready_signal.connect(
        [ = ](int x_start, int y_start, int x_end, int y_end, const void *data, void *user_data
    ) {
        // ESP_UTILS_LOGD("Animation flush: area(%d,%d,%d,%d)", x_start, y_start, x_end, y_end);

        // 只有在虚拟绘制模式下才直接绘制到屏幕
        // 这样可以避免与LVGL的正常UI渲染产生冲突
        if (is_lvgl_dummy_draw) {
            ESP_UTILS_CHECK_FALSE_EXIT(
                draw_bitmap_with_lock(disp, x_start, y_start, x_end, y_end, data), 
                "Failed to draw animation frame to screen"
            );
        }

        // 通知动画播放器当前帧已完成绘制，可以继续下一帧
        auto player = static_cast<AnimPlayer *>(user_data);
        ESP_UTILS_CHECK_NULL_EXIT(player, "Invalid animation player pointer");
        player->notifyFlushFinished();
    });
    
    /**
     * 动画停止信号处理
     * 当动画播放结束时触发此回调
     * 负责清除动画占用的屏幕区域，为恢复UI界面做准备
     */
    AnimPlayer::animation_stop_signal.connect(
        [ = ](int x_start, int y_start, int x_end, int y_end, void *user_data
    ) {
        // ESP_UTILS_LOGD("Animation stop: clear area(%d,%d,%d,%d)", x_start, y_start, x_end, y_end);

        // 只有在虚拟绘制模式下才需要清除动画区域
        if (is_lvgl_dummy_draw) {
            // 创建黑色像素缓冲区，大小为动画区域的像素数据量
            std::vector<uint8_t> buffer((x_end - x_start) * (y_end - y_start) * 2, 0);
            ESP_UTILS_CHECK_FALSE_EXIT(
                draw_bitmap_with_lock(disp, x_start, y_start, x_end, y_end, buffer.data()), 
                "Failed to clear animation area after stop"
            );
        }
    });
    
    // ==================== 虚拟绘制模式控制机制 ====================
    /**
     * 虚拟绘制模式切换信号处理
     * 
     * 虚拟绘制模式的工作原理：
     * - enable=true (动画模式): LVGL停止渲染UI，动画播放器直接向屏幕绘制
     * - enable=false (UI模式): 恢复LVGL正常渲染，显示应用界面
     * 
     * 这种机制确保了动画播放和UI显示不会相互干扰，
     * 同时也避免了两个渲染系统同时操作屏幕导致的显示异常。
     */
    Display::on_dummy_draw_signal.connect([ = ](bool enable) {
        ESP_UTILS_LOGI("Switching display mode: %s", enable ? "Animation" : "UI");

        // 获取显示传输信号量，确保模式切换时没有其他绘图操作
        ESP_UTILS_CHECK_ERROR_EXIT(
            lvgl_port_disp_take_trans_sem(disp, portMAX_DELAY), 
            "Failed to acquire display semaphore for mode switch"
        );
        
        // 切换LVGL的虚拟绘制模式
        lvgl_port_disp_set_dummy_draw(disp, enable);
        
        // 释放显示传输信号量
        lvgl_port_disp_give_trans_sem(disp, false);

        if (!enable) {
            // 恢复UI模式：重新激活LVGL界面渲染
            bsp_display_lock(0);    // 获取显示锁
            lv_obj_invalidate(lv_screen_active());  // 标记当前屏幕需要重绘
            bsp_display_unlock();   // 释放显示锁
        } else {
            // 进入动画模式：清空屏幕为动画播放做准备
            ESP_UTILS_CHECK_FALSE_EXIT(clear_display(disp), "Failed to clear display for animation mode");
        }

        // 更新本地的模式标志
        is_lvgl_dummy_draw = enable;
    });

    return true;
}

/**
 * @brief 初始化SD卡存储系统
 * 
 * SD卡是智能音箱的重要存储介质，用于存储：
 * - AI助手配置文件 (bot_setting.json)
 * - 加密密钥文件 (private_key.pem)
 * - 媒体资源文件 (音频、图像等)
 * - 用户数据和设置
 * 
 * 如果SD卡未插入或读取失败，系统会显示提示信息并等待用户插入SD卡。
 * 这是一个阻塞式的初始化过程，确保SD卡可用后才继续系统启动。
 * 
 * @return true SD卡初始化成功，false 初始化失败
 * 
 * @note 此函数会临时切换显示模式来显示错误提示界面
 */
static bool init_sdcard()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 尝试挂载SD卡文件系统
    auto ret = bsp_sdcard_mount();
    if (ret == ESP_OK) {
        ESP_UTILS_LOGI("SD card mounted successfully");
        return true;
    } else {
        ESP_UTILS_LOGE("Failed to mount SD card: %s", esp_err_to_name(ret));
    }

    // ==================== SD卡错误处理和用户提示 ====================
    
    // 切换到UI显示模式，准备显示错误提示界面
    Display::on_dummy_draw_signal(false);

    // 获取显示锁，开始创建提示界面
    bsp_display_lock(0);

    // 创建错误提示标签
    auto label = lv_label_create(lv_screen_active());
    lv_obj_set_size(label, 300, LV_SIZE_CONTENT);                      // 设置标签尺寸
    lv_obj_set_style_text_font(label, &esp_brookesia_font_maison_neue_book_26, 0); // 设置字体
    lv_label_set_text(label, "SD card not found, please insert a SD card!");       // 设置提示文本
    lv_obj_center(label);                                               // 居中显示

    // 释放显示锁，显示提示界面
    bsp_display_unlock();

    // ==================== 等待SD卡插入的循环 ====================
    // 持续尝试挂载SD卡，直到成功为止
    while ((ret = bsp_sdcard_mount()) != ESP_OK) {
        ESP_UTILS_LOGE("SD card mount failed: %s, retrying in 1 second...", esp_err_to_name(ret));
        vTaskDelay(pdMS_TO_TICKS(1000));  // 等待1秒后重试
    }

    // SD卡挂载成功，清除错误提示界面
    bsp_display_lock(0);
    lv_obj_del(label);        // 删除提示标签
    bsp_display_unlock();

    // 切换回动画显示模式，准备继续系统初始化
    Display::on_dummy_draw_signal(true);

    ESP_UTILS_LOGI("SD card successfully mounted after retry");
    return true;
}

// ==================== USB开发者模式相关函数 ====================

/**
 * @brief USB串行JTAG物理层初始化
 * 
 * 这个函数配置USB串行JTAG的物理层参数，包括上拉/下拉电阻配置。
 * 主要用于从开发者模式退出时重新初始化USB硬件，确保USB通信正常。
 * 
 * @note 这是一个底层硬件配置函数，直接操作寄存器
 */
static void _usb_serial_jtag_phy_init()
{
    // 启用PAD拉电阻覆盖控制
    SET_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_PAD_PULL_OVERRIDE);
    
    // 清除DP上拉电阻，设置DP下拉电阻(USB断开状态)
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLUP);
    SET_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLDOWN);
    
    // 等待10ms确保信号稳定
    vTaskDelay(pdMS_TO_TICKS(10));

#if USB_SERIAL_JTAG_LL_EXT_PHY_SUPPORTED
    // 对于支持外部PHY的芯片，禁用外部PHY并启用内部PHY
    usb_serial_jtag_ll_phy_enable_external(false);  // 使用内部PHY
    usb_serial_jtag_ll_phy_enable_pad(true);        // 启用USB PHY pads
#else
    // 对于不支持外部PHY的芯片，设置默认PHY参数
    usb_serial_jtag_ll_phy_set_defaults();          // 设置默认PHY值
#endif

    // 清除DP下拉电阻，设置DP上拉电阻(USB连接状态)
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLDOWN);
    SET_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLUP);
    
    // 禁用PAD拉电阻覆盖控制，恢复正常工作模式
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_PAD_PULL_OVERRIDE);
}

/**
 * @brief 检查是否进入开发者模式
 * 
 * 开发者模式是一个特殊的系统状态，允许开发者通过USB直接访问设备的SD卡内容。
 * 当用户在设置应用中激活开发者模式时，系统会设置特殊的密钥并重启。
 * 重启后，如果检测到密钥匹配，系统将进入开发者模式：
 * 
 * 开发者模式功能：
 * 1. 设备作为USB大容量存储设备出现在电脑上
 * 2. 用户可以直接编辑SD卡中的配置文件
 * 3. 支持修改AI助手配置(bot_setting.json)和密钥文件(private_key.pem)
 * 4. 提供图形界面指导用户操作
 * 5. 提供退出按钮来退出开发者模式
 * 
 * @return true 正常模式或开发者模式处理完成，false 处理失败
 * 
 * @note 在开发者模式下，此函数会进入无限循环，直到用户点击退出按钮
 */
static bool check_whether_enter_developer_mode()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 检查是否设置了开发者模式密钥
    if (developer_mode_key != DEVELOPER_MODE_KEY) {
        ESP_UTILS_LOGI("Normal startup mode - developer mode not activated");
        return true;
    }

    ESP_UTILS_LOGI("Developer mode activated - entering USB mass storage mode");

    // ==================== 开发者模式界面创建 ====================
    
    // 获取显示锁，开始创建开发者模式界面
    bsp_display_lock(0);

    // 创建标题标签
    auto title_label = lv_label_create(lv_screen_active());
    lv_obj_set_size(title_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);                    // 自适应尺寸
    lv_obj_set_style_text_font(title_label, &esp_brookesia_font_maison_neue_book_26, 0); // 26号字体
    lv_label_set_text(title_label, "Developer Mode");                                 // 标题文本
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 60);                              // 顶部居中，向下偏移60像素

    // 创建说明内容标签
    auto content_label = lv_label_create(lv_screen_active());
    lv_obj_set_size(content_label, LV_PCT(80), LV_SIZE_CONTENT);                      // 宽度80%，高度自适应
    lv_obj_set_style_text_font(content_label, &esp_brookesia_font_maison_neue_book_18, 0); // 18号字体
    lv_obj_set_style_text_align(content_label, LV_TEXT_ALIGN_CENTER, 0);             // 文本居中对齐
    
    // 设置详细的使用说明文本
    lv_label_set_text(
        content_label, 
        "Please connect the device to your computer via USB. A USB drive will appear. "
        "You can create or modify configuration files in the SD card "
        "(such as `bot_setting.json` and `private_key.pem`) as needed."
    );
    lv_obj_align_to(content_label, title_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20); // 位于标题下方，间距20像素

    // 创建退出按钮
    auto exit_button = lv_btn_create(lv_screen_active());
    lv_obj_set_size(exit_button, LV_SIZE_CONTENT, LV_SIZE_CONTENT);               // 按钮尺寸自适应内容
    lv_obj_align(exit_button, LV_ALIGN_BOTTOM_MID, 0, -60);                      // 底部居中，向上偏移60像素
    
    // 为退出按钮添加点击事件处理
    lv_obj_add_event_cb(exit_button, [](lv_event_t *e) {
        ESP_UTILS_LOGI("User requested to exit developer mode");
        
        // 清除开发者模式密钥，下次启动将进入正常模式
        developer_mode_key = 0;
        
        // 重新初始化USB物理层
        _usb_serial_jtag_phy_init();
        
        // 重启系统进入正常工作模式
        esp_restart();
    }, LV_EVENT_CLICKED, nullptr);

    // 创建按钮内的文本标签
    auto label_button = lv_label_create(exit_button);
    lv_obj_set_style_text_font(label_button, &esp_brookesia_font_maison_neue_book_16, 0); // 16号字体
    lv_label_set_text(label_button, "Exit and reboot");                          // 按钮文本
    lv_obj_center(label_button);                                                 // 文本在按钮中居中

    // 释放显示锁，显示开发者模式界面
    bsp_display_unlock();

    // ==================== 启动USB大容量存储模式 ====================
    
    // 将设备配置为USB大容量存储设备，允许电脑访问SD卡内容
    // 注意：这里注释掉了另一种实现方式 mount_wl_basic_and_tusb()
    ESP_UTILS_CHECK_ERROR_RETURN(
        usb_msc_mount(), false, "Failed to mount USB Mass Storage Class device"
    );

    ESP_UTILS_LOGI("USB mass storage mode activated - device is now accessible as USB drive");

    // ==================== 开发者模式主循环 ====================
    // 在开发者模式下，系统进入无限循环等待用户操作
    // 只有当用户点击"Exit and reboot"按钮时才会退出循环并重启系统
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // 每秒检查一次，减少CPU占用
    }
}

/**
 * @brief 初始化媒体音频系统
 * 
 * 这个函数负责初始化整个音频子系统，包括：
 * 1. 检测PCB版本信息，获取音频硬件配置
 * 2. 配置I2S音频接口和音频编解码器(ES7210+ES8311)
 * 3. 初始化音频管理器，创建播放和录音设备句柄
 * 4. 启动音频提示系统
 * 5. 挂载SPIFFS音乐分区，用于存储音频文件
 * 
 * 音频系统架构：
 * - ES7210: ADC音频编解码器，负责麦克风录音
 * - ES8311: DAC音频编解码器，负责扬声器播放
 * - I2S接口: 数字音频传输接口
 * - SPIFFS分区: 存储音频文件的闪存分区
 * 
 * @return true 音频系统初始化成功，false 初始化失败
 * 
 * @note 此函数会根据PCB版本自动配置正确的引脚映射
 */
static bool init_media_audio()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // ==================== PCB版本检测和硬件配置 ====================
    
    // 检测PCB版本，获取硬件差异信息(主要是音频引脚配置)
    bsp_pcd_diff_info_t pcb_info = {};
    ESP_UTILS_CHECK_ERROR_RETURN(
        bsp_pcb_version_detect(&pcb_info), false, "Failed to detect PCB version information"
    );

    // ==================== 音频外设硬件信息配置 ====================

    // 配置音频系统的硬件参数，包括I2C、I2S和编解码器设置
    esp_gmf_setup_periph_hardware_info periph_info = {
        // I2C配置 - 用于控制音频编解码器芯片
        .i2c = {
            .handle = bsp_i2c_get_handle(),    // 获取BSP提供的I2C句柄
        },
        // 音频编解码器配置
        .codec = {
            .io_pa = pcb_info.audio.pa_pin,    // 功放使能引脚(Power Amplifier Enable)
            .type = ESP_GMF_CODEC_TYPE_ES7210_IN_ES8311_OUT,  // 编解码器类型组合
            
            // DAC配置 - 数字到模拟转换器(用于音频播放)
            .dac = {
                .io_mclk = BSP_I2S_MCLK,       // 主时钟引脚
                .io_bclk = BSP_I2S_SCLK,       // 位时钟引脚
                .io_ws = BSP_I2S_LCLK,         // 字选择引脚(左右声道切换)
                .io_do = BSP_I2S_DOUT,         // 数据输出引脚
                .io_di = pcb_info.audio.i2s_din_pin,  // 数据输入引脚(PCB版本相关)
                .sample_rate = 16000,
                .channel = 2,
                .bits_per_sample = 32,
                .port_num  = 0,
            },
            .adc = {
                .io_mclk = BSP_I2S_MCLK,
                .io_bclk = BSP_I2S_SCLK,
                .io_ws = BSP_I2S_LCLK,
                .io_do = BSP_I2S_DOUT,
                .io_di = pcb_info.audio.i2s_din_pin,
                .sample_rate = 16000,
                .channel = 2,
                .bits_per_sample = 32,
                .port_num  = 0,
            },
        },
    };
    
    // ==================== 音频管理器初始化 ====================
    
    // 初始化音频管理器，创建播放和录音设备句柄
    ESP_UTILS_CHECK_ERROR_RETURN(
        audio_manager_init(&periph_info, &play_dev, &rec_dev), false, 
        "Failed to initialize audio manager and create device handles"
    );
    
    // 启动音频提示系统(用于播放系统提示音)
    ESP_UTILS_CHECK_ERROR_RETURN(
        audio_prompt_open(), false, "Failed to open audio prompt system"
    );

    // ==================== SPIFFS音乐分区初始化 ====================
    
    // 配置SPIFFS文件系统，用于存储和访问音频文件
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",                    // 挂载路径
        .partition_label = MUSIC_PARTITION_LABEL,  // 分区标签
        .max_files = 5,                            // 最大同时打开文件数
        .format_if_mount_failed = false            // 挂载失败时不自动格式化
    };
    
    auto ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        // 根据不同错误类型提供详细的错误信息
        if (ret == ESP_FAIL) {
            ESP_UTILS_CHECK_FALSE_RETURN(false, false, "Failed to mount or format SPIFFS filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_UTILS_CHECK_FALSE_RETURN(false, false, "SPIFFS partition not found in partition table");
        } else {
            ESP_UTILS_CHECK_FALSE_RETURN(false, false, "Failed to initialize SPIFFS: %s", esp_err_to_name(ret));
        }
    }
    
    // 获取并显示SPIFFS分区使用情况
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(MUSIC_PARTITION_LABEL, &total, &used);
    if (ret != ESP_OK) {
        ESP_UTILS_CHECK_FALSE_RETURN(
            false, false, "Failed to get SPIFFS partition information: %s", esp_err_to_name(ret)
        );
    } else {
        ESP_UTILS_LOGI("SPIFFS partition - Total: %d bytes, Used: %d bytes (%.1f%%)", 
                       total, used, (float)used/total*100);
    }

    ESP_UTILS_LOGI("Audio system initialization completed successfully");
    return true;
}

/**
 * @brief 设置媒体音频播放音量
 * 
 * 此函数负责调整系统的音频播放音量，同时将设置保存到NVS存储中。
 * 音量调节会影响所有音频输出，包括AI语音回复、系统提示音等。
 * 
 * @param volume 目标音量值(0-100)，会自动限制到有效范围内
 * @return true 音量设置成功，false 设置失败
 * 
 * @note 音量值会被自动限制在PARAM_SOUND_VOLUME_MIN到PARAM_SOUND_VOLUME_MAX范围内
 */
static bool set_media_sound_volume(int volume)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 检查播放设备句柄有效性
    ESP_UTILS_CHECK_NULL_RETURN(play_dev, false, "Audio play device handle is invalid");

    ESP_UTILS_LOGI("Setting volume to: %d", volume);

    // 将音量值限制在有效范围内
    volume = std::clamp(volume, PARAM_SOUND_VOLUME_MIN, PARAM_SOUND_VOLUME_MAX);
    
    // 设置音频编解码器的输出音量
    ESP_UTILS_CHECK_FALSE_RETURN(
        esp_codec_dev_set_out_vol(play_dev, volume) == ESP_CODEC_DEV_OK, false, 
        "Failed to set audio codec output volume"
    );
    
    // 将音量设置保存到NVS存储，确保重启后设置依然有效
    ESP_UTILS_CHECK_FALSE_RETURN(
        StorageNVS::requestInstance().setLocalParam(SETTINGS_NVS_KEY_VOLUME, volume), false,
        "Failed to save volume setting to NVS storage"
    );

    return true;
}

/**
 * @brief 获取当前媒体音频播放音量
 * 
 * 从NVS存储中读取当前保存的音量设置。
 * 如果读取失败，返回系统默认音量值。
 * 
 * @return 当前音量值(0-100)，读取失败时返回默认值
 */
static int get_media_sound_volume()
{
    StorageNVS::Value volume;
    ESP_UTILS_CHECK_FALSE_RETURN(
        StorageNVS::requestInstance().getLocalParam(SETTINGS_NVS_KEY_VOLUME, volume),
        PARAM_SOUND_VOLUME_DEFAULT, "Failed to get volume from NVS, using default value"
    );

    return std::get<int>(volume);
}

/**
 * @brief 设置显示屏背光亮度
 * 
 * 此函数负责调整LCD屏幕的背光亮度，同时将设置保存到NVS存储中。
 * 亮度调节会直接影响用户体验和设备功耗。
 * 
 * @param brightness 目标亮度值(10-100)，会自动限制到有效范围内
 * @return true 亮度设置成功，false 设置失败
 * 
 * @note 亮度最小值为10%，避免屏幕过暗导致无法操作
 */
static bool set_media_display_brightness(int brightness)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    ESP_UTILS_LOGI("Setting display brightness to: %d", brightness);

    // 将亮度值限制在有效范围内(10%-100%)
    brightness = std::clamp(brightness, PARAM_DISPLAY_BRIGHTNESS_MIN, PARAM_DISPLAY_BRIGHTNESS_MAX);
    
    // 设置LCD背光亮度
    ESP_UTILS_CHECK_FALSE_RETURN(
        bsp_display_brightness_set(brightness) == ESP_OK, false, 
        "Failed to set LCD display brightness"
    );
    
    // 将亮度设置保存到NVS存储，确保重启后设置依然有效
    ESP_UTILS_CHECK_FALSE_RETURN(
        StorageNVS::requestInstance().setLocalParam(SETTINGS_NVS_KEY_BRIGHTNESS, brightness), false,
        "Failed to save brightness setting to NVS storage"
    );

    return true;
}

/**
 * @brief 获取当前显示屏背光亮度
 * 
 * 从NVS存储中读取当前保存的亮度设置。
 * 如果读取失败，返回系统默认亮度值。
 * 
 * @return 当前亮度值(10-100)，读取失败时返回默认值
 */
static int get_media_display_brightness()
{
    StorageNVS::Value brightness;
    ESP_UTILS_CHECK_FALSE_RETURN(
        StorageNVS::requestInstance().getLocalParam(SETTINGS_NVS_KEY_BRIGHTNESS, brightness),
        PARAM_DISPLAY_BRIGHTNESS_DEFAULT, "Failed to get brightness from NVS, using default value"
    );

    return std::get<int>(brightness);
}

/**
 * @brief 初始化系统服务
 * 
 * 此函数负责启动和初始化系统核心服务，主要包括：
 * 1. 启动NVS(非易失性存储)服务
 * 2. 从NVS中加载用户设置参数
 * 3. 应用音量和亮度设置到硬件
 * 
 * NVS存储用于保存用户的个性化设置，确保设备重启后
 * 设置依然保持，提供一致的用户体验。
 * 
 * @return true 服务初始化成功，false 初始化失败
 */
static bool init_services()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // ==================== NVS存储服务启动 ====================
    
    // 启动NVS存储服务，这是所有持久化设置的基础
    ESP_UTILS_CHECK_FALSE_RETURN(
        StorageNVS::requestInstance().begin(), false, 
        "Failed to initialize NVS storage service"
    );

    // ==================== 系统设置参数初始化 ====================
    
    // 初始化音量设置
    StorageNVS::Value volume = PARAM_SOUND_VOLUME_DEFAULT;
    if (!StorageNVS::requestInstance().getLocalParam(SETTINGS_NVS_KEY_VOLUME, volume)) {
        // 如果NVS中没有音量设置，使用默认值并记录警告
        ESP_UTILS_LOGW("Volume setting not found in NVS, using default value: %d", std::get<int>(volume));
    }
    // 应用音量设置到音频硬件
    ESP_UTILS_CHECK_FALSE_RETURN(
        set_media_sound_volume(std::get<int>(volume)), false, 
        "Failed to apply volume setting to audio hardware"
    );
    
    // 初始化显示亮度设置
    StorageNVS::Value brightness = PARAM_DISPLAY_BRIGHTNESS_DEFAULT;
    if (!StorageNVS::requestInstance().getLocalParam(SETTINGS_NVS_KEY_BRIGHTNESS, brightness)) {
        // 如果NVS中没有亮度设置，使用默认值并记录警告
        ESP_UTILS_LOGW("Brightness setting not found in NVS, using default value: %d", std::get<int>(brightness));
    }
    // 应用亮度设置到显示硬件
    ESP_UTILS_CHECK_FALSE_RETURN(
        set_media_display_brightness(std::get<int>(brightness)), false, 
        "Failed to apply brightness setting to display hardware"
    );

    ESP_UTILS_LOGI("System services initialization completed successfully");
    return true;
}

// ==================== 字符串处理工具函数 ====================

/**
 * @brief 将字符串转换为小写
 * 
 * 用于应用名称的标准化处理，确保不区分大小写的应用名称匹配。
 * 
 * @param input 输入字符串
 * @return 转换后的小写字符串
 */
static std::string to_lower(const std::string &input)
{
    std::string result = input;
    for (char &c : result) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return result;
}

/**
 * @brief 获取字符串中第一个空格之前的部分
 * 
 * 用于从AI识别的应用名称中提取关键词。
 * 例如："settings app" -> "settings"
 * 
 * @param input 输入字符串
 * @return 第一个空格之前的子字符串，如果没有空格则返回整个字符串
 */
static std::string get_before_space(const std::string &input)
{
    size_t pos = input.find(' ');
    return input.substr(0, pos);
}

/**
 * @brief 加载Coze AI代理配置
 * 
 * 这个函数负责加载AI助手(Coze平台)的配置信息，包括：
 * 1. 从SD卡读取用户自定义配置文件(bot_setting.json)
 * 2. 如果配置文件不存在，使用编译时嵌入的默认配置
 * 3. 解析认证信息(App ID、公钥、私钥等)
 * 4. 配置机器人信息(名称、ID、语音ID、描述等)
 * 5. 将配置应用到AI代理实例
 * 
 * 配置优先级：
 * 1. SD卡中的用户配置文件(最高优先级)
 * 2. 编译时嵌入的默认配置(备用)
 * 
 * @return true 配置加载成功，false 加载失败
 * 
 * @note 此函数允许失败，系统会使用内置默认配置继续运行
 */
static bool load_coze_agent_config()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 准备配置数据结构
    coze_agent_config_t config = {};           // 原始配置数据
    CozeChatAgentInfo agent_info = {};         // AI代理信息
    std::vector<CozeChatRobotInfo> robot_infos; // 机器人信息列表

    // ==================== 尝试从SD卡读取用户配置 ====================
    
    if (coze_agent_config_read(&config) == ESP_OK) {
        // 成功读取用户配置文件，解析配置信息
        ESP_UTILS_LOGI("Successfully loaded user configuration from SD card");
        
        // 提取AI代理认证信息
        agent_info.custom_consumer = config.custom_consumer ? config.custom_consumer : "";
        agent_info.app_id = config.appid ? config.appid : "";
        agent_info.public_key = config.public_key ? config.public_key : "";
        agent_info.private_key = config.private_key ? config.private_key : "";
        
        // 提取机器人配置信息
        for (int i = 0; i < config.bot_num; i++) {
            robot_infos.push_back(CozeChatRobotInfo{
                .name = config.bot[i].bot_name ? config.bot[i].bot_name : "",
                .bot_id = config.bot[i].bot_id ? config.bot[i].bot_id : "",
                .voice_id = config.bot[i].voice_id ? config.bot[i].voice_id : "",
                .description = config.bot[i].bot_description ? config.bot[i].bot_description : "",
            });
        }
        
        // 释放配置数据内存
        ESP_UTILS_CHECK_FALSE_RETURN(
            coze_agent_config_release(&config) == ESP_OK, false, 
            "Failed to release configuration memory"
        );
    } else {
        // ==================== 使用默认配置 ====================
        
#if COZE_AGENT_ENABLE_DEFAULT_CONFIG
        // 用户配置文件不存在或读取失败，使用编译时嵌入的默认配置
        ESP_UTILS_LOGW("User configuration not found, using embedded default configuration");
        
        // 使用预定义的默认AI代理信息
        agent_info.custom_consumer = COZE_AGENT_CUSTOM_CONSUMER;
        agent_info.app_id = COZE_AGENT_APP_ID;
        agent_info.public_key = COZE_AGENT_DEVICE_PUBLIC_KEY;
        // 私钥从嵌入的二进制数据中读取
        agent_info.private_key = std::string(private_key_pem_start, private_key_pem_end - private_key_pem_start);
        
        // 添加默认机器人配置(如果启用)
#if COZE_AGENT_BOT1_ENABLE
        robot_infos.push_back(CozeChatRobotInfo {
            .name = COZE_AGENT_BOT1_NAME,
            .bot_id = COZE_AGENT_BOT1_ID,
            .voice_id = COZE_AGENT_BOT1_VOICE_ID,
            .description = COZE_AGENT_BOT1_DESCRIPTION,
        });
        ESP_UTILS_LOGI("Added default bot 1: %s", COZE_AGENT_BOT1_NAME);
#endif // COZE_AGENT_BOT1_ENABLE

#if COZE_AGENT_BOT2_ENABLE
        robot_infos.push_back(CozeChatRobotInfo {
            .name = COZE_AGENT_BOT2_NAME,
            .bot_id = COZE_AGENT_BOT2_ID,
            .voice_id = COZE_AGENT_BOT2_VOICE_ID,
            .description = COZE_AGENT_BOT2_DESCRIPTION,
        });
        ESP_UTILS_LOGI("Added default bot 2: %s", COZE_AGENT_BOT2_NAME);
#endif // COZE_AGENT_BOT2_ENABLE

#else
        // 如果没有启用默认配置，且用户配置读取失败，则返回错误
        ESP_UTILS_CHECK_FALSE_RETURN(false, false, 
            "No configuration available: user config failed and default config disabled");
#endif // COZE_AGENT_ENABLE_DEFAULT_CONFIG
    }

    // ==================== 应用配置到AI代理实例 ====================
    
    // 将解析后的配置信息应用到AI代理实例
    ESP_UTILS_CHECK_FALSE_RETURN(
        Agent::requestInstance()->configCozeAgentConfig(agent_info, robot_infos), false, 
        "Failed to apply configuration to AI agent instance"
    );

    ESP_UTILS_LOGI("Coze agent configuration loaded successfully - %zu robot(s) configured", robot_infos.size());
    return true;
}

// ==================== 时钟更新功能(已注释) ====================
// 以下是时钟显示更新的回调函数，目前已被注释掉
// 如果需要在状态栏显示实时时钟，可以取消注释并启用相关代码

// static void on_clock_update_timer_cb(struct _lv_timer_t *t)

// {
//     time_t now;
//     struct tm timeinfo;
//     Speaker *speaker = (Speaker *)t->user_data;

//     time(&now);
//     localtime_r(&now, &timeinfo);

//     /* Since this callback is called from LVGL task, it is safe to operate LVGL */
//     // Update clock on "Status Bar"
//     ESP_UTILS_CHECK_FALSE_EXIT(
//         speaker->display.getQuickSettings().setClockTime(timeinfo.tm_hour, timeinfo.tm_min),
//         "Refresh status bar failed"
//     );

// }

static bool create_speaker_and_install_apps()
{
    ESP_UTILS_LOG_TRACE_GUARD();

    Speaker *speaker = nullptr;
    /* Create a speaker object */
    ESP_UTILS_CHECK_EXCEPTION_RETURN(
        speaker = new Speaker(), false, "Create speaker failed"
    );

    /* Try using a stylesheet that corresponds to the resolution */
    std::unique_ptr<SpeakerStylesheet_t> stylesheet;
    ESP_UTILS_CHECK_EXCEPTION_RETURN(
        stylesheet = std::make_unique<SpeakerStylesheet_t>(ESP_BROOKESIA_SPEAKER_360_360_DARK_STYLESHEET), false,
        "Create stylesheet failed"
    );
    ESP_UTILS_LOGI("Using stylesheet (%s)", stylesheet->core.name);
    ESP_UTILS_CHECK_FALSE_RETURN(speaker->addStylesheet(stylesheet.get()), false, "Add stylesheet failed");
    ESP_UTILS_CHECK_FALSE_RETURN(speaker->activateStylesheet(stylesheet.get()), false, "Activate stylesheet failed");
    stylesheet = nullptr;

    /* Configure and begin the speaker */
    speaker->registerLvLockCallback((LockCallback)(bsp_display_lock), 0);
    speaker->registerLvUnlockCallback((UnlockCallback)(bsp_display_unlock));
    ESP_UTILS_LOGI("Display ESP-Brookesia speaker demo");

    speaker->lockLv();

    ESP_UTILS_CHECK_FALSE_RETURN(speaker->begin(), false, "Begin failed");

    /* 5.Install app settings */
    auto app_settings = Settings::requestInstance();
    ESP_UTILS_CHECK_NULL_RETURN(app_settings, false, "Get app settings failed");
    // Add app settings stylesheet
    std::unique_ptr<SettingsStylesheetData> app_settings_stylesheet;
    ESP_UTILS_CHECK_EXCEPTION_RETURN(
        app_settings_stylesheet = std::make_unique<SettingsStylesheetData>(SETTINGS_UI_360_360_STYLESHEET_DARK()),
        false, "Create app settings stylesheet failed"
    );
    app_settings_stylesheet->screen_size = ESP_BROOKESIA_STYLE_SIZE_RECT_PERCENT(100, 100);
    app_settings_stylesheet->manager.wlan.scan_ap_count_max = 30;
    app_settings_stylesheet->manager.wlan.scan_interval_ms = 10000;
    app_settings_stylesheet->manager.about.device_board_name = "EchoEar";
    app_settings_stylesheet->manager.about.device_ram_main = "512KB";
    app_settings_stylesheet->manager.about.device_ram_minor = "16MB";
    ESP_UTILS_CHECK_FALSE_RETURN(
        app_settings->addStylesheet(speaker, app_settings_stylesheet.get()), false, "Add app settings stylesheet failed"
    );
    ESP_UTILS_CHECK_FALSE_RETURN(
        app_settings->activateStylesheet(app_settings_stylesheet.get()), false, "Activate app settings stylesheet failed"
    );
    app_settings_stylesheet = nullptr;
    // Process settings events
    app_settings->manager.event_signal.connect([](SettingsManager::EventType event_type, SettingsManager::EventData event_data) {
        ESP_UTILS_LOG_TRACE_GUARD();

        ESP_UTILS_LOGD("Param: event_type(%d), event_data(%s)", static_cast<int>(event_type), event_data.type().name());

        switch (event_type) {
        case SettingsManager::EventType::EnterDeveloperMode: {
            ESP_UTILS_CHECK_FALSE_RETURN(
                event_data.type() == typeid(SETTINGS_EVENT_DATA_TYPE_ENTER_DEVELOPER_MODE), false,
                "Invalid developer mode type"
            );

            ESP_UTILS_LOGW("Enter developer mode");
            developer_mode_key = DEVELOPER_MODE_KEY;
            esp_restart();
            break;
        }
        case SettingsManager::EventType::SetSoundVolume: {
            ESP_UTILS_CHECK_FALSE_RETURN(
                event_data.type() == typeid(SETTINGS_EVENT_DATA_TYPE_SET_SOUND_VOLUME), false, "Invalid volume type"
            );

            auto volume = std::any_cast<SETTINGS_EVENT_DATA_TYPE_SET_SOUND_VOLUME>(event_data);
            ESP_UTILS_CHECK_FALSE_RETURN(
                set_media_sound_volume(volume), false, "Set media sound volume failed"
            );
            break;
        }
        case SettingsManager::EventType::GetSoundVolume: {
            ESP_UTILS_CHECK_FALSE_RETURN(
                event_data.type() == typeid(SETTINGS_EVENT_DATA_TYPE_GET_SOUND_VOLUME), false, "Invalid volume type"
            );

            auto &volume = std::any_cast<SETTINGS_EVENT_DATA_TYPE_GET_SOUND_VOLUME>(event_data).get();
            volume = get_media_sound_volume();
            break;
        }
        case SettingsManager::EventType::SetDisplayBrightness: {
            ESP_UTILS_CHECK_FALSE_RETURN(
                event_data.type() == typeid(SETTINGS_EVENT_DATA_TYPE_SET_DISPLAY_BRIGHTNESS), false,
                "Invalid brightness type"
            );

            auto brightness = std::any_cast<SETTINGS_EVENT_DATA_TYPE_SET_DISPLAY_BRIGHTNESS>(event_data);
            ESP_UTILS_CHECK_FALSE_RETURN(
                set_media_display_brightness(brightness), false, "Set media display brightness failed"
            );
            break;
        }
        case SettingsManager::EventType::GetDisplayBrightness: {
            ESP_UTILS_CHECK_FALSE_RETURN(
                event_data.type() == typeid(SETTINGS_EVENT_DATA_TYPE_GET_DISPLAY_BRIGHTNESS), false,
                "Invalid brightness type"
            );

            auto &brightness = std::any_cast<SETTINGS_EVENT_DATA_TYPE_GET_DISPLAY_BRIGHTNESS>(event_data).get();
            brightness = get_media_display_brightness();
            break;
        }
        default:
            return false;
        }

        return true;
    });
    auto app_settings_id = speaker->installApp(app_settings);
    ESP_UTILS_CHECK_FALSE_RETURN(speaker->checkAppID_Valid(app_settings_id), false, "Install app settings failed");

    /* 6.Install app ai profile */
    auto app_ai_profile = AI_Profile::requestInstance();
    ESP_UTILS_CHECK_NULL_RETURN(app_ai_profile, false, "Get app ai profile failed");
    auto app_ai_profile_id = speaker->installApp(app_ai_profile);
    ESP_UTILS_CHECK_FALSE_RETURN(speaker->checkAppID_Valid(app_ai_profile_id), false, "Install app ai profile failed");

    /* Install 2048 game app */
    Game2048 *app_game_2048 = nullptr;
    ESP_UTILS_CHECK_EXCEPTION_RETURN(app_game_2048 = new Game2048(240, 360), false, "Create 2048 game app failed");
    auto app_game_2048_id = speaker->installApp(app_game_2048);
    ESP_UTILS_CHECK_FALSE_RETURN(speaker->checkAppID_Valid(app_game_2048_id), false, "Install 2048 game app failed");

    /* Install calculator app */
    Calculator *app_calculator = nullptr;
    ESP_UTILS_CHECK_EXCEPTION_RETURN(app_calculator = new Calculator(), false, "Create calculator app failed");
    auto app_calculator_id = speaker->installApp(app_calculator);
    ESP_UTILS_CHECK_FALSE_RETURN(speaker->checkAppID_Valid(app_calculator_id), false, "Install calculator app failed");

    /* Install timer app */
    auto app_timer = Timer::requestInstance();
    ESP_UTILS_CHECK_NULL_RETURN(app_timer, false, "Get timer app failed");
    auto app_timer_id = speaker->installApp(app_timer);
    ESP_UTILS_CHECK_FALSE_RETURN(speaker->checkAppID_Valid(app_timer_id), false, "Install timer app failed");
    /* 7. 解锁 LVGL */
    speaker->unlockLv();

    /* Register function callings */
    FunctionDefinition openApp("open_app", "Open a specific app.打开一个应用");
    //添加参数定义
    openApp.addParameter("app_name", "The name of the app to open.应用名称", FunctionParameter::ValueType::String);
    openApp.setCallback([ = ](const std::vector<FunctionParameter> &params) {
        ESP_UTILS_LOG_TRACE_GUARD();

        static std::map<int, std::vector<std::string>> app_name_map = {
            {app_settings_id, {app_settings->getName(), "setting", "settings", "设置", "设置应用", "设置app"}},
            {app_game_2048_id, {app_game_2048->getName(), "2048", "game", "游戏", "2048游戏", "2048app"}},
            {app_calculator_id, {app_calculator->getName(), "calculator", "calc", "计算器", "计算器应用", "计算器app"}},
            {app_ai_profile_id, {app_ai_profile->getName(), "AI profile", "ai 配置", "ai配置", "ai设置", "ai设置应用", "ai设置app"}},
            {app_timer_id, {app_timer->getName(), "timer", "时钟", "时钟应用", "时钟app"}}
        };
        //数据解析，看看有没有功能能用上
        //目前支持打开应用，调节音量控制和亮度控制
        for (const auto &param : params) {
            if (param.name() == "app_name") {
                auto app_name = param.string();
                ESP_UTILS_LOGI("Opening app: %s", app_name.c_str());

                ESP_Brookesia_CoreAppEventData_t event_data = {
                    .id = -1,
                    .type = ESP_BROOKESIA_CORE_APP_EVENT_TYPE_START,
                    .data = nullptr
                };
                auto target_name = to_lower(get_before_space(app_name));
                for (const auto &pair : app_name_map) {
                    if (std::find(pair.second.begin(), pair.second.end(), target_name) != pair.second.end()) {
                        event_data.id = pair.first;
                        break;
                    }
                }

                if (event_data.id == -1) {
                    ESP_UTILS_LOGW("App name not found");
                    return;
                }

                boost::this_thread::sleep_for(boost::chrono::milliseconds(FUNCTION_OPEN_APP_WAIT_SPEAKING_PRE_MS));

                int wait_count = 0;
                int wait_interval_ms = FUNCTION_OPEN_APP_WAIT_SPEAKING_INTERVAL_MS;
                int wait_max_count = FUNCTION_OPEN_APP_WAIT_SPEAKING_MAX_MS / wait_interval_ms;
                while ((wait_count < wait_max_count) && AI_Buddy::requestInstance()->isSpeaking()) {
                    boost::this_thread::sleep_for(boost::chrono::milliseconds(wait_interval_ms));
                    wait_count++;
                }

                speaker->lockLv();
                speaker->manager.processDisplayScreenChange(
                    ESP_BROOKESIA_SPEAKER_MANAGER_SCREEN_MAIN, nullptr
                );
                speaker->sendAppEvent(&event_data);
                speaker->unlockLv();
            }
        }
    }, std::make_optional<FunctionDefinition::CallbackThreadConfig>({
        .name = FUNCTION_OPEN_APP_THREAD_NAME,
        .stack_size = FUNCTION_OPEN_APP_THREAD_STACK_SIZE,
        .stack_in_ext = FUNCTION_OPEN_APP_THREAD_STACK_CAPS_EXT,
    }));
    FunctionDefinitionList::requestInstance().addFunction(openApp);

    /* 注册AI函数调用 - 音量控制功能 */
    // 这个函数允许AI助手通过语音指令控制设备音量
    FunctionDefinition setVolume("set_volume", "Adjust the system volume. Range is from 0 to 100.");
    setVolume.addParameter("level", "The desired volume level (0 to 100).", FunctionParameter::ValueType::String);
    setVolume.setCallback([ = ](const std::vector<FunctionParameter> &params) {
        ESP_UTILS_LOG_TRACE_GUARD();

        // 获取AI伙伴实例，用于控制机器人表情
        auto ai_buddy = AI_Buddy::requestInstance();
        ESP_UTILS_CHECK_NULL_EXIT(ai_buddy, "Failed to get ai buddy instance");

        for (const auto &param : params) {
            if (param.name() == "level") {
                int last_volume = get_media_sound_volume();
                int volume = atoi(param.string().c_str());

                // 根据音量变化显示对应的表情图标
                if (volume < 0) {
                    // 降低音量
                    volume = last_volume - FUNCTION_VOLUME_CHANGE_STEP;
                    if (volume <= 0) {
                        // 音量为0时显示静音图标
                        ESP_UTILS_CHECK_FALSE_EXIT(
                            ai_buddy->expression.setSystemIcon("volume_mute"),
                            "Failed to set volume mute icon"
                        );
                    } else {
                        // 显示音量降低图标
                        ESP_UTILS_CHECK_FALSE_EXIT(
                            ai_buddy->expression.setSystemIcon("volume_down"), "Failed to set volume down icon"
                        );
                    }
                } else if (volume > 100) {
                    // 提高音量，显示音量提高图标
                    volume = last_volume + FUNCTION_VOLUME_CHANGE_STEP;
                    ESP_UTILS_CHECK_FALSE_EXIT(
                        ai_buddy->expression.setSystemIcon("volume_up"), "Failed to set volume up icon"
                    );
                }
                ESP_UTILS_CHECK_FALSE_EXIT(set_media_sound_volume(volume), "Failed to set volume");
            }
        }
    }, std::make_optional<FunctionDefinition::CallbackThreadConfig>(FunctionDefinition::CallbackThreadConfig{
        .name = FUNCTION_VOLUME_CHANGE_THREAD_NAME,
        .stack_size = FUNCTION_VOLUME_CHANGE_THREAD_STACK_SIZE,
        .stack_in_ext = FUNCTION_VOLUME_CHANGE_THREAD_STACK_CAPS_EXT,
    }));
    FunctionDefinitionList::requestInstance().addFunction(setVolume);

    /* 注册AI函数调用 - 亮度控制功能 */
    // 这个函数允许AI助手通过语音指令控制屏幕亮度
    FunctionDefinition setBrightness("set_brightness", "Adjust the system brightness. Range is from 10 to 100.");
    setBrightness.addParameter("level", "The desired brightness level (10 to 100).", FunctionParameter::ValueType::String);
    setBrightness.setCallback([ = ](const std::vector<FunctionParameter> &params) {
        ESP_UTILS_LOG_TRACE_GUARD();

        // 获取AI伙伴实例，用于控制机器人表情
        auto ai_buddy = AI_Buddy::requestInstance();
        ESP_UTILS_CHECK_NULL_EXIT(ai_buddy, "Failed to get ai buddy instance");

        for (const auto &param : params) {
            if (param.name() == "level") {
                int last_brightness = get_media_display_brightness();
                int brightness = atoi(param.string().c_str());

                // 根据亮度变化显示对应的表情图标
                if (brightness < 0) {
                    // 降低亮度，显示亮度降低图标
                    brightness = last_brightness - FUNCTION_BRIGHTNESS_CHANGE_STEP;
                    ESP_UTILS_CHECK_FALSE_EXIT(
                        ai_buddy->expression.setSystemIcon("brightness_down"), "Failed to set brightness down icon"
                    );
                } else if (brightness > 100) {
                    // 提高亮度，显示亮度提高图标
                    brightness = last_brightness + FUNCTION_BRIGHTNESS_CHANGE_STEP;
                    ESP_UTILS_CHECK_FALSE_EXIT(
                        ai_buddy->expression.setSystemIcon("brightness_up"), "Failed to set brightness up icon"
                    );
                }
                ESP_UTILS_CHECK_FALSE_EXIT(set_media_display_brightness(brightness), "Failed to set brightness");
            }
        }
    }, std::make_optional<FunctionDefinition::CallbackThreadConfig>(FunctionDefinition::CallbackThreadConfig{
        .name = FUNCTION_BRIGHTNESS_CHANGE_THREAD_NAME,
        .stack_size = FUNCTION_BRIGHTNESS_CHANGE_THREAD_STACK_SIZE,
        .stack_in_ext = FUNCTION_BRIGHTNESS_CHANGE_THREAD_STACK_CAPS_EXT,
    }));
    FunctionDefinitionList::requestInstance().addFunction(setBrightness);

    // /* Connect the quick settings event signal */
    // speaker->getDisplay().getQuickSettings().on_event_signal.connect([=](QuickSettings::EventType event_type) {
    //     if ((event_type != QuickSettings::EventType::SettingsButtonClicked) &&
    //         (speaker->getCoreManager().getRunningAppById(app_settings_id) != nullptr)) {
    //         return true;
    //     }

    //     ESP_Brookesia_CoreAppEventData_t event_data = {
    //         .id = app_settings_id,
    //         .type = ESP_BROOKESIA_CORE_APP_EVENT_TYPE_START,
    //         .data = nullptr
    //     };
    //     speaker->sendAppEvent(&event_data);

    //     return true;
    // });

    /* Create a timer to update the clock */
    // ESP_UTILS_CHECK_NULL_EXIT(lv_timer_create(on_clock_update_timer_cb, 1000, speaker), "Create clock update timer failed");

    return true;
}
