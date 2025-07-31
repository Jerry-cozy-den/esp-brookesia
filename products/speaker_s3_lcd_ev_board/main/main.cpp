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
// ==================== FreeRTOS 实时操作系统相关头文件 ====================
#include "freertos/FreeRTOS.h"         // FreeRTOS实时操作系统
#include "freertos/task.h"             // FreeRTOS任务管理
#include "esp_check.h"                 // ESP错误检查宏
#include "esp_log.h"                   // ESP日志系统
#include "esp_event.h"            // ESP事件循环：系统事件分发和处理机制
#include "esp_spiffs.h"           // SPIFFS文件系统：轻量级文件系统，用于存储配置文件

// ==================== 图形界面和音频相关头文件 ====================
#include "esp_lvgl_port.h"             // LVGL图形库移植层
#include "bsp/esp-bsp.h"               // 板级支持包
#include "bsp/display.h"               // 显示屏驱动
#include "bsp/touch.h"                 // 触摸屏驱动
#include "esp_brookesia.hpp"           // ESP-Brookesia手机框架

#include "esp_brookesia_app_squareline_demo.hpp"  // SquareLine演示应用
#include "esp_brookesia_app_game_2048_phone.hpp"  // 2048游戏应用 (Phone版本)
using namespace esp_brookesia::apps;   // 使用esp_brookesia应用命名空间

#define EXAMPLE_SHOW_MEM_INFO             (1)    // 是否显示内存信息的开关

// ==================== 静态函数声明 ====================
// 这些函数按照系统初始化的顺序排列，每个函数负责初始化特定的子系统
static bool init_display_and_draw_logic();     // 初始化显示系统和动画绘图逻辑

// ==================== 全局变量定义 ====================
// 音频设备句柄 - 用于音频播放和录音功能
static esp_codec_dev_handle_t play_dev = nullptr;  // 音频播放设备句柄(扬声器)
static esp_codec_dev_handle_t rec_dev = nullptr;   // 音频录音设备句柄(麦克风)




static const char *TAG = "app_main";  // 日志标签

// 时钟更新定时器回调函数声明
static void on_clock_update_timer_cb(struct _lv_timer_t *t);

// 应用程序主入口函数
extern "C" void app_main(void)
{
    // ==================== 系统初始化阶段 ====================
    // 按照依赖关系的顺序初始化各个子系统，使用assert确保每步都成功
    
    // 第1步：初始化显示系统 - LCD屏幕、LVGL图形库、动画引擎
    assert(init_display_and_draw_logic()        && "Initialize display and draw logic failed");


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
// LVGL移植层初始化配置宏
#define LVGL_PORT_INIT_CONFIG() \
    {                               \

    }
static bool init_display_and_draw_logic()
{
    // 配置显示屏参数
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = {
            .task_priority = 4,       /* LVGL任务优先级 */      \
            .task_stack = 10 * 1024,  /* LVGL任务栈大小(10KB) */ \
            .task_affinity = -1,      /* CPU亲和性(-1表示任意CPU) */ \
            .task_max_sleep_ms = 500, /* 任务最大休眠时间(ms) */ \
            .timer_period_ms = 5,     /* 定时器周期(ms) */ \
        },
    };
    // 启动显示屏并获取显示对象
    lv_display_t *disp = bsp_display_start_with_config(&cfg);

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