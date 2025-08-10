/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// esp_brookesia_phone.hpp
// 本文件定义了 Phone 类及其样式表管理，负责手机系统的初始化、样式切换、显示和应用管理功能
#pragma once

#include <list>
#include <memory>
#include "esp_brookesia_systems_internal.h"
#include "systems/core/esp_brookesia_core_stylesheet_manager.hpp"
#include "esp_brookesia_phone_home.hpp"
#include "esp_brookesia_phone_manager.hpp"
#include "esp_brookesia_phone_app.hpp"

// *INDENT-OFF*


// 手机系统的样式表数据结构，包括核心、主页面和管理器的数据
typedef struct {
    ESP_Brookesia_CoreData_t core;                // 核心数据
    ESP_Brookesia_PhoneHomeData_t home;           // 主页面数据
    ESP_Brookesia_PhoneManagerData_t manager;     // 管理器数据
} ESP_Brookesia_PhoneStylesheet_t;


// 手机系统样式表管理器类型别名
using ESP_Brookesia_PhoneStylesheetManager = ESP_Brookesia_CoreStylesheetManager<ESP_Brookesia_PhoneStylesheet_t>;


// ESP_Brookesia_Phone 类：手机系统的核心入口，负责初始化、配置样式表和管理各个子模块
class ESP_Brookesia_Phone: public ESP_Brookesia_Core, public ESP_Brookesia_PhoneStylesheetManager {
public:
    // 构造与析构
    ESP_Brookesia_Phone(lv_display_t *display = nullptr); // 构造函数，可指定显示设备
    ~ESP_Brookesia_Phone();                               // 析构函数

    // 安装应用到手机系统
    int installApp(ESP_Brookesia_PhoneApp &app)    { return _core_manager.installApp(app); }
    int installApp(ESP_Brookesia_PhoneApp *app)    { return _core_manager.installApp(app); }
    // 卸载应用
    int uninstallApp(ESP_Brookesia_PhoneApp &app)  { return _core_manager.uninstallApp(app); }
    int uninstallApp(ESP_Brookesia_PhoneApp *app)  { return _core_manager.uninstallApp(app); }
    bool uninstallApp(int id)               { return _core_manager.uninstallApp(id); }

    // 系统启动与销毁
    bool begin(void);    // 启动手机系统
    bool del(void);      // 销毁手机系统

    // 样式表相关操作
    bool addStylesheet(const ESP_Brookesia_PhoneStylesheet_t &stylesheet);      // 添加样式表（引用）
    bool addStylesheet(const ESP_Brookesia_PhoneStylesheet_t *stylesheet);      // 添加样式表（指针）
    bool activateStylesheet(const ESP_Brookesia_PhoneStylesheet_t &stylesheet); // 激活样式表（引用）
    bool activateStylesheet(const ESP_Brookesia_PhoneStylesheet_t *stylesheet); // 激活样式表（指针）

    // 校准屏幕尺寸
    bool calibrateScreenSize(ESP_Brookesia_StyleSize_t &size) override;

    // 获取主页面和管理器对象
    ESP_Brookesia_PhoneHome &getHome(void)         { return _home; }      // 获取主页面对象
    ESP_Brookesia_PhoneManager &getManager(void)   { return _manager; }   // 获取管理器对象

private:
    // 校准样式表（内部实现）
    bool calibrateStylesheet(const ESP_Brookesia_StyleSize_t &screen_size, ESP_Brookesia_PhoneStylesheet_t &sheetstyle) override;

    ESP_Brookesia_PhoneHome _home;         // 主页面对象
    ESP_Brookesia_PhoneManager _manager;   // 管理器对象

    static const ESP_Brookesia_PhoneStylesheet_t _default_stylesheet_dark; // 默认深色样式表
};

// *INDENT-ON*
