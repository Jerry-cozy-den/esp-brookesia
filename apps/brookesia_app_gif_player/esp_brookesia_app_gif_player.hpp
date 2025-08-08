/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "systems/phone/esp_brookesia_phone_app.hpp"

namespace esp_brookesia::apps {

/**
 * @brief GIF播放器App - 用于播放动画GIF图片
 * 
 * 这个App可以加载和播放存储在Flash或SD卡中的GIF动画文件
 * 支持播放控制、缩放、循环播放等功能
 */
class GifPlayer: public ESP_Brookesia_PhoneApp {
public:
    /**
     * @brief 获取GifPlayer的单例实例
     *
     * @param use_status_bar 是否显示状态栏
     * @param use_navigation_bar 是否显示导航栏
     * @return 单例实例指针
     */
    static GifPlayer *requestInstance(bool use_status_bar = true, bool use_navigation_bar = true);

    /**
     * @brief 析构函数
     */
    ~GifPlayer();

    /**
     * @brief 播放指定的GIF文件
     * 
     * @param gif_path GIF文件路径
     * @return true 成功, false 失败
     */
    bool playGif(const char *gif_path);

    /**
     * @brief 停止当前播放的GIF
     */
    void stopGif(void);

    /**
     * @brief 暂停/恢复GIF播放
     */
    void togglePause(void);

    /**
     * @brief 设置是否循环播放
     * 
     * @param loop true循环播放, false播放一次
     */
    void setLoop(bool loop);

    using ESP_Brookesia_PhoneApp::startRecordResource;
    using ESP_Brookesia_PhoneApp::endRecordResource;

protected:
    /**
     * @brief 私有构造函数，实现单例模式
     *
     * @param use_status_bar 是否显示状态栏
     * @param use_navigation_bar 是否显示导航栏
     */
    GifPlayer(bool use_status_bar, bool use_navigation_bar);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////// 必须由用户App类实现的函数 /////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief App开始运行时调用，创建所有UI资源
     *
     * @return true 成功, false 失败
     */
    bool run(void) override;

    /**
     * @brief 接收返回事件时调用
     *
     * @return true 成功, false 失败
     */
    bool back(void) override;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////// 可选重写的函数 //////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief App开始关闭时调用
     *
     * @return true 成功, false 失败
     */
    bool close(void) override;

    /**
     * @brief App安装时调用，进行初始化
     *
     * @return true 成功, false 失败
     */
    bool init(void) override;

    /**
     * @brief App卸载时调用，进行清理
     *
     * @return true 成功, false 失败
     */
    bool deinit(void) override;

private:
    // 单例实例
    static GifPlayer *_instance;

    // UI组件
    lv_obj_t *_main_screen;       // 主屏幕
    lv_obj_t *_gif_container;     // GIF容器
    lv_obj_t *_gif_img;           // GIF图像对象
    lv_obj_t *_control_panel;     // 控制面板
    lv_obj_t *_play_btn;          // 播放/暂停按钮
    lv_obj_t *_loop_btn;          // 循环按钮
    lv_obj_t *_file_list;         // 文件列表
    lv_obj_t *_status_label;      // 状态标签

    // GIF控制状态
    bool _is_playing;             // 是否正在播放
    bool _is_paused;              // 是否暂停
    bool _loop_enabled;           // 是否循环播放
    char _current_gif_path[256];  // 当前GIF文件路径

    // 回调函数
    static void _play_btn_event_cb(lv_event_t *e);
    static void _loop_btn_event_cb(lv_event_t *e);
    static void _file_list_event_cb(lv_event_t *e);

    // 内部方法
    void _create_ui(void);
    void _create_control_panel(void);
    void _create_file_list(void);
    void _update_button_state(void);
    void _scan_gif_files(void);
    bool _load_gif_file(const char *path);
};

} // namespace esp_brookesia::apps
