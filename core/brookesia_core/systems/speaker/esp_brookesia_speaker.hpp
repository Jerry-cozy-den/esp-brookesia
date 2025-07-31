/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
// esp_brookesia_speaker.hpp
// 本文件定义了 Speaker 类及其样式表管理，负责扬声器系统的初始化、样式切换、显示和应用管理功能

#pragma once

#include <list>
#include "boost/thread.hpp"
#include "systems/core/esp_brookesia_core_stylesheet_manager.hpp"
#include "esp_brookesia_speaker_ai_buddy.hpp"
#include "esp_brookesia_speaker_display.hpp"
#include "esp_brookesia_speaker_manager.hpp"
#include "esp_brookesia_speaker_app.hpp"

namespace esp_brookesia::speaker {

typedef struct {
    ESP_Brookesia_CoreData_t core;
    DisplayData display;
    ManagerData manager;
    AI_BuddyData ai_buddy;
} SpeakerStylesheet_t;
// 定义扬声器系统的样式表数据结构，包括核心、显示、管理器和 AI 伴侣的数据

using SpeakerStylesheet = ESP_Brookesia_CoreStylesheetManager<SpeakerStylesheet_t>;

// Speaker 类：扬声器系统的核心入口，负责初始化、配置样式表和管理各个子模块
class Speaker: public ESP_Brookesia_Core, public SpeakerStylesheet {
public:
    Speaker(lv_disp_t *display_device = nullptr);
    ~Speaker();

    Speaker(const Speaker&) = delete;
    Speaker(Speaker&&) = delete;
    Speaker& operator=(const Speaker&) = delete;
    Speaker& operator=(Speaker&&) = delete;

    // 安装应用到扬声器系统
    int installApp(App &app)    { return _core_manager.installApp(app); }
    int installApp(App *app)    { return _core_manager.installApp(app); }
    // 卸载应用
    int uninstallApp(App &app)  { return _core_manager.uninstallApp(app); }
    int uninstallApp(App *app)  { return _core_manager.uninstallApp(app); }
    bool uninstallApp(int id)               { return _core_manager.uninstallApp(id); }

    bool begin(void);
    bool del(void);
    bool addStylesheet(const SpeakerStylesheet_t &stylesheet);
    bool addStylesheet(const SpeakerStylesheet_t *stylesheet);
    bool activateStylesheet(const SpeakerStylesheet_t &stylesheet);
    bool activateStylesheet(const SpeakerStylesheet_t *stylesheet);

    bool calibrateScreenSize(ESP_Brookesia_StyleSize_t &size) override;

    Display display;
    Manager manager;

private:
    bool calibrateStylesheet(const ESP_Brookesia_StyleSize_t &screen_size, SpeakerStylesheet_t &sheetstyle) override;

    static const SpeakerStylesheet_t _default_stylesheet_dark;
};
// *INDENT-OFF*

} // namespace esp_brookesia::speaker
