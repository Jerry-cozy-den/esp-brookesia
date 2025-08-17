/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <math.h>
#include <vector>
#include <unistd.h>
#ifdef ESP_UTILS_LOG_TAG
#   undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:App:Player"
#include "esp_lib_utils.h"
#include "esp_brookesia_app_Player.hpp"
#include "ui/ui.h"

using namespace std;
using namespace esp_brookesia::speaker;

LV_IMG_DECLARE(img_app_Player);

// 键盘高度占屏幕的70%
#define KEYBOARD_H_PERCENT      70
#define KEYBOARD_FONT           &lv_font_montserrat_20
#define KEYBOARD_SPECIAL_COLOR  lv_color_hex(0xFF3034)
#define KEYBOARD_BG_COLOR       lv_color_hex(0xFFFFFF)

#define LABEL_PAD               3
#define LABEL_FONT_SMALL        &lv_font_montserrat_16
#define LABEL_FONT_BIG          &lv_font_montserrat_20
#define LABEL_COLOR             lv_color_hex(0xFF3034)
#define LABEL_FORMULA_LEN_MAX   256

// Adaptation for 360x360 round screen
#define SCREEN_360_EFFECTIVE_WIDTH  320  // Effective display area for round screen
#define SCREEN_360_EFFECTIVE_HEIGHT 320

// 键盘布局定义
static const char *keyboard_map[] = {
    "C", "/", "x", LV_SYMBOL_BACKSPACE, "\n",  // 第一行: 清除、除号、乘号、退格
    "7", "8", "9", "-", "\n",                  // 第二行: 数字7-9和减号
    "4", "5", "6", "+", "\n",                  // 第三行: 数字4-6和加号
    "1", "2", "3", "%", "\n",                  // 第四行: 数字1-3和百分号
    "0", ".", "=", ""                          // 第五行: 数字0、小数点、等号
};

namespace esp_brookesia::speaker_apps {

Player::Player():
    App( {
    .name = "Player",
    .launcher_icon = ESP_BROOKESIA_STYLE_IMAGE(&img_app_Player),
    .screen_size = ESP_BROOKESIA_STYLE_SIZE_RECT_PERCENT(100, 100),
    .flags = {
        .enable_default_screen = 1,
        .enable_recycle_resource = 0,
        .enable_resize_visual_area = 1,
    },
},
{
    .app_launcher_page_index = 0,
    .flags = {
        .enable_navigation_gesture = 1,
    },
}
   )
{
}

Player::~Player()
{
    ESP_UTILS_LOGD("Destroy(@0x%p)", this);
}

bool Player::init(void)
{
    ESP_UTILS_LOGD("Init(@0x%p)", this);
    return true;
}

bool Player::deinit(void)
{
    ESP_UTILS_LOGD("Deinit(@0x%p)", this);
    return true;
}

bool Player::run(void)
{
    ESP_UTILS_LOGD("Run(@0x%p)", this);
    
    _is_starting = true;

    player_ui_init();

    _is_starting = false;
    return true;
}

bool Player::back(void)
{
    ESP_UTILS_LOGD("Back(@0x%p)", this);

    ESP_UTILS_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

    return true;
}

bool Player::close(void)
{
    ESP_UTILS_LOGD("Close(@0x%p)", this);
    _is_stopping = true;

    _is_stopping = false;
    return true;
}

bool Player::isStartZero(void)
{
    const char *text = lv_label_get_text(formula_label);
    int text_len = strlen(text);

    if ((text_len == 1) && (text[0] == '0')) {
        // LOG_I(TAG, "start 0");
        return true;
    }
    if ((text[text_len - 1] == '0') &&
            (text[text_len - 2] > '9') &&
            (text[text_len - 2] < '0')) {
        // LOG_I(TAG, "start 0");
        return true;
    }

    return false;
}

bool Player::isStartNum(void)
{
    const char *text = lv_label_get_text(formula_label);
    int text_len = strlen(text);

    if ((text[text_len - 1] >= '0') && (text[text_len - 1] <= '9')) {
        // LOG_I(TAG, "start num");
        return true;
    }

    return false;
}

bool Player::isStartPercent(void)
{
    const char *text = lv_label_get_text(formula_label);
    int text_len = strlen(text);

    if (text[text_len - 1] == '%') {
        // LOG_I(TAG, "start %");
        return true;
    }

    return false;
}

bool Player::isLegalDot(void)
{
    const char *text = lv_label_get_text(formula_label);
    int text_len = strlen(text);

    while (text_len-- > 0) {
        if (text[text_len] == '.') {
            // LOG_I(TAG, "illegal dot");
            return false;
        } else if ((text[text_len] < '0') ||
                   (text[text_len] > '9')) {
            return true;
        }
    }

    return true;
}

double Player::calculate(const char *input)
{
    vector<double> stk;
    int input_len = strlen(input);
    double num = 0;
    bool dot_flag = false;
    int dot_len = 0;
    char pre_sign = '+';

    for (int i = 0; i < input_len; i++) {
        if (input[i] == '.') {
            dot_flag = true;
            dot_len = 0;
        } else if (isdigit(input[i])) {
            if (!dot_flag) {
                num = num * 10 + input[i] - '0';
            } else {
                num += (input[i] - '0') / pow(10.0, ++dot_len);
            }
        } else if (input[i] == '%') {
            num /= 100.0;
        } else if (i != input_len - 1) {
            dot_flag = false;
            dot_len = 0;
            switch (pre_sign) {
            case '+':
                stk.push_back(num);
                break;
            case '-':
                stk.push_back(-num);
                break;
            case 'x':
                stk.back() *= num;
                break;
            default:
                if (num != 0) {
                    stk.back() /= num;
                } else {
                    return 0;
                }
            }
            num = 0;
            pre_sign = input[i];
        }

        if (i == input_len - 1) {
            switch (pre_sign) {
            case '+':
                stk.push_back(num);
                break;
            case '-':
                stk.push_back(-num);
                break;
            case 'x':
                stk.back() *= num;
                break;
            default:
                if (num != 0) {
                    stk.back() /= num;
                } else {
                    return 0;
                }
            }
            num = 0;
        }
    }

    for (int i = 0; i < stk.size(); i++) {
        num += stk.at(i);
    }
    // LOG_I(TAG, "cal: %s = %f", input, num);

    return num;
}

void Player::keyboard_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    Player *app = (Player *)lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        int btn_id = lv_btnmatrix_get_selected_btn(app->keyboard);
        bool calculate_flag = false;
        bool equal_flag = false;
        double res_num;
        char res_str[32];
        char history_str[32];

        if (lv_obj_get_style_text_font(app->formula_label, 0) == LABEL_FONT_SMALL) {
            lv_obj_set_style_text_font(app->formula_label, LABEL_FONT_BIG, 0);
            lv_obj_set_style_text_font(app->result_label, LABEL_FONT_SMALL, 0);
        }

        switch (btn_id) {
        // "C"
        case 0:
            lv_label_set_text(app->formula_label, "0");
            app->formula_len = 1;
            calculate_flag = true;
            break;
        // "<"
        case 3:
            if ((app->formula_len == 1) && app->isStartZero()) {
                break;
            }
            lv_label_cut_text(
                app->formula_label,
                --(app->formula_len),
                1
            );
            if (app->formula_len == 0) {
                lv_label_set_text(app->formula_label, "0");
                app->formula_len = 1;
            }
            calculate_flag = true;
            break;
        // "="
        case 18:
            calculate_flag = true;
            equal_flag = true;
            break;
        // "/,X,-,+,%"
        case 1:
        case 2:
        case 7:
        case 11:
        case 15:
            // Need number or percent
            if (app->isStartPercent() ||
                    app->isStartNum()) {
                // Firstly input "+, -"
                if (((btn_id == 7) || (btn_id == 11)) &&
                        (app->isStartZero())) {
                    // Remove "0"
                    lv_label_cut_text(
                        app->formula_label,
                        --(app->formula_len),
                        1
                    );
                }
                lv_label_ins_text(
                    app->formula_label,
                    app->formula_len++,
                    lv_btnmatrix_get_btn_text(app->keyboard, btn_id)
                );
                if (btn_id == 15) {
                    calculate_flag = true;
                }
            }
            break;
        // "1234567890"
        case 4:
        case 5:
        case 6:
        case 8:
        case 9:
        case 10:
        case 12:
        case 13:
        case 14:
        case 16:
            // Remove First "0"
            if (app->isStartZero()) {
                lv_label_cut_text(
                    app->formula_label,
                    --(app->formula_len),
                    1
                );
            }
            // Except "%"
            if (!app->isStartPercent()) {
                lv_label_ins_text(
                    app->formula_label,
                    app->formula_len++,
                    lv_btnmatrix_get_btn_text(app->keyboard, btn_id)
                );
                calculate_flag = true;
            }
            break;
        // "."
        case 17:
            if (app->isLegalDot() && app->isStartNum()) {
                lv_label_ins_text(
                    app->formula_label,
                    app->formula_len++,
                    "."
                );
            }
            break;
        default:
            break;
        }

        if (calculate_flag) {
            lv_obj_set_style_text_font(app->formula_label, LABEL_FONT_BIG, 0);

            res_num = app->calculate(lv_label_get_text(app->formula_label));
            if (int(res_num) == res_num) {
                snprintf(res_str, sizeof(res_str) - 1, "%ld", long(res_num));
            } else {
                // Limit decimal places to fit small screens
                snprintf(res_str, sizeof(res_str) - 1, "%.3f", res_num);
                // Remove trailing zeros
                char *end = res_str + strlen(res_str) - 1;
                while (end > res_str && *end == '0') {
                    *end-- = '\0';
                }
                if (end > res_str && *end == '.') {
                    *end = '\0';
                }
            }
            lv_label_set_text_fmt(app->result_label, "= %s", res_str);
            lv_obj_set_style_text_font(app->result_label, LABEL_FONT_SMALL, 0);
        }

        if (equal_flag) {
            lv_obj_set_style_text_font(app->result_label, LABEL_FONT_BIG, 0);

            snprintf(history_str, sizeof(history_str) - 1, "\n%s = %s ", lv_label_get_text(app->formula_label), res_str);
            lv_textarea_set_cursor_pos(app->history_label, strlen(lv_textarea_get_text(app->history_label)));
            lv_textarea_add_text(app->history_label, history_str);

            lv_label_set_text_fmt(app->formula_label, "%s", res_str);
            lv_obj_set_style_text_font(app->formula_label, LABEL_FONT_SMALL, 0);
            app->formula_len = strlen(res_str);
        }
    }
}

}
