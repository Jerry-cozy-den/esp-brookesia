/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "systems/phone/esp_brookesia_phone_app.hpp"

namespace esp_brookesia::apps {

/**
 * @brief 一个使用 Squareline Studio 导出 UI 的手机应用模板。用户可以修改此模板来设计自己的应用。
 *
 */
class SquarelineDemo: public ESP_Brookesia_PhoneApp {
public:
    /**
     * @brief 获取 SquarelineDemo 的单例实例
     *
     * @param use_status_bar 是否显示状态栏的标志
     * @param use_navigation_bar 是否显示导航栏的标志
     * @return 指向单例实例的指针
     */
    static SquarelineDemo *requestInstance(bool use_status_bar = false, bool use_navigation_bar = false);

    /**
     * @brief 手机应用的析构函数
     *
     */
    ~SquarelineDemo();

    using ESP_Brookesia_PhoneApp::startRecordResource;
    using ESP_Brookesia_PhoneApp::endRecordResource;

protected:
    /**
     * @brief 私有构造函数，用于强制实现单例模式
     *
     * @param use_status_bar 是否显示状态栏的标志
     * @param use_navigation_bar 是否显示导航栏的标志
     */
    SquarelineDemo(bool use_status_bar, bool use_navigation_bar);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// 以下函数必须由用户的应用类实现。 /////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief 当应用开始运行时调用。这是应用的入口点，应在此处创建所有 UI 资源。
     *
     * @note 如果在 `ESP_Brookesia_CoreAppData_t` 中设置了 `enable_default_screen` 标志，当应用启动时，
     *       核心将创建一个默认屏幕，该屏幕将自动加载和清理。然后应用应在此函数中使用 `lv_scr_act()` 
     *       在其上创建所有 UI 资源。否则，应用需要创建一个新屏幕并在此函数中手动加载它
     * @note 如果在 `ESP_Brookesia_CoreAppData_t` 中设置了 `enable_recycle_resource` 标志，当应用关闭时，
     *       核心将自动清理所有记录的资源，包括屏幕 (`lv_obj_create(NULL)`)、
     *       动画 (`lv_anim_start()`) 和定时器 (`lv_timer_create()`)。在此函数中创建的资源将被记录。
     *       否则，应用需要调用 `cleanRecordResource()` 函数来手动清理
     * @note 如果在 `ESP_Brookesia_CoreAppData_t` 中设置了 `enable_resize_visual_area` 标志，核心将调整
     *       所有记录屏幕的可视区域。在此函数中创建的屏幕将被记录。这在屏幕显示浮动 UI（如状态栏）时很有用。
     *       否则，应用的屏幕将以全屏显示，但某些区域可能不可见。应用可以调用 `getVisualArea()` 
     *       函数来获取最终的可视区域
     *
     * @return 成功返回 true，否则返回 false
     *
     */
    bool run(void) override;

    /**
     * @brief 当应用接收到返回事件时调用。要退出，应用可以调用 `notifyCoreClosed()` 来通知核心关闭应用。
     *
     * @return 成功返回 true，否则返回 false
     *
     */
    bool back(void) override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// 以下函数可以由用户的应用类重新定义。 //////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief 当应用开始关闭时调用。应用可以在此处执行必要的操作。
     *
     * @note  应用不应在此函数中调用 `notifyCoreClosed()` 函数。
     *
     * @return 成功返回 true，否则返回 false
     *
     */
    // bool close(void) override;

    /**
     * @brief 当应用开始安装时调用。应用可以在此处执行初始化。
     *
     * @return 成功返回 true，否则返回 false
     *
     */
    // bool init(void) override;

    /**
     * @brief 当应用开始卸载时调用。应用可以在此处执行反初始化。
     *
     * @return 成功返回 true，否则返回 false
     *
     */
    // bool deinit(void) override;

    /**
     * @brief 当应用被暂停时调用。应用可以在此处执行必要的操作。
     *
     * @return 成功返回 true，否则返回 false
     *
     */
    // bool pause(void) override;

    /**
     * @brief 当应用恢复时调用。应用可以在此处执行必要的操作。
     *
     * @note 如果在 `ESP_Brookesia_CoreAppData_t` 中设置了 `enable_recycle_resource` 标志，当应用关闭时，
     *       核心将自动清理所有记录的资源，包括屏幕 (`lv_obj_create(NULL)`)、
     *       动画 (`lv_anim_start()`) 和定时器 (`lv_timer_create()`)。在此函数中创建的资源将被记录。
     *       否则，应用需要调用 `cleanRecordResource()` 函数来手动清理
     * @note 如果在 `ESP_Brookesia_CoreAppData_t` 中设置了 `enable_resize_visual_area` 标志，核心将调整
     *       所有记录屏幕的可视区域。在此函数中创建的屏幕将被记录。这在屏幕显示浮动 UI（如状态栏）时很有用。
     *       否则，应用的屏幕将以全屏显示，但某些区域可能不可见。应用可以调用 `getVisualArea()` 
     *       函数来获取最终的可视区域
     *
     * @return 成功返回 true，否则返回 false
     *
     */
    // bool resume(void) override;

    /**
     * @brief 当应用开始关闭时调用。应用可以在此处执行额外的资源清理。
     *
     * @note 如果有未被核心记录的资源（不是在 `run()` 和 `pause()` 函数中创建的，
     *       或者不在 `startRecordResource()` 和 `stopRecordResource()` 函数之间），
     *       应用应该调用此函数来手动清理这些资源。此函数不与 `cleanRecordResource()` 函数冲突。
     *
     * @return 成功返回 true，否则返回 false
     *
     */
    // bool cleanResource(void) override;

private:
    static SquarelineDemo *_instance; // 单例实例
};

} // namespace esp_brookesia::apps
