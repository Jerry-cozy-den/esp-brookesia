/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <memory>
#include "boost/signals2/signal.hpp"
#include "systems/core/esp_brookesia_core.hpp"
#include "widgets/app_launcher/esp_brookesia_app_launcher.hpp"
#include "widgets/quick_settings/esp_brookesia_speaker_quick_settings.hpp"
#include "widgets/keyboard/esp_brookesia_keyboard.hpp"
#include "anim_player/esp_brookesia_anim_player.hpp"

// *INDENT-OFF*

namespace esp_brookesia::speaker {

// DisplayData: 定义显示模块所需的配置数据，包括启动动画、应用启动器、快速设置和键盘数据，以及布局标志位。
struct DisplayData {
    struct {
        gui::AnimPlayerData data;
    } boot_animation;
    struct {
        AppLauncherData data;
        ESP_Brookesia_StyleImage_t default_image;
    } app_launcher;
    struct {
        QuickSettingsData data;
    } quick_settings;
    struct {
        KeyboardData data;
    } keyboard;
    struct {
        uint8_t enable_app_launcher_flex_size: 1;
    } flags;
};

// Display: 扬声器系统的显示管理模块类
// 继承自 ESP_Brookesia_CoreDisplay，负责：
// - 初始化并管理 LVGL 显示设备
// - 处理应用启动器图标显示与布局
// - 管理快速设置面板和虚拟键盘
// - 控制启动动画和虚拟绘制遮罩
// 用法示例：
//   Speaker speaker; 
//   speaker.begin(); // 内部会调用 Display::begin 初始化显示模块
//   auto disp = speaker.getDisplay().getDummyDrawMask(); // 获取虚拟绘制遮罩对象
//   speaker.getDisplay().startBootAnimation(); // 播放启动动画
class Display: public ESP_Brookesia_CoreDisplay {
public:
    friend class Speaker;
    friend class Manager;

    // OnDummyDrawSignal: 定义虚拟绘制信号类型
    // 当需要启用或禁用虚拟绘制遮罩时，该信号会通知所有连接槽函数
    using OnDummyDrawSignal = boost::signals2::signal<void(bool)>;

    // 构造函数：传入核心对象和显示数据初始化显示模块
    Display(ESP_Brookesia_Core &core, const DisplayData &data);
    // 析构函数：清理显示模块资源
    ~Display();

    // 禁用复制和移动构造及赋值，确保唯一性
    Display(const Display&) = delete;
    Display(Display&&) = delete;
    Display& operator=(const Display&) = delete;
    Display& operator=(Display&&) = delete;

    // 检查显示模块是否已初始化
    bool checkInitialized(void) const               { return  _app_launcher.checkInitialized(); }
    // 获取显示配置数据引用
    const DisplayData &getData(void) const        { return _data; }
    // 获取应用启动器对象，用于添加或移除应用图标
    AppLauncher *getAppLauncher(void)               { return &_app_launcher; }
    // 获取快速设置面板对象，用于控制快速设置界面
    QuickSettings &getQuickSettings(void)           { return _quick_settings; }
    // 获取虚拟键盘对象，用于弹出或隐藏键盘
    Keyboard &getKeyboard(void)                     { return _keyboard; }
    // 获取虚拟绘制遮罩对象，用于启用/禁用界面遮罩绘制
    gui::LvContainer *getDummyDrawMask(void)        { return _dummy_draw_mask.get(); }

    // 播放启动动画，需在显示初始化后调用
    bool startBootAnimation(void);

    // 根据屏幕尺寸校准各子组件的显示数据和布局
    bool calibrateData(const ESP_Brookesia_StyleSize_t &screen_size, DisplayData &data);

    // 虚拟绘制信号，可用于订阅遮罩显示状态变化
    static OnDummyDrawSignal on_dummy_draw_signal;

private:
    // begin/del 由 Speaker 内部调用，不建议外部直接使用
    bool begin(void);
    bool del(void);
    // 应用管理回调，内部使用
    bool processAppInstall(ESP_Brookesia_CoreApp *app) override;
    bool processAppUninstall(ESP_Brookesia_CoreApp *app) override;
    bool processAppRun(ESP_Brookesia_CoreApp *app) override;
    bool processAppResume(ESP_Brookesia_CoreApp *app) override;
    bool processAppClose(ESP_Brookesia_CoreApp *app) override;
    bool processMainScreenLoad(void) override;
    bool getAppVisualArea(ESP_Brookesia_CoreApp *app, lv_area_t &app_visual_area) const override;

    // 处理虚拟绘制开关事件
    bool processDummyDraw(bool is_visible);

    // Core: 引用显示配置数据
    const DisplayData &_data;
    // Widgets: 子组件实例
    AppLauncher _app_launcher;
    QuickSettings _quick_settings;
    Keyboard _keyboard;
    gui::LvContainerUniquePtr _dummy_draw_mask;
};
// *INDENT-ON*

} // namespace esp_brookesia::speaker
