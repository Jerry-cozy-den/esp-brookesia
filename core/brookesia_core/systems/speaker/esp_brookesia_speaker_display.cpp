/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// ESP Brookesia Speaker 显示模块实现
// 主要功能：
// 1. 管理应用启动器界面和图标显示
// 2. 处理快速设置面板和键盘组件
// 3. 控制启动动画和虚拟绘制遮罩
// 4. 管理应用的安装、卸载、运行状态变化

#include <mutex>
#include "esp_brookesia_systems_internal.h"
#if !ESP_BROOKESIA_SPEAKER_DISPLAY_ENABLE_DEBUG_LOG
#   define ESP_BROOKESIA_UTILS_DISABLE_DEBUG_LOG
#endif
#include "private/esp_brookesia_speaker_utils.hpp"
#include "esp_brookesia_speaker_app.hpp"
#include "esp_brookesia_speaker_display.hpp"

using namespace std;

namespace esp_brookesia::speaker {

// 虚拟绘制信号，用于控制界面绘制状态
Display::OnDummyDrawSignal Display::on_dummy_draw_signal;

// 构造函数：初始化显示模块及其子组件（应用启动器、快速设置、键盘）
Display::Display(ESP_Brookesia_Core &core, const DisplayData &data):
    ESP_Brookesia_CoreDisplay(core, core.getCoreData().display),
    _data(data),
    _app_launcher(core, data.app_launcher.data),
    _quick_settings(core, data.quick_settings.data),
    _keyboard(core, data.keyboard.data)
{
}

// 析构函数：清理资源
Display::~Display()
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    if (!del()) {
        ESP_UTILS_LOGE("Failed to delete");
    }

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
}

bool Display::begin(void)
{
    // 初始化显示模块：启动应用启动器、键盘和快速设置组件
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(!checkInitialized(), false, "Already initialized");

    // 初始化应用启动器到主屏幕
    const auto main_screen_obj = _core.getCoreDisplay().getMainScreenObjectPtr();
    ESP_UTILS_CHECK_FALSE_RETURN(
        _app_launcher.begin(main_screen_obj->getNativeHandle()), false, "Begin app launcher failed"
    );

    // 初始化键盘和快速设置到系统屏幕，默认隐藏
    const auto system_screen_obj = _core.getCoreDisplay().getSystemScreenObjectPtr();
    ESP_UTILS_CHECK_FALSE_RETURN(
        _keyboard.begin(system_screen_obj), false, "Begin keyboard failed"
    );
    ESP_UTILS_CHECK_FALSE_RETURN(
        _keyboard.setVisible(false), false, "Set keyboard visible failed"
    );

    ESP_UTILS_CHECK_FALSE_RETURN(
        _quick_settings.begin(*system_screen_obj), false, "Begin quick settings failed"
    );
    ESP_UTILS_CHECK_FALSE_RETURN(
        _quick_settings.setVisible(false), false, "Set quick settings visible failed"
    );

    // 创建虚拟绘制遮罩，用于控制界面绘制状态
    _dummy_draw_mask = std::make_unique<gui::LvContainer>(_core.getCoreDisplay().getSystemScreenObjectPtr());
    ESP_UTILS_CHECK_NULL_RETURN(_dummy_draw_mask, false, "Create dummy draw mask failed");
    _dummy_draw_mask->moveForeground();
    _dummy_draw_mask->setStyleAttribute(gui::StyleFlag::STYLE_FLAG_HIDDEN | gui::StyleFlag::STYLE_FLAG_CLICKABLE, true);

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool Display::del(void)
{
    // 删除显示模块：清理应用启动器等组件
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    if (!checkInitialized()) {
        return true;
    }

    if (!_app_launcher.del()) {
        ESP_UTILS_LOGE("Delete app launcher failed");
    }

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::processAppInstall(ESP_Brookesia_CoreApp *app)
{
    // 处理应用安装：在应用启动器中添加应用图标
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    App *speaker_app = static_cast<App *>(app);
    AppLauncherIconInfo_t icon_info = {};

    ESP_UTILS_CHECK_NULL_RETURN(speaker_app, false, "Invalid speaker app");
    ESP_UTILS_CHECK_FALSE_RETURN(checkInitialized(), false, "Not initialized");
    ESP_UTILS_LOGD("Param: app_id(%d)", speaker_app->getId());

    // 为应用启动器添加图标，如果没有提供图标则使用默认图标
    icon_info = (AppLauncherIconInfo_t) {
        speaker_app->getName(), speaker_app->getLauncherIcon(), speaker_app->getId()
    };
    if (speaker_app->getLauncherIcon().resource == nullptr) {
        ESP_UTILS_LOGW("No launcher icon provided, use default icon");
        icon_info.image = _data.app_launcher.default_image;
        speaker_app->setLauncherIconImage(icon_info.image);
    }
    ESP_UTILS_CHECK_FALSE_RETURN(_app_launcher.addIcon(speaker_app->getActiveData().app_launcher_page_index, icon_info),
                                 false, "Add launcher icon failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::processAppUninstall(ESP_Brookesia_CoreApp *app)
{
    // 处理应用卸载：从应用启动器中移除应用图标
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    App *speaker_app = static_cast<App *>(app);

    ESP_UTILS_CHECK_NULL_RETURN(speaker_app, false, "Invalid speaker app");
    ESP_UTILS_CHECK_FALSE_RETURN(checkInitialized(), false, "Not initialized");
    ESP_UTILS_LOGD("Param: app_id(%d)", speaker_app->getId());

    // 从应用启动器中移除图标
    ESP_UTILS_CHECK_FALSE_RETURN(_app_launcher.removeIcon(speaker_app->getId()), false, "Remove launcher icon failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::processAppRun(ESP_Brookesia_CoreApp *app)
{
    // 处理应用运行：当应用开始运行时的显示处理
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    App *speaker_app = static_cast<App *>(app);

    ESP_UTILS_CHECK_NULL_RETURN(speaker_app, false, "Invalid speaker app");
    ESP_UTILS_CHECK_FALSE_RETURN(checkInitialized(), false, "Not initialized");
    ESP_UTILS_LOGD("Param: app_id(%d)", speaker_app->getId());

    // const AppData_t &app_data = speaker_app->getActiveData();

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::processAppResume(ESP_Brookesia_CoreApp *app)
{
    // 处理应用恢复：当应用从后台恢复时的显示处理
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    App *speaker_app = static_cast<App *>(app);

    ESP_UTILS_CHECK_NULL_RETURN(speaker_app, false, "Invalid speaker app");
    ESP_UTILS_CHECK_FALSE_RETURN(checkInitialized(), false, "Not initialized");
    ESP_UTILS_LOGD("Param: app_id(%d)", speaker_app->getId());

    // const AppData_t &app_data = speaker_app->getActiveData();

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::processAppClose(ESP_Brookesia_CoreApp *app)
{
    // 处理应用关闭：当应用关闭时的显示清理
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    App *speaker_app = static_cast<App *>(app);

    ESP_UTILS_CHECK_NULL_RETURN(speaker_app, false, "Invalid speaker app");
    ESP_UTILS_CHECK_FALSE_RETURN(checkInitialized(), false, "Not initialized");
    ESP_UTILS_LOGD("Param: app_id(%d)", speaker_app->getId());

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::processMainScreenLoad(void)
{
    // 加载主屏幕：切换到主屏幕显示
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkInitialized(), false, "Not initialized");

    lv_obj_t *main_screen = _core.getCoreDisplay().getMainScreen();
    ESP_UTILS_CHECK_FALSE_RETURN(lv_obj_is_valid(main_screen), false, "Invalid main screen");
    lv_scr_load(main_screen);

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::getAppVisualArea(ESP_Brookesia_CoreApp *app, lv_area_t &app_visual_area) const
{
    // 获取应用的可视区域：返回应用在屏幕上的显示区域
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    App *speaker_app = static_cast<App *>(app);

    ESP_UTILS_CHECK_NULL_RETURN(speaker_app, false, "Invalid speaker app");
    ESP_UTILS_LOGD("Param: app_id(%d)", speaker_app->getId());

    // 设置应用可视区域为整个屏幕大小
    lv_area_t visual_area = {
        .x1 = 0,
        .y1 = 0,
        .x2 = (lv_coord_t)(_core.getCoreData().screen_size.width - 1),
        .y2 = (lv_coord_t)(_core.getCoreData().screen_size.height - 1),
    };

    app_visual_area = visual_area;

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::processDummyDraw(bool enable)
{
    // 处理虚拟绘制：控制绘制遮罩的显示/隐藏状态
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    ESP_UTILS_CHECK_FALSE_RETURN(checkInitialized(), false, "Not initialized");
    ESP_UTILS_LOGD("Param: enable(%d)", enable);

    // 设置遮罩的隐藏状态并发送信号
    _dummy_draw_mask->setStyleAttribute(gui::StyleFlag::STYLE_FLAG_HIDDEN, !enable);
    on_dummy_draw_signal(enable);

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::startBootAnimation(void)
{
    // 启动开机动画：播放系统启动时的动画效果
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    std::unique_ptr<gui::AnimPlayer> boot_animation = std::make_unique<gui::AnimPlayer>();
    ESP_UTILS_CHECK_FALSE_RETURN(boot_animation->begin(_data.boot_animation.data), false, "Begin boot animation failed");
    ESP_UTILS_CHECK_FALSE_RETURN(
        boot_animation->sendEvent({0, gui::AnimPlayer::Operation::PlayOnceStop, {true, true}}, true), false,
        "Set boot animation failed"
    );
    ESP_UTILS_CHECK_FALSE_RETURN(boot_animation->waitAnimationStop(), false, "Wait boot animation end failed");

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

bool Display::calibrateData(const ESP_Brookesia_StyleSize_t &screen_size, DisplayData &data)
{
    // 校准显示数据：根据屏幕尺寸调整各组件的大小和位置
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();

    // 初始化弹性组件的尺寸
    if (data.flags.enable_app_launcher_flex_size) {
        data.app_launcher.data.main.y_start = 0;
        data.app_launcher.data.main.size.flags.enable_height_percent = 0;
        data.app_launcher.data.main.size.height = screen_size.height;
    }

    // 校准应用启动器数据
    ESP_UTILS_CHECK_FALSE_RETURN(
        AppLauncher::calibrateData(screen_size, *this, data.app_launcher.data), false,
        "Calibrate app launcher data failed"
    );
    // 校准快速设置数据
    ESP_UTILS_CHECK_FALSE_RETURN(
        QuickSettings::calibrateData(screen_size, *this, data.quick_settings.data), false,
        "Calibrate quick settings data failed"
    );
    // 校准键盘数据
    ESP_UTILS_CHECK_FALSE_RETURN(
        Keyboard::calibrateData(screen_size, *this, data.keyboard.data), false,
        "Calibrate keyboard data failed"
    );

    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

} // namespace esp_brookesia::speaker
