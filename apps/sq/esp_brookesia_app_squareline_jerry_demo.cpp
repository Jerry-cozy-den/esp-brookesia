/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "lvgl.h"
#include "esp_brookesia.hpp"
#ifdef ESP_UTILS_LOG_TAG
#   undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:Squareline" // 日志标签，用于区分不同模块的日志输出
#include "esp_lib_utils.h"
#include "ui/ui.h"
#include "esp_brookesia_app_squareline_jerry_demo.hpp"

using namespace std;
using namespace esp_brookesia::gui;

LV_IMG_DECLARE(esp_brookesia_app_icon_launcher_squareline_jerry_112_112);


// SquarelineDemo 应用命名空间，所有相关类和函数都在此命名空间下
namespace esp_brookesia::apps {


// 静态成员变量，单例模式下用于保存唯一实例指针
SquarelineDemo *SquarelineDemo::_instance = nullptr;


// 获取 SquarelineDemo 单例实例，若不存在则创建。可指定是否启用状态栏和导航栏。
SquarelineDemo *SquarelineDemo::requestInstance(bool use_status_bar, bool use_navigation_bar)
{
    if (_instance == nullptr) {
        // 首次调用时创建唯一实例
        _instance = new SquarelineDemo(use_status_bar, use_navigation_bar);
    }
    return _instance;
}


// 构造函数，初始化父类并设置应用名称、图标、状态栏和导航栏参数
SquarelineDemo::SquarelineDemo(bool use_status_bar, bool use_navigation_bar):
    ESP_Brookesia_PhoneApp("Squareline", &esp_brookesia_app_icon_launcher_squareline_jerry_112_112, false, use_status_bar, use_navigation_bar)
{
    // 这里可以做一些成员变量初始化或资源分配
}


// 析构函数，释放资源
SquarelineDemo::~SquarelineDemo()
{
    // 如有需要可在此处释放自定义资源
}


// 应用主入口，运行时会被调用
// 主要作用：初始化并创建所有 UI 资源，显示主界面
bool SquarelineDemo::run(void)
{
    ESP_UTILS_LOGD("Run"); // 输出调试日志，标记进入 run 流程

    // 创建所有 UI 资源（由 Squareline Studio 生成的 UI 初始化函数）
    phone_app_squareline_jerry_ui_init();

    return true;
}

    
// 返回/退出应用时调用，通知核心关闭本应用
bool SquarelineDemo::back(void)
{
    ESP_UTILS_LOGD("Back"); // 输出调试日志，标记进入 back 流程

    // 如果需要退出应用，调用 notifyCoreClosed() 通知核心关闭本应用
    ESP_UTILS_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

    return true;
}


// 下面这些函数为可选的生命周期钩子函数，按需解开注释并实现自定义逻辑：
// 1. close()：关闭应用时调用
// 2. init()：初始化应用时调用
// 3. deinit()：反初始化/卸载应用时调用
// 4. pause()：应用暂停时调用
// 5. resume()：应用恢复时调用
// 6. cleanResource()：清理资源时调用
// 这些函数可用于资源管理、状态保存、动画暂停等场景
// bool SquarelineDemo::close(void)
// {
//     ESP_UTILS_LOGD("Close");
//     /* 如有需要可在此处添加关闭前的操作 */
//     return true;
// }

// bool SquarelineDemo::init()
// {
//     ESP_UTILS_LOGD("Init");
//     /* 如有需要可在此处添加初始化操作 */
//     return true;
// }

// bool SquarelineDemo::deinit()
// {
//     ESP_UTILS_LOGD("Deinit");
//     /* 如有需要可在此处添加反初始化操作 */
//     return true;
// }

// bool SquarelineDemo::pause()
// {
//     ESP_UTILS_LOGD("Pause");
//     /* 如有需要可在此处添加暂停操作，比如暂停动画、保存状态等 */
//     return true;
// }

// bool SquarelineDemo::resume()
// {
//     ESP_UTILS_LOGD("Resume");
//     /* 如有需要可在此处添加恢复操作，比如恢复动画、还原状态等 */
//     return true;
// }

// bool SquarelineDemo::cleanResource()
// {
//     ESP_UTILS_LOGD("Clean resource");
//     /* 如有需要可在此处添加资源清理操作 */
//     return true;
// }

extern "C" {

    /**
     * 以下函数由 Squareline Studio 工具自动生成。
     * 
     * 这些函数的主要作用是在创建动画前后，对动画相关的资源进行记录。
     * 这样做的好处是：
     * 1. 在应用退出时，可以自动清理所有动画相关的资源，防止内存泄漏。
     * 2. 避免动画在执行过程中访问已经被销毁或清理的 UI 元素，从而防止程序崩溃或异常。
     * 3. 通过 startRecordResource() 和 endRecordResource() 这两个函数，能够精确地标记动画资源的生命周期，确保动画资源管理的安全性和自动化。
     * 4. 这些机制对于嵌入式 GUI 应用尤为重要，因为动画和 UI 资源频繁动态创建和销毁，自动管理可以极大提升系统稳定性。
     * 
     * 总结：
     * 通过自动记录和清理动画资源，提升了 UI 动画的健壮性和系统的可靠性。
     */
    void upanim_Animation(lv_obj_t * TargetObject, int delay)
    {
        ui_anim_user_data_t * PropertyAnimation_0_user_data = lv_malloc(sizeof(ui_anim_user_data_t));
        PropertyAnimation_0_user_data->target = TargetObject;
        PropertyAnimation_0_user_data->val = -1;
        lv_anim_t PropertyAnimation_0;
        lv_anim_init(&PropertyAnimation_0);
        lv_anim_set_time(&PropertyAnimation_0, 200);
        lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
        lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_y);
        lv_anim_set_values(&PropertyAnimation_0, -30, 0);
        lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_ease_out);
        lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
        lv_anim_set_deleted_cb(&PropertyAnimation_0, _ui_anim_callback_free_user_data);
        lv_anim_set_playback_time(&PropertyAnimation_0, 0);
        lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
        lv_anim_set_repeat_count(&PropertyAnimation_0, 0);
        lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
        lv_anim_set_early_apply(&PropertyAnimation_0, false);
        lv_anim_set_get_value_cb(&PropertyAnimation_0, &_ui_anim_callback_get_y);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->startRecordResource(), "Start record resource failed"
        );
        lv_anim_start(&PropertyAnimation_0);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->endRecordResource(), "End record resource failed"
        );
        ui_anim_user_data_t * PropertyAnimation_1_user_data = lv_malloc(sizeof(ui_anim_user_data_t));
        PropertyAnimation_1_user_data->target = TargetObject;
        PropertyAnimation_1_user_data->val = -1;
        lv_anim_t PropertyAnimation_1;
        lv_anim_init(&PropertyAnimation_1);
        lv_anim_set_time(&PropertyAnimation_1, 100);
        lv_anim_set_user_data(&PropertyAnimation_1, PropertyAnimation_1_user_data);
        lv_anim_set_custom_exec_cb(&PropertyAnimation_1, _ui_anim_callback_set_opacity);
        lv_anim_set_values(&PropertyAnimation_1, 0, 255);
        lv_anim_set_path_cb(&PropertyAnimation_1, lv_anim_path_linear);
        lv_anim_set_delay(&PropertyAnimation_1, delay + 0);
        lv_anim_set_deleted_cb(&PropertyAnimation_1, _ui_anim_callback_free_user_data);
        lv_anim_set_playback_time(&PropertyAnimation_1, 0);
        lv_anim_set_playback_delay(&PropertyAnimation_1, 0);
        lv_anim_set_repeat_count(&PropertyAnimation_1, 0);
        lv_anim_set_repeat_delay(&PropertyAnimation_1, 0);
        lv_anim_set_early_apply(&PropertyAnimation_1, true);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->startRecordResource(), "Start record resource failed"
        );
        lv_anim_start(&PropertyAnimation_1);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->endRecordResource(), "End record resource failed"
        );

    }
    void hour_Animation(lv_obj_t * TargetObject, int delay)
    {
        ui_anim_user_data_t * PropertyAnimation_0_user_data = lv_malloc(sizeof(ui_anim_user_data_t));
        PropertyAnimation_0_user_data->target = TargetObject;
        PropertyAnimation_0_user_data->val = -1;
        lv_anim_t PropertyAnimation_0;
        lv_anim_init(&PropertyAnimation_0);
        lv_anim_set_time(&PropertyAnimation_0, 1000);
        lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
        lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_image_angle);
        lv_anim_set_values(&PropertyAnimation_0, 0, 2800);
        lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_ease_out);
        lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
        lv_anim_set_deleted_cb(&PropertyAnimation_0, _ui_anim_callback_free_user_data);
        lv_anim_set_playback_time(&PropertyAnimation_0, 0);
        lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
        lv_anim_set_repeat_count(&PropertyAnimation_0, 0);
        lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
        lv_anim_set_early_apply(&PropertyAnimation_0, false);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->startRecordResource(), "Start record resource failed"
        );
        lv_anim_start(&PropertyAnimation_0);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->endRecordResource(), "End record resource failed"
        );
        ui_anim_user_data_t * PropertyAnimation_1_user_data = lv_malloc(sizeof(ui_anim_user_data_t));
        PropertyAnimation_1_user_data->target = TargetObject;
        PropertyAnimation_1_user_data->val = -1;
        lv_anim_t PropertyAnimation_1;
        lv_anim_init(&PropertyAnimation_1);
        lv_anim_set_time(&PropertyAnimation_1, 300);
        lv_anim_set_user_data(&PropertyAnimation_1, PropertyAnimation_1_user_data);
        lv_anim_set_custom_exec_cb(&PropertyAnimation_1, _ui_anim_callback_set_opacity);
        lv_anim_set_values(&PropertyAnimation_1, 0, 255);
        lv_anim_set_path_cb(&PropertyAnimation_1, lv_anim_path_linear);
        lv_anim_set_delay(&PropertyAnimation_1, delay + 0);
        lv_anim_set_deleted_cb(&PropertyAnimation_1, _ui_anim_callback_free_user_data);
        lv_anim_set_playback_time(&PropertyAnimation_1, 0);
        lv_anim_set_playback_delay(&PropertyAnimation_1, 0);
        lv_anim_set_repeat_count(&PropertyAnimation_1, 0);
        lv_anim_set_repeat_delay(&PropertyAnimation_1, 0);
        lv_anim_set_early_apply(&PropertyAnimation_1, true);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->startRecordResource(), "Start record resource failed"
        );
        lv_anim_start(&PropertyAnimation_1);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->endRecordResource(), "End record resource failed"
        );

    }
    void min_Animation(lv_obj_t * TargetObject, int delay)
    {
        ui_anim_user_data_t * PropertyAnimation_0_user_data = lv_malloc(sizeof(ui_anim_user_data_t));
        PropertyAnimation_0_user_data->target = TargetObject;
        PropertyAnimation_0_user_data->val = -1;
        lv_anim_t PropertyAnimation_0;
        lv_anim_init(&PropertyAnimation_0);
        lv_anim_set_time(&PropertyAnimation_0, 1000);
        lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
        lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_image_angle);
        lv_anim_set_values(&PropertyAnimation_0, 0, 2100);
        lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_ease_out);
        lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
        lv_anim_set_deleted_cb(&PropertyAnimation_0, _ui_anim_callback_free_user_data);
        lv_anim_set_playback_time(&PropertyAnimation_0, 0);
        lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
        lv_anim_set_repeat_count(&PropertyAnimation_0, 0);
        lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
        lv_anim_set_early_apply(&PropertyAnimation_0, false);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->startRecordResource(), "Start record resource failed"
        );
        lv_anim_start(&PropertyAnimation_0);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->endRecordResource(), "End record resource failed"
        );
        ui_anim_user_data_t * PropertyAnimation_1_user_data = lv_malloc(sizeof(ui_anim_user_data_t));
        PropertyAnimation_1_user_data->target = TargetObject;
        PropertyAnimation_1_user_data->val = -1;
        lv_anim_t PropertyAnimation_1;
        lv_anim_init(&PropertyAnimation_1);
        lv_anim_set_time(&PropertyAnimation_1, 200);
        lv_anim_set_user_data(&PropertyAnimation_1, PropertyAnimation_1_user_data);
        lv_anim_set_custom_exec_cb(&PropertyAnimation_1, _ui_anim_callback_set_opacity);
        lv_anim_set_values(&PropertyAnimation_1, 0, 255);
        lv_anim_set_path_cb(&PropertyAnimation_1, lv_anim_path_linear);
        lv_anim_set_delay(&PropertyAnimation_1, delay + 0);
        lv_anim_set_deleted_cb(&PropertyAnimation_1, _ui_anim_callback_free_user_data);
        lv_anim_set_playback_time(&PropertyAnimation_1, 0);
        lv_anim_set_playback_delay(&PropertyAnimation_1, 0);
        lv_anim_set_repeat_count(&PropertyAnimation_1, 0);
        lv_anim_set_repeat_delay(&PropertyAnimation_1, 0);
        lv_anim_set_early_apply(&PropertyAnimation_1, true);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->startRecordResource(), "Start record resource failed"
        );
        lv_anim_start(&PropertyAnimation_1);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->endRecordResource(), "End record resource failed"
        );

    }
    void sec_Animation(lv_obj_t * TargetObject, int delay)
    {
        ui_anim_user_data_t * PropertyAnimation_0_user_data = lv_malloc(sizeof(ui_anim_user_data_t));
        PropertyAnimation_0_user_data->target = TargetObject;
        PropertyAnimation_0_user_data->val = -1;
        lv_anim_t PropertyAnimation_0;
        lv_anim_init(&PropertyAnimation_0);
        lv_anim_set_time(&PropertyAnimation_0, 600);
        lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
        lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_image_angle);
        lv_anim_set_values(&PropertyAnimation_0, 0, 3600);
        lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_linear);
        lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
        lv_anim_set_deleted_cb(&PropertyAnimation_0, _ui_anim_callback_free_user_data);
        lv_anim_set_playback_time(&PropertyAnimation_0, 0);
        lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
        lv_anim_set_repeat_count(&PropertyAnimation_0, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
        lv_anim_set_early_apply(&PropertyAnimation_0, false);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->startRecordResource(), "Start record resource failed"
        );
        lv_anim_start(&PropertyAnimation_0);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->endRecordResource(), "End record resource failed"
        );
        ui_anim_user_data_t * PropertyAnimation_1_user_data = lv_malloc(sizeof(ui_anim_user_data_t));
        PropertyAnimation_1_user_data->target = TargetObject;
        PropertyAnimation_1_user_data->val = -1;
        lv_anim_t PropertyAnimation_1;
        lv_anim_init(&PropertyAnimation_1);
        lv_anim_set_time(&PropertyAnimation_1, 1000);
        lv_anim_set_user_data(&PropertyAnimation_1, PropertyAnimation_1_user_data);
        lv_anim_set_custom_exec_cb(&PropertyAnimation_1, _ui_anim_callback_set_opacity);
        lv_anim_set_values(&PropertyAnimation_1, 0, 255);
        lv_anim_set_path_cb(&PropertyAnimation_1, lv_anim_path_linear);
        lv_anim_set_delay(&PropertyAnimation_1, delay + 0);
        lv_anim_set_deleted_cb(&PropertyAnimation_1, _ui_anim_callback_free_user_data);
        lv_anim_set_playback_time(&PropertyAnimation_1, 0);
        lv_anim_set_playback_delay(&PropertyAnimation_1, 0);
        lv_anim_set_repeat_count(&PropertyAnimation_1, 0);
        lv_anim_set_repeat_delay(&PropertyAnimation_1, 0);
        lv_anim_set_early_apply(&PropertyAnimation_1, true);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->startRecordResource(), "Start record resource failed"
        );
        lv_anim_start(&PropertyAnimation_1);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->endRecordResource(), "End record resource failed"
        );

    }
    void scrolldot_Animation(lv_obj_t * TargetObject, int delay)
    {
        ui_anim_user_data_t * PropertyAnimation_0_user_data = lv_malloc(sizeof(ui_anim_user_data_t));
        PropertyAnimation_0_user_data->target = TargetObject;
        PropertyAnimation_0_user_data->val = -1;
        lv_anim_t PropertyAnimation_0;
        lv_anim_init(&PropertyAnimation_0);
        lv_anim_set_time(&PropertyAnimation_0, 300);
        lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
        lv_anim_set_custom_exec_cb(&PropertyAnimation_0, _ui_anim_callback_set_y);
        lv_anim_set_values(&PropertyAnimation_0, 30, -8);
        lv_anim_set_path_cb(&PropertyAnimation_0, lv_anim_path_ease_out);
        lv_anim_set_delay(&PropertyAnimation_0, delay + 0);
        lv_anim_set_deleted_cb(&PropertyAnimation_0, _ui_anim_callback_free_user_data);
        lv_anim_set_playback_time(&PropertyAnimation_0, 0);
        lv_anim_set_playback_delay(&PropertyAnimation_0, 0);
        lv_anim_set_repeat_count(&PropertyAnimation_0, 0);
        lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
        lv_anim_set_early_apply(&PropertyAnimation_0, true);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->startRecordResource(), "Start record resource failed"
        );
        lv_anim_start(&PropertyAnimation_0);
        ESP_UTILS_CHECK_FALSE_EXIT(
            SquarelineDemo::requestInstance()->endRecordResource(), "End record resource failed"
        );

    }

} // extern "C"

} // namespace esp_brookesia::apps
