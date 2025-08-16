/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
// AI配置应用实现文件 - 用于管理和选择AI聊天机器人角色
#include "lvgl.h"
#ifdef ESP_UTILS_LOG_TAG
#   undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:App:AI_Profile"  // 日志标签
#include "esp_lib_utils.h"
#include "ui/ui.h"
#include "esp_brookesia_app_ai_profile.hpp"

#define MAX_ROBOT_NUM 2  // 最大支持的机器人数量

// UI界面透明度设置
#define UI_SCREEN_AI_PROFILE_PANEL_INDICATOR_OPA_ACTIVE     128  // 激活状态指示器透明度
#define UI_SCREEN_AI_PROFILE_PANEL_INDICATOR_OPA_INACTIVE   50   // 非激活状态指示器透明度

// 重启聊天线程配置
#define RESTART_CHAT_THREAD_NAME              "restart_chat"          // 线程名称
#define RESTART_CHAT_THREAD_STACK_SIZE        (10 * 1024)             // 线程栈大小(10KB)
#define RESTART_CHAT_THREAD_STACK_CAPS_EXT    (true)                  // 扩展栈能力

using namespace std;
using namespace esp_brookesia::gui;
using namespace esp_brookesia::speaker;
using namespace esp_brookesia::ai_framework;

// 声明AI配置应用图标
LV_IMG_DECLARE(esp_brookesia_app_icon_launcher_ai_profile_112_112);

namespace esp_brookesia::speaker_apps {

/**
 * @brief 获取AI配置应用实例（单例模式）
 * @return AI_Profile* 应用实例指针，失败返回nullptr
 */
AI_Profile *AI_Profile::requestInstance()
{
    if (_instance == nullptr) {
        // 创建新的AI配置应用实例
        ESP_UTILS_CHECK_EXCEPTION_RETURN(
            _instance = new AI_Profile(), nullptr, "Failed to create instance"
        );
    }
    return _instance;
}

/**
 * @brief AI配置应用构造函数
 * 初始化应用名称、图标和语音控制能力
 */
AI_Profile::AI_Profile():
    speaker::App("AI Profile", &esp_brookesia_app_icon_launcher_ai_profile_112_112, true)
{
}

/**
 * @brief AI配置应用析构函数
 */
AI_Profile::~AI_Profile()
{
}

/**
 * @brief 运行AI配置应用
 * 主要功能：
 * 1. 检查聊天服务器连接状态
 * 2. 获取可用的机器人信息
 * 3. 初始化UI界面
 * 4. 设置事件回调函数
 * @return true 成功运行，false 运行失败
 */
bool AI_Profile::run(void)
{
    ESP_UTILS_LOGD("Run");

    // 检查聊天状态是否已启动
    if (Agent::requestInstance()->hasChatState(Agent::ChatState::ChatStateStarted)) {
        goto next;  // 如果聊天已启动，跳转到正常流程
    }

    {
        // 聊天服务器未连接时，显示提示信息
        auto screen = lv_obj_create(nullptr);
        ESP_UTILS_CHECK_NULL_RETURN(screen, false, "Failed to create screen");
        auto label = lv_label_create(screen);
        ESP_UTILS_CHECK_NULL_RETURN(label, false, "Failed to create label");
        lv_label_set_text(label, "Chat server is not connected, please exit and restart the app after the server is connected");
        lv_obj_center(label);  // 居中显示标签
        lv_obj_set_size(label, lv_pct(80), LV_SIZE_CONTENT);  // 设置标签大小为屏幕宽度80%
        lv_scr_load(screen);  // 加载显示屏幕
        goto end;
    }

next: {
        // 获取机器人信息和当前选中的机器人索引
        std::vector<CozeChatRobotInfo> robot_infos;
        ESP_UTILS_CHECK_FALSE_RETURN(
            Agent::requestInstance()->getRobotInfo(robot_infos), false, "Failed to get robot infos"
        );
        ESP_UTILS_CHECK_FALSE_RETURN(
            Agent::requestInstance()->getCurrentRobotIndex(_robot_current_index),
            false,
            "Failed to get current robot index"
        );
        _robot_next_index = _robot_current_index;  // 初始化下一个要选择的机器人索引

        // 初始化UI界面资源
        speaker_ai_profile_ui_init();

        // 隐藏所有角色页面和指示器（默认状态）
        lv_obj_add_flag(ui_ScreenAIProfileTabpageTabPageRole1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ScreenAIProfilePanelPanelIndicator1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ScreenAIProfileTabpageTabPageRole2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ScreenAIProfilePanelPanelIndicator2, LV_OBJ_FLAG_HIDDEN);

        // 根据获取到的机器人信息更新UI显示
        for (int i = 0; i < robot_infos.size() && (i < MAX_ROBOT_NUM); i++) {
            if (i == 0) {
                // 设置第一个机器人角色的名称和描述
                lv_label_set_text(ui_ScreenAIProfileLabelLabelRole1Name, robot_infos[i].name.c_str());
                lv_label_set_text(ui_ScreenAIProfileLabelLabelRole1Description, robot_infos[i].description.c_str());
                // 显示第一个角色页面和指示器
                lv_obj_remove_flag(ui_ScreenAIProfileTabpageTabPageRole1, LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(ui_ScreenAIProfilePanelPanelIndicator1, LV_OBJ_FLAG_HIDDEN);
            } else {
                // 设置第二个机器人角色的名称和描述
                lv_label_set_text(ui_ScreenAIProfileLabelLabelRole2Name, robot_infos[i].name.c_str());
                lv_label_set_text(ui_ScreenAIProfileLabelLabelRole2Description, robot_infos[i].description.c_str());
                // 显示第二个角色页面和指示器
                lv_obj_remove_flag(ui_ScreenAIProfileTabpageTabPageRole2, LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(ui_ScreenAIProfilePanelPanelIndicator2, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    // 更新指示器状态（第一个激活，第二个非激活）
    lv_obj_set_style_bg_opa(ui_ScreenAIProfilePanelPanelIndicator1, UI_SCREEN_AI_PROFILE_PANEL_INDICATOR_OPA_ACTIVE, 0);
    lv_obj_set_style_bg_opa(ui_ScreenAIProfilePanelPanelIndicator2, UI_SCREEN_AI_PROFILE_PANEL_INDICATOR_OPA_INACTIVE, 0);

    // 为标签视图添加事件回调 - 处理标签页切换
    lv_obj_add_event_cb(ui_ScreenAIProfileTabviewTabView, [](lv_event_t *e) {
        auto app = (AI_Profile *)lv_event_get_user_data(e);
        ESP_UTILS_CHECK_NULL_EXIT(app, "App is NULL");

        lv_obj_t *tab_view = (lv_obj_t *)lv_event_get_target(e);
        ESP_UTILS_CHECK_NULL_EXIT(tab_view, "Tab view is NULL");

        int index = lv_tabview_get_tab_act(tab_view);  // 获取当前激活的标签页索引
        ESP_UTILS_LOGD("Tab changed to index: %d", index);

        if (index == 0) {
            // 切换到第一个标签页时的指示器状态更新
            lv_obj_set_style_bg_opa(
                ui_ScreenAIProfilePanelPanelIndicator1, UI_SCREEN_AI_PROFILE_PANEL_INDICATOR_OPA_ACTIVE, 0
            );
            lv_obj_set_style_bg_opa(
                ui_ScreenAIProfilePanelPanelIndicator2, UI_SCREEN_AI_PROFILE_PANEL_INDICATOR_OPA_INACTIVE, 0
            );
        } else if (index == 1) {
            // 切换到第二个标签页时的指示器状态更新
            lv_obj_set_style_bg_opa(
                ui_ScreenAIProfilePanelPanelIndicator1, UI_SCREEN_AI_PROFILE_PANEL_INDICATOR_OPA_INACTIVE, 0
            );
            lv_obj_set_style_bg_opa(
                ui_ScreenAIProfilePanelPanelIndicator2, UI_SCREEN_AI_PROFILE_PANEL_INDICATOR_OPA_ACTIVE, 0
            );
        }
    }, LV_EVENT_VALUE_CHANGED, this);

    // 为第一个角色选择按钮添加点击事件回调
    lv_obj_add_event_cb(ui_ScreenAIProfileButtonButtonRole1Select, [](lv_event_t *e) {
        auto app = (AI_Profile *)lv_event_get_user_data(e);
        ESP_UTILS_CHECK_NULL_EXIT(app, "App is NULL");

        // 显示第一个角色的选中状态图标，隐藏第一个角色的选择按钮
        lv_obj_remove_flag(ui_ScreenAIProfileImageImageRole1Select, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui_ScreenAIProfileButtonButtonRole2Select, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ScreenAIProfileButtonButtonRole1Select, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ScreenAIProfileImageImageRole2Select, LV_OBJ_FLAG_HIDDEN);

        app->_robot_next_index = 0;  // 设置下一个要选择的机器人索引为0
    }, LV_EVENT_CLICKED, this);

    // 为第二个角色选择按钮添加点击事件回调
    lv_obj_add_event_cb(ui_ScreenAIProfileButtonButtonRole2Select, [](lv_event_t *e) {
        auto app = (AI_Profile *)lv_event_get_user_data(e);
        ESP_UTILS_CHECK_NULL_EXIT(app, "App is NULL");

        // 显示第二个角色的选中状态图标，隐藏第二个角色的选择按钮
        lv_obj_add_flag(ui_ScreenAIProfileImageImageRole1Select, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ScreenAIProfileButtonButtonRole2Select, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui_ScreenAIProfileButtonButtonRole1Select, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui_ScreenAIProfileImageImageRole2Select, LV_OBJ_FLAG_HIDDEN);

        app->_robot_next_index = 1;  // 设置下一个要选择的机器人索引为1
    }, LV_EVENT_CLICKED, this);

    // 设置当前选中的标签页（无动画效果）
    lv_tabview_set_active(ui_ScreenAIProfileTabviewTabView, _robot_next_index, LV_ANIM_OFF);
    
    // 根据当前机器人索引设置UI元素的显示状态
    if (_robot_current_index == 0) {
        // 当前选中第一个机器人时的UI状态
        lv_obj_remove_flag(ui_ScreenAIProfileImageImageRole1Select, LV_OBJ_FLAG_HIDDEN);    // 显示第一个角色选中图标
        lv_obj_remove_flag(ui_ScreenAIProfileButtonButtonRole2Select, LV_OBJ_FLAG_HIDDEN);  // 显示第二个角色选择按钮
        lv_obj_add_flag(ui_ScreenAIProfileButtonButtonRole1Select, LV_OBJ_FLAG_HIDDEN);     // 隐藏第一个角色选择按钮
        lv_obj_add_flag(ui_ScreenAIProfileImageImageRole2Select, LV_OBJ_FLAG_HIDDEN);       // 隐藏第二个角色选中图标
    } else {
        // 当前选中第二个机器人时的UI状态
        lv_obj_add_flag(ui_ScreenAIProfileImageImageRole1Select, LV_OBJ_FLAG_HIDDEN);       // 隐藏第一个角色选中图标
        lv_obj_add_flag(ui_ScreenAIProfileButtonButtonRole2Select, LV_OBJ_FLAG_HIDDEN);     // 隐藏第二个角色选择按钮
        lv_obj_remove_flag(ui_ScreenAIProfileButtonButtonRole1Select, LV_OBJ_FLAG_HIDDEN);  // 显示第一个角色选择按钮
        lv_obj_remove_flag(ui_ScreenAIProfileImageImageRole2Select, LV_OBJ_FLAG_HIDDEN);    // 显示第二个角色选中图标
    }

end:
    return true;
}

/**
 * @brief 处理应用返回操作
 * 当用户按下返回键或执行返回操作时调用
 * @return true 成功处理返回操作，false 处理失败
 */
bool AI_Profile::back(void)
{
    ESP_UTILS_LOGD("Back");

    // 通知核心关闭应用
    ESP_UTILS_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

    return true;
}

/**
 * @brief 关闭应用时的处理函数
 * 主要功能：
 * 1. 检查机器人选择是否有变化
 * 2. 如果有变化，更新当前机器人并重启聊天服务
 * @return true 成功关闭，false 关闭失败
 */
bool AI_Profile::close(void)
{
    ESP_UTILS_LOGD("Close");

    // 检查机器人选择是否有变化
    if (_robot_current_index == _robot_next_index) {
        goto end;  // 没有变化，直接结束
    }

    // 设置新选择的机器人
    if (!Agent::requestInstance()->setCurrentRobotIndex(_robot_next_index)) {
        ESP_UTILS_LOGE("Set current robot failed");
    } else {
        // 成功设置新机器人后，重启聊天服务
        Agent::requestInstance()->sendChatEvent(Agent::ChatEvent::Stop);   // 停止当前聊天
        Agent::requestInstance()->sendChatEvent(Agent::ChatEvent::Start, false);  // 启动新的聊天会话
    }

end:
    return true;
}

// 以下为可选的应用生命周期回调函数实现
// 如果需要在特定时机执行操作，可以取消注释并实现相应逻辑

// /**
//  * @brief 初始化应用
//  * 在应用首次创建时调用，用于执行一次性初始化操作
//  * @return true 初始化成功，false 初始化失败
//  */
// bool AI_Profile::init()
// {
//     ESP_UTILS_LOGD("Init");

//     /* 在此处执行必要的初始化操作 */

//     return true;
// }

// /**
//  * @brief 反初始化应用
//  * 在应用销毁时调用，用于清理资源
//  * @return true 反初始化成功，false 反初始化失败
//  */
// bool AI_Profile::deinit()
// {
//     ESP_UTILS_LOGD("Deinit");

//     /* 在此处执行必要的反初始化操作 */

//     return true;
// }

// /**
//  * @brief 暂停应用
//  * 当应用失去焦点或进入后台时调用
//  * @return true 暂停成功，false 暂停失败
//  */
// bool AI_Profile::pause()
// {
//     ESP_UTILS_LOGD("Pause");

//     /* 在此处执行暂停相关操作 */

//     return true;
// }

// /**
//  * @brief 恢复应用
//  * 当应用重新获得焦点或从后台返回时调用
//  * @return true 恢复成功，false 恢复失败
//  */
// bool AI_Profile::resume()
// {
//     ESP_UTILS_LOGD("Resume");

//     /* 在此处执行恢复相关操作 */

//     return true;
// }

// /**
//  * @brief 清理应用资源
//  * 在需要释放应用占用的资源时调用
//  * @return true 清理成功，false 清理失败
//  */
// bool AI_Profile::cleanResource()
// {
//     ESP_UTILS_LOGD("Clean resource");

//     /* 在此处执行资源清理操作 */

//     return true;
// }

} // namespace esp_brookesia::speaker_apps
