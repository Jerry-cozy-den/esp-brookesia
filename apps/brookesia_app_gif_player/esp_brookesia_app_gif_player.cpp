/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "lvgl.h"
#include "esp_brookesia.hpp"
#ifdef ESP_UTILS_LOG_TAG
#   undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:GifPlayer"
#include "esp_lib_utils.h"
#include "esp_brookesia_app_gif_player.hpp"

using namespace std;
using namespace esp_brookesia::gui;

// 声明App图标
LV_IMG_DECLARE(esp_brookesia_app_icon_gif_player_112_112);

namespace esp_brookesia::apps {

GifPlayer *GifPlayer::_instance = nullptr;

GifPlayer *GifPlayer::requestInstance(bool use_status_bar, bool use_navigation_bar)
{
    if (_instance == nullptr) {
        _instance = new GifPlayer(use_status_bar, use_navigation_bar);
    }
    return _instance;
}

GifPlayer::GifPlayer(bool use_status_bar, bool use_navigation_bar):
    ESP_Brookesia_PhoneApp("GIF播放器", &esp_brookesia_app_icon_gif_player_112_112, false, use_status_bar, use_navigation_bar),
    _main_screen(nullptr),
    _gif_container(nullptr),
    _gif_img(nullptr),
    _control_panel(nullptr),
    _play_btn(nullptr),
    _loop_btn(nullptr),
    _file_list(nullptr),
    _status_label(nullptr),
    _is_playing(false),
    _is_paused(false),
    _loop_enabled(true)
{
    memset(_current_gif_path, 0, sizeof(_current_gif_path));
}

GifPlayer::~GifPlayer()
{
    ESP_UTILS_LOGD("析构GIF播放器");
}

bool GifPlayer::run(void)
{
    ESP_UTILS_LOGD("启动GIF播放器App");

    // 创建主UI界面
    _create_ui();
    
    // 扫描可用的GIF文件
    _scan_gif_files();

    return true;
}

bool GifPlayer::back(void)
{
    ESP_UTILS_LOGD("返回事件");

    // 停止当前播放的GIF
    stopGif();

    // 通知核心关闭App
    ESP_UTILS_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "通知核心关闭失败");

    return true;
}

bool GifPlayer::close(void)
{
    ESP_UTILS_LOGD("关闭GIF播放器");

    // 停止播放
    stopGif();

    return true;
}

bool GifPlayer::init(void)
{
    ESP_UTILS_LOGD("初始化GIF播放器");

    // 初始化状态
    _is_playing = false;
    _is_paused = false;
    _loop_enabled = true;

    return true;
}

bool GifPlayer::deinit(void)
{
    ESP_UTILS_LOGD("销毁GIF播放器");

    // 清理资源
    _instance = nullptr;

    return true;
}

void GifPlayer::_create_ui(void)
{
    // 获取当前屏幕
    lv_obj_t *screen = lv_scr_act();

    // 创建主容器
    lv_obj_t *main_container = lv_obj_create(screen);
    lv_obj_set_size(main_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(main_container, lv_color_black(), 0);
    lv_obj_set_style_border_width(main_container, 0, 0);
    lv_obj_set_style_pad_all(main_container, 10, 0);

    // 创建GIF显示区域
    _gif_container = lv_obj_create(main_container);
    lv_obj_set_size(_gif_container, LV_PCT(100), LV_PCT(70));
    lv_obj_align(_gif_container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(_gif_container, lv_color_hex(0x202020), 0);
    lv_obj_set_style_border_width(_gif_container, 1, 0);
    lv_obj_set_style_border_color(_gif_container, lv_color_hex(0x404040), 0);
    lv_obj_set_style_radius(_gif_container, 8, 0);

    // 创建GIF图像对象
    _gif_img = lv_gif_create(_gif_container);
    lv_obj_center(_gif_img);

    // 创建状态标签
    _status_label = lv_label_create(_gif_container);
    lv_label_set_text(_status_label, "请选择要播放的GIF文件");
    lv_obj_set_style_text_color(_status_label, lv_color_white(), 0);
    lv_obj_center(_status_label);

    // 创建控制面板
    _create_control_panel();

    // 创建文件列表
    _create_file_list();
}

void GifPlayer::_create_control_panel(void)
{
    lv_obj_t *main_container = lv_obj_get_parent(_gif_container);

    // 创建控制面板
    _control_panel = lv_obj_create(main_container);
    lv_obj_set_size(_control_panel, LV_PCT(100), 60);
    lv_obj_align_to(_control_panel, _gif_container, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(_control_panel, lv_color_hex(0x303030), 0);
    lv_obj_set_style_border_width(_control_panel, 1, 0);
    lv_obj_set_style_border_color(_control_panel, lv_color_hex(0x404040), 0);
    lv_obj_set_style_radius(_control_panel, 8, 0);
    lv_obj_set_flex_flow(_control_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_control_panel, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 播放/暂停按钮
    _play_btn = lv_btn_create(_control_panel);
    lv_obj_set_size(_play_btn, 80, 40);
    lv_obj_add_event_cb(_play_btn, _play_btn_event_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t *play_label = lv_label_create(_play_btn);
    lv_label_set_text(play_label, "播放");
    lv_obj_center(play_label);

    // 循环播放按钮
    _loop_btn = lv_btn_create(_control_panel);
    lv_obj_set_size(_loop_btn, 80, 40);
    lv_obj_add_event_cb(_loop_btn, _loop_btn_event_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t *loop_label = lv_label_create(_loop_btn);
    lv_label_set_text(loop_label, "循环");
    lv_obj_center(loop_label);

    // 更新按钮状态
    _update_button_state();
}

void GifPlayer::_create_file_list(void)
{
    lv_obj_t *main_container = lv_obj_get_parent(_gif_container);

    // 创建文件列表
    _file_list = lv_list_create(main_container);
    lv_obj_set_size(_file_list, LV_PCT(100), LV_PCT(20));
    lv_obj_align_to(_file_list, _control_panel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(_file_list, lv_color_hex(0x303030), 0);
    lv_obj_set_style_border_width(_file_list, 1, 0);
    lv_obj_set_style_border_color(_file_list, lv_color_hex(0x404040), 0);
    lv_obj_set_style_radius(_file_list, 8, 0);
}

void GifPlayer::_scan_gif_files(void)
{
    ESP_UTILS_LOGD("扫描GIF文件");

    // 清空现有列表
    lv_obj_clean(_file_list);

    // 模拟添加一些GIF文件（实际项目中应该扫描文件系统）
    const char *sample_gifs[] = {
        "/spiffs/gifs/sample1.gif",
        "/spiffs/gifs/sample2.gif", 
        "/spiffs/gifs/animation.gif",
        "/sdcard/gifs/demo.gif"
    };

    for (int i = 0; i < sizeof(sample_gifs) / sizeof(sample_gifs[0]); i++) {
        lv_obj_t *btn = lv_list_add_btn(_file_list, LV_SYMBOL_FILE, sample_gifs[i]);
        lv_obj_add_event_cb(btn, _file_list_event_cb, LV_EVENT_CLICKED, this);
        lv_obj_set_user_data(btn, (void*)sample_gifs[i]);
    }

    // 添加提示信息
    if (lv_obj_get_child_cnt(_file_list) == 0) {
        lv_obj_t *btn = lv_list_add_btn(_file_list, LV_SYMBOL_WARNING, "未找到GIF文件");
        lv_obj_add_state(btn, LV_STATE_DISABLED);
    }
}

bool GifPlayer::playGif(const char *gif_path)
{
    ESP_UTILS_LOGD("播放GIF: %s", gif_path);

    if (!gif_path) {
        ESP_UTILS_LOGE("GIF路径为空");
        return false;
    }

    // 停止当前播放
    stopGif();

    // 加载新的GIF文件
    if (_load_gif_file(gif_path)) {
        strcpy(_current_gif_path, gif_path);
        _is_playing = true;
        _is_paused = false;
        
        // 更新状态
        lv_label_set_text_fmt(_status_label, "正在播放: %s", gif_path);
        _update_button_state();
        
        return true;
    }

    return false;
}

void GifPlayer::stopGif(void)
{
    ESP_UTILS_LOGD("停止GIF播放");

    if (_gif_img && _is_playing) {
        // 停止GIF动画
        lv_gif_stop(_gif_img);
        _is_playing = false;
        _is_paused = false;
        
        // 更新状态
        lv_label_set_text(_status_label, "已停止播放");
        _update_button_state();
    }
}

void GifPlayer::togglePause(void)
{
    ESP_UTILS_LOGD("切换暂停状态");

    if (!_is_playing) {
        return;
    }

    if (_is_paused) {
        // 恢复播放
        lv_gif_restart(_gif_img);
        _is_paused = false;
        lv_label_set_text(_status_label, "正在播放");
    } else {
        // 暂停播放
        lv_gif_stop(_gif_img);
        _is_paused = true;
        lv_label_set_text(_status_label, "已暂停");
    }

    _update_button_state();
}

void GifPlayer::setLoop(bool loop)
{
    ESP_UTILS_LOGD("设置循环播放: %s", loop ? "开启" : "关闭");
    
    _loop_enabled = loop;
    _update_button_state();
}

bool GifPlayer::_load_gif_file(const char *path)
{
    ESP_UTILS_LOGD("加载GIF文件: %s", path);

    // 实际项目中，这里应该从文件系统加载GIF文件
    // 由于演示目的，我们使用预编译的资源
    
    // 设置GIF源（这里需要根据实际情况调整）
    // lv_gif_set_src(_gif_img, path);
    
    // 如果文件不存在，显示错误信息
    lv_label_set_text_fmt(_status_label, "错误: 无法加载 %s", path);
    
    return false; // 演示中返回false，实际实现中应该返回真实状态
}

void GifPlayer::_update_button_state(void)
{
    // 更新播放按钮
    if (_play_btn) {
        lv_obj_t *label = lv_obj_get_child(_play_btn, 0);
        if (_is_playing && !_is_paused) {
            lv_label_set_text(label, "暂停");
        } else {
            lv_label_set_text(label, "播放");
        }
    }

    // 更新循环按钮
    if (_loop_btn) {
        lv_obj_t *label = lv_obj_get_child(_loop_btn, 0);
        if (_loop_enabled) {
            lv_label_set_text(label, "循环: 开");
            lv_obj_set_style_bg_color(_loop_btn, lv_color_hex(0x00AA00), 0);
        } else {
            lv_label_set_text(label, "循环: 关");
            lv_obj_set_style_bg_color(_loop_btn, lv_color_hex(0x404040), 0);
        }
    }
}

// 静态回调函数实现
void GifPlayer::_play_btn_event_cb(lv_event_t *e)
{
    GifPlayer *app = (GifPlayer*)lv_event_get_user_data(e);
    
    if (app->_is_playing) {
        app->togglePause();
    } else {
        // 如果有当前文件路径，重新播放
        if (strlen(app->_current_gif_path) > 0) {
            app->playGif(app->_current_gif_path);
        }
    }
}

void GifPlayer::_loop_btn_event_cb(lv_event_t *e)
{
    GifPlayer *app = (GifPlayer*)lv_event_get_user_data(e);
    app->setLoop(!app->_loop_enabled);
}

void GifPlayer::_file_list_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    GifPlayer *app = (GifPlayer*)lv_event_get_user_data(e);
    
    const char *gif_path = (const char*)lv_obj_get_user_data(btn);
    if (gif_path) {
        app->playGif(gif_path);
    }
}

} // namespace esp_brookesia::apps
