/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// Coze 聊天应用实现文件
// 该文件实现了与 Coze AI 平台的语音聊天功能，包括语音识别、文本转语音、函数调用等

#include <mutex>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_gmf_element.h"
#include "esp_gmf_oal_sys.h"
#include "esp_gmf_oal_thread.h"
#include "esp_gmf_oal_mem.h"
#include "esp_coze_chat.h"
#include "esp_coze_utils.h"
#include "http_client_request.h"
#include "boost/thread.hpp"
#include "private/esp_brookesia_ai_agent_utils.hpp"
#include "audio_processor.h"
#include "function_calling.hpp"
#include "coze_chat_app.hpp"

// 语音播放超时时间（毫秒）
#define SPEAKING_TIMEOUT_MS         (2000)
// 语音播放结束后的延迟时间（毫秒）
#define SPEAKING_MUTE_DELAY_MS      (2000)

// 音频录制读取缓冲区大小
#define AUDIO_RECORDER_READ_SIZE    (1024)

// 中断重试次数
#define COZE_INTERRUPT_TIMES        (20)
// 中断重试间隔（毫秒）
#define COZE_INTERRUPT_INTERVAL_MS  (100)

using namespace esp_brookesia::ai_framework;

// Coze 聊天状态结构体
struct coze_chat_t {
    esp_coze_chat_handle_t  chat;           // Coze 聊天句柄
    std::recursive_mutex    chat_mutex;     // 聊天状态互斥锁
    bool                    chat_start;     // 聊天是否已开始
    bool                    chat_pause;     // 聊天是否暂停
    bool                    chat_sleep;     // 聊天是否休眠
    bool                    speaking;       // 是否正在播放语音
    bool                    wakeup;         // 是否处于唤醒状态
    bool                    wakeup_start;   // 是否开始唤醒
    bool                    websocket_connected; // WebSocket 是否已连接
    esp_timer_handle_t      speaking_timeout_timer; // 语音播放超时定时器
    esp_gmf_oal_thread_t    read_thread;    // 音频数据读取线程
    esp_gmf_oal_thread_t    btn_thread;     // 按键处理线程
    QueueHandle_t           btn_evt_q;      // 按键事件队列
};

static struct coze_chat_t coze_chat = {};  // 全局 Coze 聊天状态
// Coze 授权服务器 URL
static const char *coze_authorization_url = "https://api.coze.cn/api/permission/oauth2/token";

// 信号定义 - 用于通知不同模块状态变化
boost::signals2::signal<void(const std::string &emoji)> coze_chat_emoji_signal;     // 表情信号
boost::signals2::signal<void(bool is_speaking)> coze_chat_speaking_signal;          // 语音播放状态信号
boost::signals2::signal<void(void)> coze_chat_response_signal;                      // 响应信号
boost::signals2::signal<void(bool is_wake_up)> coze_chat_wake_up_signal;            // 唤醒状态信号
boost::signals2::signal<void(void)> coze_chat_websocket_disconnected_signal;        // WebSocket 断连信号

// CozeChatAgentInfo 类的调试输出方法
void CozeChatAgentInfo::dump() const
{
    ESP_UTILS_LOGI(
        "\n{ChatInfo}:\n"
        "\t-session_name: %s\n"
        "\t-device_id: %s\n"
        "\t-app_id: %s\n"
        "\t-user_id: %s\n"
        "\t-public_key: %s\n"
        "\t-private_key: %s\n"
        "\t-custom_consumer: %s\n",
        session_name.c_str(), device_id.c_str(), app_id.c_str(), user_id.c_str(), public_key.c_str(),
        private_key.c_str(), custom_consumer.c_str()
    );
}

// 检查 CozeChatAgentInfo 是否有效
bool CozeChatAgentInfo::isValid() const
{
    return !session_name.empty() && !device_id.empty() && !user_id.empty() && !app_id.empty() &&
           !public_key.empty() && !private_key.empty();
}

// CozeChatRobotInfo 类的调试输出方法
void CozeChatRobotInfo::dump() const
{
    ESP_UTILS_LOGI(
        "\n{RobotInfo}:\n"
        "\t-name: %s\n"
        "\t-bot_id: %s\n"
        "\t-voice_id: %s\n"
        "\t-description: %s\n",
        name.c_str(), bot_id.c_str(), voice_id.c_str(), description.c_str()
    );
}

// 检查 CozeChatRobotInfo 是否有效
bool CozeChatRobotInfo::isValid() const
{
    return !name.empty() && !bot_id.empty() && !voice_id.empty() && !description.empty();
}


/**
 * @brief 改变语音播放状态
 * @param is_speaking 是否正在播放语音
 * @param force 是否强制更改状态
 */
static void change_speaking_state(bool is_speaking, bool force = false)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    esp_err_t ret = ESP_OK;
    // std::unique_lock<std::recursive_mutex> lock(coze_chat.chat_mutex);

    // 如果状态没有变化且非强制模式，只重启超时定时器
    if ((is_speaking == coze_chat.speaking) && !force) {
        if (is_speaking) {
            ret = esp_timer_restart(coze_chat.speaking_timeout_timer, SPEAKING_TIMEOUT_MS * 1000);
            if (ret != ESP_OK) {
                ESP_UTILS_LOGE("Restart speaking timeout timer failed(%s)", esp_err_to_name(ret));
            }
        }
        return;
    }

    ESP_UTILS_LOGI("change_speaking_state: %d, force: %d", is_speaking, force);

    if (is_speaking) {
        // 开始播放时保持唤醒状态
        if (esp_gmf_afe_keep_awake(audio_processor_get_afe_handle(), true) != ESP_OK) {
            ESP_UTILS_LOGE("Keep awake failed");
        }
        // 启动语音播放超时定时器
        if (!esp_timer_is_active(coze_chat.speaking_timeout_timer)) {
            ret = esp_timer_start_once(coze_chat.speaking_timeout_timer, SPEAKING_TIMEOUT_MS * 1000);
            if (ret != ESP_OK) {
                ESP_UTILS_LOGE("Start speaking timeout timer failed(%s)", esp_err_to_name(ret));
            }
        }
    } else {
        // 停止播放时取消保持唤醒
        if (esp_gmf_afe_keep_awake(audio_processor_get_afe_handle(), false) != ESP_OK) {
            ESP_UTILS_LOGE("Keep awake failed");
        }
        // 停止语音播放超时定时器
        if (esp_timer_is_active(coze_chat.speaking_timeout_timer)) {
            ret = esp_timer_stop(coze_chat.speaking_timeout_timer);
            if (ret != ESP_OK) {
                ESP_UTILS_LOGE("Stop speaking timeout timer failed(%s)", esp_err_to_name(ret));
            }
        }
    }
    coze_chat.speaking = is_speaking;
    // lock.unlock();

    // 发送语音播放状态信号
    coze_chat_speaking_signal(is_speaking);
}

/**
 * @brief 改变唤醒状态
 * @param is_wakeup 是否处于唤醒状态
 * @param force 是否强制更改状态
 */
static void change_wakeup_state(bool is_wakeup, bool force = false)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // std::unique_lock<std::recursive_mutex> lock(coze_chat.chat_mutex);

    // 如果状态没有变化且非强制模式，直接返回
    if ((is_wakeup == coze_chat.wakeup) && !force) {
        return;
    }

    ESP_UTILS_LOGI("change_wakeup_state: %d, force: %d", is_wakeup, force);

    coze_chat.wakeup = is_wakeup;
    // lock.unlock();

    // 发送唤醒状态信号
    coze_chat_wake_up_signal(is_wakeup);
}

/**
 * @brief 音频事件回调函数
 * 处理来自 Coze 聊天的各种音频相关事件
 * @param event 事件类型
 * @param data 事件数据
 * @param ctx 上下文指针
 */
static void audio_event_callback(esp_coze_chat_event_t event, char *data, void *ctx)
{
    if (event == ESP_COZE_CHAT_EVENT_CHAT_SPEECH_STARTED) {
        ESP_UTILS_LOGI("chat start");
        coze_chat.wakeup_start = false;
    } else if (event == ESP_COZE_CHAT_EVENT_CHAT_SPEECH_STOPED) {
        ESP_UTILS_LOGI("chat stop");
        // change_speaking_state(true);
    } else if (event == ESP_COZE_CHAT_EVENT_CHAT_COMPLETED) {
        // 聊天完成后延迟停止语音播放
        boost::thread([&]() {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(SPEAKING_MUTE_DELAY_MS));
            change_speaking_state(false);
        }).detach();
        ESP_UTILS_LOGI("chat complete");
    } else if (event == ESP_COZE_CHAT_EVENT_CHAT_CUSTOMER_DATA) {
        // 处理自定义数据（函数调用）
        ESP_UTILS_LOGI("Customer data: %s", data);

        // 解析 JSON 数据并调用相应函数
        cJSON *json_data = cJSON_Parse(data);
        if (json_data == NULL) {
            ESP_UTILS_LOGE("Failed to parse JSON data");
            return;
        }
        // 调试输出 JSON 内容
        cJSON *json_item = NULL;
        cJSON_ArrayForEach(json_item, json_data) {
            char *key = json_item->string;
            char *value = cJSON_Print(json_item);
            if (key && value) {
                ESP_UTILS_LOGI("Key: %s, Value: %s", key, value);
                cJSON_free(value);
            }
        }

        // 提取函数调用信息
        cJSON *data_json = cJSON_GetObjectItem(json_data, "data");
        if (data_json == NULL) {
            ESP_UTILS_LOGE("No data found in JSON data");
            cJSON_Delete(json_data);
            return;
        }

        cJSON *required_action = cJSON_GetObjectItem(data_json, "required_action");
        if (required_action == NULL) {
            ESP_UTILS_LOGE("No required_action found in JSON data");
            cJSON_Delete(json_data);
            return;
        }

        cJSON *submit_tool_outputs = cJSON_GetObjectItem(required_action, "submit_tool_outputs");
        if (submit_tool_outputs == NULL) {
            ESP_UTILS_LOGE("No submit_tool_outputs found in JSON data");
            cJSON_Delete(json_data);
            return;
        }

        cJSON *tool_calls = cJSON_GetObjectItem(submit_tool_outputs, "tool_calls");
        if (tool_calls == NULL || !cJSON_IsArray(tool_calls)) {
            ESP_UTILS_LOGE("No tool_calls found or tool_calls is not an array");
            cJSON_Delete(json_data);
            return;
        }

        cJSON *first_tool_call = cJSON_GetArrayItem(tool_calls, 0);
        if (first_tool_call == NULL) {
            ESP_UTILS_LOGE("No first tool call found in tool_calls");
            cJSON_Delete(json_data);
            return;
        }

        char *function_str = cJSON_Print(first_tool_call);
        if (function_str) {
            ESP_UTILS_LOGI("Function JSON: %s", function_str);
            free(function_str);
        } else {
            ESP_UTILS_LOGE("Failed to print function JSON");
        }

        // 调用函数
        FunctionDefinitionList::requestInstance().invokeFunction(first_tool_call);

        cJSON_Delete(json_data);
    } else if (event == ESP_COZE_CHAT_EVENT_CHAT_SUBTITLE_EVENT) {
        // 处理字幕事件，提取表情符号
        if (strncmp(data, "（", 3) == 0 && strncmp(data + strlen(data) - 3, "）", 3) == 0) {
            std::string emoji_str(data + 3);
            emoji_str = emoji_str.substr(0, emoji_str.length() - 3);
            if (emoji_str.front() == ':' && emoji_str.back() == ':') {
                emoji_str = emoji_str.substr(1, emoji_str.length() - 2);
                ESP_UTILS_LOGI("Emoji: %s\n", emoji_str.c_str());
                coze_chat_emoji_signal(emoji_str);
            }
        }
    }
}

/**
 * @brief WebSocket 事件回调函数
 * 处理 WebSocket 连接状态变化
 * @param event WebSocket 事件指针
 */
static void websocket_event_callback(esp_coze_ws_event_t *event)
{
    switch (event->event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_UTILS_LOGI("Websocket connected");
        coze_chat.websocket_connected = true;
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
    case WEBSOCKET_EVENT_ERROR:
        ESP_UTILS_LOGE("Websocket disconnected or error");
        coze_chat.websocket_connected = false;
        coze_chat_websocket_disconnected_signal();
        break;
    default:
        break;
    }
}

/**
 * @brief 音频数据回调函数
 * 处理从 Coze 服务接收到的音频数据
 * @param data 音频数据指针
 * @param len 音频数据长度
 * @param ctx 上下文指针
 */
static void audio_data_callback(char *data, int len, void *ctx)
{
    ESP_UTILS_LOGD("audio_data_callback");
    // 只有在未暂停、未休眠且正在播放时才播放音频
    if (!coze_chat.chat_pause && !coze_chat.chat_sleep && coze_chat.speaking) {
        audio_playback_feed_data((uint8_t *)data, len);
    }
    // 如果不是在唤醒开始阶段且未暂停未休眠，则设置为播放状态
    if (!coze_chat.wakeup_start && !coze_chat.chat_pause && !coze_chat.chat_sleep) {
        change_speaking_state(true);
    }
}

/**
 * @brief 生成随机字符串
 * @param output 输出缓冲区
 * @param length 字符串长度
 */
void generate_random_string(char *output, size_t length)
{
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t charset_size = sizeof(charset) - 1;
    for (size_t i = 0; i < length; i++) {
        int key = esp_random() % charset_size;
        output[i] = charset[key];
    }
    output[length] = '\0';
}

/**
 * @brief 获取 Coze 访问令牌
 * 使用 JWT 认证方式从 Coze 服务获取访问令牌
 * @param agent_info 代理信息，包含认证所需的密钥
 * @return 成功返回访问令牌字符串，失败返回 NULL
 */
static char *coze_get_access_token(const CozeChatAgentInfo &agent_info)
{
    // 构建 JWT payload（有效载荷）
    cJSON *payload_json = cJSON_CreateObject();
    if (!payload_json) {
        ESP_UTILS_LOGE("Failed to create payload_json");
        return NULL;
    }
    
    // 生成随机字符串作为 JWT ID
    char random_str[33] = {0};
    generate_random_string(random_str, 32);
    time_t now = time(NULL);
    
    // 添加 JWT 标准声明
    cJSON_AddStringToObject(payload_json, "iss", agent_info.app_id.c_str());     // 发行者
    cJSON_AddStringToObject(payload_json, "aud", "api.coze.cn");                 // 受众
    cJSON_AddNumberToObject(payload_json, "iat", now);                           // 发行时间
    cJSON_AddNumberToObject(payload_json, "exp", now + 6000);                    // 过期时间
    cJSON_AddStringToObject(payload_json, "jti", random_str);                    // JWT ID
    cJSON_AddStringToObject(payload_json, "session_name", agent_info.session_name.c_str());
    
    // 添加会话上下文信息
    cJSON *session_context_json = cJSON_CreateObject();
    cJSON *device_info_json = cJSON_CreateObject();
    cJSON_AddStringToObject(device_info_json, "device_id", agent_info.device_id.c_str());
    cJSON_AddStringToObject(device_info_json, "custom_consumer", agent_info.custom_consumer.c_str());
    cJSON_AddItemToObject(session_context_json, "device_info", device_info_json);
    cJSON_AddItemToObject(payload_json, "session_context", session_context_json);

    // 将 JWT payload 转换为字符串
    char *payload_str = cJSON_PrintUnformatted(payload_json);
    if (!payload_str) {
        ESP_UTILS_LOGE("Failed to convert payload_json to string");
        cJSON_Delete(payload_json);
        return NULL;
    }
    ESP_UTILS_LOGI("JWT payload string created, length: %zu", strlen(payload_str));
    ESP_UTILS_LOGD("JWT payload content: %s", payload_str);
    
    // 打印格式化的 payload（用于调试）
    char *formatted_payload_str = cJSON_Print(payload_json);
    if (formatted_payload_str) {
        ESP_UTILS_LOGD("Formatted JWT payload:\n%s", formatted_payload_str);
        free(formatted_payload_str);
    }

    // 创建 JWT 令牌
    ESP_UTILS_LOGI("Creating JWT token with public key length: %zu, private key length: %zu", 
                   agent_info.public_key.length(), agent_info.private_key.length());
    char *jwt = coze_jwt_create_handler(
                    agent_info.public_key.c_str(), payload_str, (const uint8_t *)agent_info.private_key.c_str(),
                    strlen(agent_info.private_key.c_str())
                );
    cJSON_Delete(payload_json);
    free(payload_str);

    if (!jwt) {
        ESP_UTILS_LOGE("Failed to create JWT token");
        return NULL;
    }
    ESP_UTILS_LOGI("JWT token created successfully, length: %zu", strlen(jwt));
    ESP_UTILS_LOGD("JWT token: %s", jwt);

    // 创建 Authorization 头
    char *authorization = (char *)calloc(1, strlen(jwt) + 16);
    if (!authorization) {
        ESP_UTILS_LOGE("Failed to allocate memory for authorization header");
        free(jwt);
        return NULL;
    }
    sprintf(authorization, "Bearer %s", jwt);
    ESP_UTILS_LOGI("Authorization header created, length: %zu", strlen(authorization));
    ESP_UTILS_LOGD("Authorization header: %s", authorization);

    // 创建 HTTP 请求体
    cJSON *http_req_json = cJSON_CreateObject();
    if (!http_req_json) {
        ESP_UTILS_LOGE("Failed to create HTTP request JSON object");
        free(jwt);
        free(authorization);
        return NULL;
    }
    cJSON_AddNumberToObject(http_req_json, "duration_seconds", 86399);
    cJSON_AddStringToObject(http_req_json, "grant_type", "urn:ietf:params:oauth:grant-type:jwt-bearer");
    
    char *http_req_json_str = cJSON_PrintUnformatted(http_req_json);
    if (!http_req_json_str) {
        ESP_UTILS_LOGE("Failed to convert HTTP request JSON to string");
        free(jwt);
        free(authorization);
        cJSON_Delete(http_req_json);
        return NULL;
    }
    ESP_UTILS_LOGI("HTTP request body created, length: %zu", strlen(http_req_json_str));
    ESP_UTILS_LOGD("HTTP request body: %s", http_req_json_str);

    // 准备 HTTP 请求头
    http_req_header_t header[] = {
        {"Content-Type", "application/json"},
        {"Authorization", authorization},
        {NULL, NULL}
    };
    ESP_UTILS_LOGI("HTTP headers prepared, sending POST request to: %s", coze_authorization_url);

    // 发送 HTTP POST 请求获取访问令牌
    http_response_t response = {0};
    esp_err_t ret = http_client_post(coze_authorization_url, header, http_req_json_str, &response);
    if (ret != ESP_OK) {
        ESP_UTILS_LOGE("HTTP POST failed, error code: %s", esp_err_to_name(ret));
        // 释放已分配的内存
        free(jwt);
        free(authorization);
        cJSON_Delete(http_req_json);
        free(http_req_json_str);
        return NULL;
    }

    char *access_token = NULL;
    if (response.body) {
        ESP_UTILS_LOGI("HTTP response body length: %zu bytes", strlen(response.body));
        ESP_UTILS_LOGI("HTTP response body content: %s", response.body);

        cJSON *root = cJSON_Parse(response.body);
        if (root) {
            ESP_UTILS_LOGI("Successfully parsed JSON response");
            
            // 解析访问令牌
            cJSON *access_token_item = cJSON_GetObjectItem(root, "access_token");
            if (cJSON_IsString(access_token_item) && access_token_item->valuestring != NULL) {
                ESP_UTILS_LOGI("Successfully extracted access_token from response");
                ESP_UTILS_LOGD("access_token value: %s", access_token_item->valuestring);
                access_token = strdup(access_token_item->valuestring);
            } else {
                ESP_UTILS_LOGE("access_token is invalid or not exist in response");
            }

            // 解析过期时间
            cJSON *expires_in_item = cJSON_GetObjectItem(root, "expires_in");
            if (cJSON_IsNumber(expires_in_item)) {
                ESP_UTILS_LOGI("Token expires in: %d seconds", expires_in_item->valueint);
            } else {
                ESP_UTILS_LOGW("expires_in field not found or invalid in response");
            }

            // 解析令牌类型
            cJSON *token_type_item = cJSON_GetObjectItem(root, "token_type");
            if (cJSON_IsString(token_type_item)) {
                ESP_UTILS_LOGI("token_type: %s\n", token_type_item->valuestring);
            }

            cJSON_Delete(root);
        } else {
            ESP_UTILS_LOGE("Failed to parse JSON response, invalid JSON format");
        }
    } else {
        ESP_UTILS_LOGE("HTTP response body is empty or NULL");
    }

    // 释放内存并打印内存释放信息
    ESP_UTILS_LOGD("Cleaning up memory allocations");
    free(jwt);
    free(authorization);
    cJSON_Delete(http_req_json);
    free(http_req_json_str);
    if (response.body) {
        free(response.body);
    }

    if (access_token) {
        ESP_UTILS_LOGI("Access token acquisition successful, token length: %zu", strlen(access_token));
    } else {
        ESP_UTILS_LOGE("Access token acquisition failed");
    }

    return access_token;
}

/**
 * @brief 音频录制事件回调函数
 * 处理来自音频前端的各种事件（唤醒、VAD 等）
 * @param event 事件指针
 * @param ctx 上下文指针
 */
static void recorder_event_callback_fn(void *event, void *ctx)
{
    if (!coze_chat.chat_start || coze_chat.chat_pause) {
        ESP_UTILS_LOGW("chat is not started or paused, skip SAFE event");
        return;
    }

    esp_gmf_afe_evt_t *afe_evt = (esp_gmf_afe_evt_t *)event;
    switch (afe_evt->type) {
    case ESP_GMF_AFE_EVT_WAKEUP_START: {
        ESP_UTILS_LOGI("wakeup start");
        // 如果 WebSocket 已连接且未休眠，中断当前聊天
        if (coze_chat.websocket_connected && !coze_chat.chat_sleep) {
            coze_chat_app_interrupt();
        }
        change_speaking_state(false);
        change_wakeup_state(true);
        coze_chat.wakeup_start = true;
        coze_chat_response_signal();
        break;
    }
    case ESP_GMF_AFE_EVT_WAKEUP_END:
        ESP_UTILS_LOGI("wakeup end");
        change_speaking_state(false);
        change_wakeup_state(false);
        break;
    case ESP_GMF_AFE_EVT_VAD_START:
        // 语音活动检测开始，保持唤醒状态
        if (esp_gmf_afe_keep_awake(audio_processor_get_afe_handle(), true) != ESP_OK) {
            ESP_UTILS_LOGE("Keep awake failed");
        }
        ESP_UTILS_LOGI("vad start");
        break;
    case ESP_GMF_AFE_EVT_VAD_END:
        // 语音活动检测结束，取消保持唤醒
        if (esp_gmf_afe_keep_awake(audio_processor_get_afe_handle(), false) != ESP_OK) {
            ESP_UTILS_LOGE("Keep awake failed");
        }
        ESP_UTILS_LOGI("vad end");
        break;
    case ESP_GMF_AFE_EVT_VCMD_DECT_TIMEOUT:
        ESP_UTILS_LOGI("vcmd detect timeout");
        break;
    default: {
        // TODO: 语音命令检测
        // esp_gmf_afe_vcmd_info_t *info = event->event_data;
        // ESP_UTILS_LOGW("Command %d, phrase_id %d, prob %f, str: %s", event->type, info->phrase_id, info->prob, info->str);
    }
    }
}

/**
 * @brief 音频数据读取任务
 * 持续从音频录制器读取数据并发送到 Coze 聊天
 * @param pv 任务参数（coze_chat_t 结构体指针）
 */
static void audio_data_read_task(void *pv)
{
    coze_chat_t *coze_chat = (coze_chat_t *)pv;

    uint8_t *data = (uint8_t *)esp_gmf_oal_calloc(1, AUDIO_RECORDER_READ_SIZE);
    int ret = 0;
    while (true) {
        ret = audio_recorder_read_data(data, AUDIO_RECORDER_READ_SIZE);
        // 只有在聊天已开始、处于唤醒状态、未暂停、未休眠且未播放时才发送音频数据
        if (coze_chat->chat_start && coze_chat->wakeup && !coze_chat->chat_pause && !coze_chat->chat_sleep && !coze_chat->speaking) {
            esp_coze_chat_send_audio_data(coze_chat->chat, (char *)data, ret);
        }
        // heap_caps_check_integrity_all(true); // 内存完整性检查（调试用）
    }
}

/**
 * @brief 打开音频管道
 * 初始化音频录制和播放功能
 */
static void audio_pipe_open(void)
{
    audio_recorder_open(recorder_event_callback_fn, NULL);
    audio_playback_open();
    audio_playback_run();
}

// 注释掉的音频管道关闭函数
// static void audio_pipe_close(void)
// {
//     audio_playback_stop();
//     audio_playback_close();
//     audio_recorder_close();
// }

/**
 * @brief 初始化 Coze 聊天应用
 * 创建必要的定时器、音频管道和线程
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t coze_chat_app_init(void)
{
    ESP_UTILS_LOG_TRACE_GUARD();
    ESP_UTILS_LOGI("Initializing Coze chat application...");

    // 创建语音播放超时定时器
    ESP_UTILS_LOGI("Creating speaking timeout timer...");
    esp_timer_create_args_t timer_args = {
        .callback = [](void *arg)
        {
            ESP_UTILS_LOGI("Speaking timeout triggered, stopping speech");
            boost::thread([&]() {
                change_speaking_state(false);
            }).detach();
            ESP_UTILS_LOGI("Speaking timeout handler completed");
        },
        .arg = &coze_chat,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "speaking_timeout",
        .skip_unhandled_events = false
    };
    esp_err_t ret = esp_timer_create(&timer_args, &coze_chat.speaking_timeout_timer);
    if (ret != ESP_OK) {
        ESP_UTILS_LOGE("Failed to create speaking timeout timer: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_UTILS_LOGI("Speaking timeout timer created successfully");

    // 打开音频管道
    ESP_UTILS_LOGI("Opening audio pipeline...");
    audio_pipe_open();
    ESP_UTILS_LOGI("Audio pipeline opened successfully");

    // 创建音频数据读取线程
    ESP_UTILS_LOGI("Creating audio data read thread...");
    ret = esp_gmf_oal_thread_create(
        &coze_chat.read_thread, "audio_data_read", audio_data_read_task, (void *)&coze_chat, 3096, 12, true, 1
    );
    if (ret != ESP_OK) {
        ESP_UTILS_LOGE("Failed to create audio data read thread: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_UTILS_LOGI("Audio data read thread created successfully");

    ESP_UTILS_LOGI("Coze chat application initialization completed successfully");
    return ESP_OK;
}

/**
 * @brief 启动 Coze 聊天应用
 * 使用提供的代理信息和机器人信息初始化并启动聊天服务
 * @param agent_info 代理信息，包含认证密钥等
 * @param robot_info 机器人信息，包含机器人 ID、语音 ID 等
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t coze_chat_app_start(const CozeChatAgentInfo &agent_info, const CozeChatRobotInfo &robot_info)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 打印传入的配置信息
    ESP_UTILS_LOGI("Starting Coze chat app with following configuration:");
    agent_info.dump();
    robot_info.dump();

    // 验证配置信息有效性
    if (!agent_info.isValid()) {
        ESP_UTILS_LOGE("Agent info is invalid, cannot start chat app");
        return ESP_FAIL;
    }
    if (!robot_info.isValid()) {
        ESP_UTILS_LOGE("Robot info is invalid, cannot start chat app");
        return ESP_FAIL;
    }
    ESP_UTILS_LOGI("Configuration validation passed");

    // 获取访问令牌
    ESP_UTILS_LOGI("Attempting to get access token...");
    char *token_str = NULL;
    token_str = coze_get_access_token(agent_info);
    if (token_str == NULL) {
        ESP_UTILS_LOGE("Failed to get access token, cannot proceed");
        return ESP_FAIL;
    }
    ESP_UTILS_LOGI("Access token obtained successfully");

    // 配置 Coze 聊天参数
    ESP_UTILS_LOGI("Configuring Coze chat parameters...");
    esp_coze_chat_config_t chat_config = ESP_COZE_CHAT_DEFAULT_CONFIG();
    chat_config.enable_subtitle = true;                         // 启用字幕
    chat_config.subscribe_event = (const char *[]) {            // 订阅的事件
        "conversation.chat.requires_action", NULL
    };
    chat_config.user_id = agent_info.user_id.c_str();          // 用户 ID
    chat_config.bot_id = robot_info.bot_id.c_str();            // 机器人 ID
    chat_config.voice_id = robot_info.voice_id.c_str();        // 语音 ID
    chat_config.access_token = token_str;                       // 访问令牌
    chat_config.uplink_audio_type = ESP_COZE_CHAT_AUDIO_TYPE_G711A; // 上行音频格式
    chat_config.audio_callback = audio_data_callback;          // 音频数据回调
    chat_config.event_callback = audio_event_callback;         // 事件回调
    chat_config.ws_event_callback = websocket_event_callback;  // WebSocket 事件回调
    
    ESP_UTILS_LOGI("Chat configuration completed:");
    ESP_UTILS_LOGI("  - User ID: %s", chat_config.user_id);
    ESP_UTILS_LOGI("  - Bot ID: %s", chat_config.bot_id);
    ESP_UTILS_LOGI("  - Voice ID: %s", chat_config.voice_id);
    ESP_UTILS_LOGI("  - Subtitle enabled: %s", chat_config.enable_subtitle ? "true" : "false");
    ESP_UTILS_LOGI("  - Audio type: G711A");
    // chat_config.websocket_buffer_size = 4096;               // WebSocket 缓冲区大小
    // chat_config.mode = ESP_COZE_CHAT_NORMAL_MODE;           // 聊天模式

    ESP_UTILS_LOGI("Initializing Coze chat with mutex lock...");
    std::lock_guard lock(coze_chat.chat_mutex);
    
    // 初始化 Coze 聊天
    esp_err_t ret = esp_coze_chat_init(&chat_config, &coze_chat.chat);
    ESP_UTILS_CHECK_FALSE_RETURN(ret == ESP_OK, ret, "esp_coze_chat_init failed(%s)", esp_err_to_name(ret));
    ESP_UTILS_LOGI("Coze chat initialization successful");

    // 设置函数调用配置
    ESP_UTILS_LOGI("Setting up function call configuration...");
    static auto func_call = FunctionDefinitionList::requestInstance().getJson();
    ESP_UTILS_LOGI("Function call JSON length: %zu", func_call.length());
    ESP_UTILS_LOGD("Function call JSON content: %s", func_call.c_str());

    esp_coze_parameters_kv_t param[] = {
        {"func_call", func_call.c_str()},
        {NULL, NULL}
    };
    ret = esp_coze_set_chat_config_parameters(coze_chat.chat, param);
    ESP_UTILS_CHECK_FALSE_RETURN(ret == ESP_OK, ret, "esp_coze_set_chat_config_parameters failed(%s)", esp_err_to_name(ret));
    ESP_UTILS_LOGI("Function call configuration set successfully");
    
    // 启动聊天
    ESP_UTILS_LOGI("Starting Coze chat service...");
    ret = esp_coze_chat_start(coze_chat.chat);
    ESP_UTILS_CHECK_FALSE_RETURN(ret == ESP_OK, ret, "esp_coze_chat_start failed(%s)", esp_err_to_name(ret));

    coze_chat.chat_start = true;
    ESP_UTILS_LOGI("Coze chat service started successfully");

    free(token_str);
    ESP_UTILS_LOGI("Access token memory freed");

    return ESP_OK;
}

/**
 * @brief 停止 Coze 聊天应用
 * 停止并清理聊天服务
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t coze_chat_app_stop(void)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    std::lock_guard lock(coze_chat.chat_mutex);

    // 停止聊天
    esp_err_t ret = esp_coze_chat_stop(coze_chat.chat);
    ESP_UTILS_CHECK_FALSE_RETURN(ret == ESP_OK, ret, "esp_coze_chat_stop failed(%s)", esp_err_to_name(ret));

    // 清理聊天资源
    ret = esp_coze_chat_deinit(coze_chat.chat);
    ESP_UTILS_CHECK_FALSE_RETURN(ret == ESP_OK, ret, "esp_coze_chat_deinit failed(%s)", esp_err_to_name(ret));
    coze_chat.chat = NULL;

    coze_chat.chat_start = false;

    return ESP_OK;
}

/**
 * @brief 恢复 Coze 聊天应用
 * 从暂停状态恢复聊天功能
 */
void coze_chat_app_resume(void)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    coze_chat.chat_pause = false;
}

/**
 * @brief 暂停 Coze 聊天应用
 * 暂停聊天功能，中断当前对话
 */
void coze_chat_app_pause(void)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 如果 WebSocket 已连接，先中断当前聊天
    if (coze_chat.websocket_connected) {
        coze_chat_app_interrupt();
    }
    // esp_gmf_afe_reset_state(audio_processor_get_afe_handle()); // 重置音频前端状态
    coze_chat.chat_pause = true;
    change_speaking_state(false);
    // change_wakeup_state(false);
}

/**
 * @brief 唤醒 Coze 聊天应用
 * 从休眠状态唤醒聊天功能
 */
void coze_chat_app_wakeup(void)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    coze_chat.chat_sleep = false;
    change_wakeup_state(true);
}

/**
 * @brief 使 Coze 聊天应用进入休眠状态
 * 进入休眠模式，停止所有聊天活动
 */
void coze_chat_app_sleep(void)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 如果 WebSocket 已连接，先中断当前聊天
    if (coze_chat.websocket_connected) {
        coze_chat_app_interrupt();
    }
    coze_chat.chat_sleep = true;
    change_wakeup_state(false);
    change_speaking_state(false);
}

/**
 * @brief 中断 Coze 聊天应用
 * 发送音频取消信号中断当前对话
 */
void coze_chat_app_interrupt(void)
{
    ESP_UTILS_LOG_TRACE_GUARD();

    // 在后台线程中重复发送中断信号
    boost::thread([&]() {
        ESP_UTILS_LOG_TRACE_GUARD();
        for (int i = 0; i < COZE_INTERRUPT_TIMES; i++) {
            {
                std::lock_guard lock(coze_chat.chat_mutex);
                if ((coze_chat.chat == NULL) || !coze_chat.websocket_connected) {
                    break;
                }
                esp_coze_chat_send_audio_cancel(coze_chat.chat);
            }
            boost::this_thread::sleep_for(boost::chrono::milliseconds(COZE_INTERRUPT_INTERVAL_MS));
        }
    }).detach();
}
