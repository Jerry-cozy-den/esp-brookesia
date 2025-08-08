/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <memory>
#include "esp_netif.h"
#include "boost/thread.hpp"
#include "ai_framework/agent/esp_brookesia_ai_agent.hpp"
#include "ai_framework/expression/esp_brookesia_ai_expression.hpp"
#include "assets/esp_brookesia_speaker_assets.h"

namespace esp_brookesia::speaker {

/**
 * @brief AI伙伴数据结构，包含表情系统配置
 */
struct AI_BuddyData {
    struct {
        ai_framework::ExpressionData data;  // 表情系统数据
    } expression;
};

/**
 * @brief AI伙伴类 - 智能音箱的AI助手核心
 * 
 * 功能说明：
 * - 管理AI助手的表情显示（情感表情和系统图标）
 * - 处理语音播放和音频事件
 * - 监控WiFi连接状态
 * - 管理AI对话状态（是否正在说话、暂停等）
 * - 提供单例模式访问
 */
class AI_Buddy {
public:
    /**
     * @brief 表情情感类型枚举
     * 定义AI伙伴可显示的各种情感表情
     */
    enum ExpressionEmotionType {
        ExpressionEmotionTypeNone = ai_framework::Expression::EMOTION_TYPE_NONE,    // 无表情
        ExpressionEmotionAngry = MMAP_EMOTION_EMOTION_ANGRY_284_126_AAF,            // 愤怒
        ExpressionEmotionFastBlink = MMAP_EMOTION_EMOTION_BLINK_FAST_284_126_AAF,   // 快速眨眼
        ExpressionEmotionSlowBlink = MMAP_EMOTION_EMOTION_BLINK_SLOW_284_126_AAF,   // 慢速眨眼
        ExpressionEmotionHappy = MMAP_EMOTION_EMOTION_HAPPY_284_126_AAF,            // 开心
        ExpressionEmotionSad = MMAP_EMOTION_EMOTION_SAD_284_126_AAF,                // 悲伤
        ExpressionEmotionSleep = MMAP_EMOTION_EMOTION_SLEEP_284_126_AAF,            // 睡眠
    };
    
    /**
     * @brief 表情图标类型枚举
     * 定义AI伙伴可显示的各种系统状态图标
     */
    enum ExpressionIconType {
        ExpressionIconTypeNone = ai_framework::Expression::ICON_TYPE_NONE,              // 无图标
        ExpressionIconSystemBrightnessDown = MMAP_ICON_ICON_BRIGHTNESS_DOWN_64_AAF,     // 亮度降低图标
        ExpressionIconSystemBrightnessUp = MMAP_ICON_ICON_BRIGHTNESS_UP_64_AAF,         // 亮度提高图标
        ExpressionIconEmotionConfused = MMAP_ICON_ICON_EMOTION_CONFUSED_64_AAF,         // 困惑表情图标
        ExpressionIconEmotionSleep = MMAP_ICON_ICON_EMOTION_SLEEP_64_AAF,               // 睡眠表情图标
        ExpressionIconEmotionThinking = MMAP_ICON_ICON_EMOTION_THINKING_64_AAF,         // 思考表情图标
        ExpressionIconSystemServerConnected = MMAP_ICON_ICON_SERVER_CONNECTED_64_AAF,   // 服务器已连接图标
        ExpressionIconSystemServerConnecting = MMAP_ICON_ICON_SERVER_CONNECTING_64_AAF, // 服务器连接中图标
        ExpressionIconSystemVolumeDown = MMAP_ICON_ICON_VOLUME_DOWN_64_AAF,             // 音量降低图标
        ExpressionIconSystemVolumeMute = MMAP_ICON_ICON_VOLUME_MUTE_64_AAF,             // 静音图标
        ExpressionIconSystemVolumeUp = MMAP_ICON_ICON_VOLUME_UP_64_AAF,                 // 音量提高图标
        ExpressionIconSystemWifiDisconnected = MMAP_ICON_ICON_WIFI_DISCONNECT_64_AAF,  // WiFi断开图标
    };

    // 禁用拷贝构造和赋值操作，确保单例唯一性
    AI_Buddy(const AI_Buddy &) = delete;
    AI_Buddy(AI_Buddy &&) = delete;
    AI_Buddy &operator=(const AI_Buddy &) = delete;
    AI_Buddy &operator=(AI_Buddy &&) = delete;

    ~AI_Buddy();

    // 启动AI伙伴系统
    bool begin(const AI_BuddyData &data);
    // 恢复AI伙伴运行
    bool resume();
    // 暂停AI伙伴
    bool pause();
    // 删除AI伙伴
    bool del();

    /**
     * @brief 检查AI伙伴是否正在说话
     */
    bool isSpeaking() const
    {
        return _flags.is_speaking;
    }

    /**
     * @brief 检查AI伙伴是否处于暂停状态
     */
    bool isPause() const
    {
        return _flags.is_pause;
    }

    /**
     * @brief 检查WiFi连接是否有效
     */
    bool isWiFiValid() const
    {
        return _flags.is_wifi_connected;
    }

    // 获取AI伙伴单例实例
    static std::shared_ptr<AI_Buddy> requestInstance();
    // 释放AI伙伴单例实例
    static void releaseInstance();

    ai_framework::Expression expression;  // 表情控制对象

private:
    /**
     * @brief 音频事件类型枚举
     * 定义AI伙伴可播放的各种音频类型
     */
    enum class AudioType {
        WifiNeedConnect,        // WiFi需要连接
        WifiConnected,          // WiFi已连接
        WifiDisconnected,       // WiFi已断开
        ServerConnected,        // 服务器已连接
        ServerDisconnected,     // 服务器已断开
        ServerConnecting,       // 服务器连接中
        MicOn,                  // 麦克风开启
        MicOff,                 // 麦克风关闭
        WakeUp,                 // 唤醒
        ResponseLaiLo,          // 回应"来咯"
        ResponseWoZaiTingNe,    // 回应"我在听呢"
        ResponseWoZai,          // 回应"我在"
        ResponseZaiNe,          // 回应"在呢"
        SleepBaiBaiLo,          // 睡眠"拜拜咯"
        SleepHaoDe,             // 睡眠"好的"
        SleepWoTuiXiaLe,        // 睡眠"我退下了"
        SleepXianZheYangLo,     // 睡眠"先这样咯"
        InvalidConfig,          // 无效配置
        Max,
    };
    
    /**
     * @brief 音频事件结构
     */
    struct AudioEvent {
        AudioType type;         // 音频类型
        int repeat_count;       // 重复次数
        int repeat_interval_ms; // 重复间隔（毫秒）
    };
    
    /**
     * @brief 音频处理信息结构
     */
    struct AudioProcessInfo {
        AudioEvent event;           // 音频事件
        int64_t create_time_ms;     // 创建时间
        int64_t last_play_time_ms;  // 上次播放时间
    };
    
    using AudioInfo = std::pair<std::string, int>;                     // 音频信息：文件路径和时长
    using RandomAudios = std::vector<std::pair<float, AudioType>>;     // 随机音频：概率和音频类型

    AI_Buddy() = default;  // 私有构造函数，确保单例

    // 音频事件处理方法
    void sendAudioEvent(const AudioEvent &event);          // 发送音频事件
    void stopAudio(AudioType type);                         // 停止音频播放
    bool processAudioEvent(AudioProcessInfo &info);        // 处理音频事件
    bool play_random_audio(const RandomAudios &audios);    // 播放随机音频
    std::string getAudioName(AudioType type) const;        // 获取音频文件名

    // WiFi事件处理
    bool processOnWiFiEvent(esp_event_base_t event_base, int32_t event_id, void *event_data);

    /**
     * @brief 状态标志位结构
     */
    struct {
        int is_begun: 1;            // 是否已启动
        int is_pause: 1;            // 是否暂停
        int is_wifi_connected: 1;   // WiFi是否连接
        int is_speaking: 1;         // 是否正在说话
    } _flags = {};
    
    std::recursive_mutex _mutex;    // 递归互斥锁

    // AI代理相关
    std::shared_ptr<ai_framework::Agent> _agent;                   // AI代理对象
    std::vector<boost::signals2::connection> _agent_connections;    // AI代理信号连接

    // 音频事件处理线程相关
    boost::thread _audio_event_thread;                              // 音频事件处理线程
    std::vector<AudioType> _audio_removed_process_infos;            // 待移除的音频处理信息
    std::list<AudioProcessInfo> _audio_current_process_infos;       // 当前音频处理信息队列
    std::list<AudioProcessInfo> _audio_next_process_infos;          // 下一轮音频处理信息队列
    AudioType _audio_playing_type = AudioType::Max;                 // 当前播放的音频类型
    std::recursive_mutex _audio_event_mutex;                        // 音频事件互斥锁
    boost::condition_variable_any _audio_event_cv;                  // 音频事件条件变量

    esp_event_handler_instance_t _wifi_event_handler = nullptr;     // WiFi事件处理器

    // 单例相关静态成员
    inline static std::mutex _instance_mutex;                       // 实例互斥锁
    inline static std::shared_ptr<AI_Buddy> _instance;              // 单例实例

    /**
     * @brief 表情符号映射表
     * 将文本表情描述映射到具体的表情类型和图标类型
     */
    inline static ai_framework::Expression::EmojiMap _emoji_map = {
        {"neutral", std::make_pair(ExpressionEmotionSlowBlink, ExpressionIconTypeNone)},     // 中性
        {"happy", std::make_pair(ExpressionEmotionHappy, ExpressionIconTypeNone)},           // 开心
        {"laughing", std::make_pair(ExpressionEmotionHappy, ExpressionIconTypeNone)},        // 大笑
        {"funny", std::make_pair(ExpressionEmotionHappy, ExpressionIconTypeNone)},           // 有趣
        {"sad", std::make_pair(ExpressionEmotionSad, ExpressionIconTypeNone)},               // 悲伤
        {"angry", std::make_pair(ExpressionEmotionAngry, ExpressionIconTypeNone)},           // 愤怒
        {"crying", std::make_pair(ExpressionEmotionSad, ExpressionIconTypeNone)},            // 哭泣
        {"loving", std::make_pair(ExpressionEmotionHappy, ExpressionIconTypeNone)},          // 爱意
        {"embarrassed", std::make_pair(ExpressionEmotionFastBlink, ExpressionIconEmotionThinking)}, // 尴尬
        {"surprised", std::make_pair(ExpressionEmotionFastBlink, ExpressionIconTypeNone)},   // 惊讶
        {"shocked", std::make_pair(ExpressionEmotionFastBlink, ExpressionIconTypeNone)},     // 震惊
        {"thinking", std::make_pair(ExpressionEmotionFastBlink, ExpressionIconEmotionThinking)}, // 思考
        {"relaxed", std::make_pair(ExpressionEmotionHappy, ExpressionIconTypeNone)},         // 放松
        {"delicious", std::make_pair(ExpressionEmotionHappy, ExpressionIconTypeNone)},       // 美味
        {"kissy", std::make_pair(ExpressionEmotionHappy, ExpressionIconTypeNone)},           // 亲吻
        {"confident", std::make_pair(ExpressionEmotionHappy, ExpressionIconTypeNone)},       // 自信
        {"sleepy", std::make_pair(ExpressionEmotionSleep, ExpressionIconEmotionSleep)},      // 困倦
        {"silly", std::make_pair(ExpressionEmotionFastBlink, ExpressionIconTypeNone)},       // 愚蠢
        {"confused", std::make_pair(ExpressionEmotionFastBlink, ExpressionIconEmotionConfused)}, // 困惑
        {"curious", std::make_pair(ExpressionEmotionFastBlink, ExpressionIconEmotionConfused)},  // 好奇
    };
    
    /**
     * @brief 系统图标映射表
     * 将系统图标名称映射到具体的图标类型
     */
    inline static ai_framework::Expression::SystemIconMap _system_icon_map = {
        {"brightness_down", ExpressionIconSystemBrightnessDown},    // 亮度降低
        {"brightness_up", ExpressionIconSystemBrightnessUp},        // 亮度提高
        {"server_connected", ExpressionIconSystemServerConnected},  // 服务器已连接
        {"server_connecting", ExpressionIconSystemServerConnecting}, // 服务器连接中
        {"volume_down", ExpressionIconSystemVolumeDown},            // 音量降低
        {"volume_mute", ExpressionIconSystemVolumeMute},            // 静音
        {"volume_up", ExpressionIconSystemVolumeUp},                // 音量提高
        {"wifi_disconnected", ExpressionIconSystemWifiDisconnected}, // WiFi断开
    };
    
    /**
     * @brief 音频文件映射表
     * 将音频类型映射到文件路径和播放时长
     */
    inline static std::map<AudioType, AudioInfo> _audio_file_map = {
        {AI_Buddy::AudioType::WifiNeedConnect,      {"file://spiffs/wifi_need_connect.mp3", 4 * 1000}},
        {AI_Buddy::AudioType::WifiConnected,        {"file://spiffs/wifi_connect_success.mp3", 2 * 1000}},
        {AI_Buddy::AudioType::WifiDisconnected,     {"file://spiffs/wifi_disconnect.mp3", 4 * 1000}},
        {AI_Buddy::AudioType::ServerConnected,      {"file://spiffs/server_connected.mp3", 2 * 1000}},
        {AI_Buddy::AudioType::ServerDisconnected,   {"file://spiffs/server_disconnect.mp3", 2 * 1000}},
        {AI_Buddy::AudioType::ServerConnecting,     {"file://spiffs/server_connecting.mp3", 3 * 1000}},
        {AI_Buddy::AudioType::MicOn,                {"file://spiffs/mic_open.mp3", 2 * 1000}},
        {AI_Buddy::AudioType::MicOff,               {"file://spiffs/mic_close.mp3", 5 * 1000}},
        {AI_Buddy::AudioType::WakeUp,               {"file://spiffs/wake_up.mp3", 3 * 1000}},
        {AI_Buddy::AudioType::ResponseLaiLo,        {"file://spiffs/response_lai_lo.mp3", 2 * 1000}},
        {AI_Buddy::AudioType::ResponseWoZaiTingNe,  {"file://spiffs/response_wo_zai_ting_ne.mp3", 2 * 1000}},
        {AI_Buddy::AudioType::ResponseWoZai,        {"file://spiffs/response_wo_zai.mp3", 2 * 1000}},
        {AI_Buddy::AudioType::ResponseZaiNe,        {"file://spiffs/response_zai_ne.mp3", 1 * 1000}},
        {AI_Buddy::AudioType::SleepBaiBaiLo,        {"file://spiffs/sleep_bai_bai_lo.mp3", 2 * 1000}},
        {AI_Buddy::AudioType::SleepHaoDe,           {"file://spiffs/sleep_hao_de.mp3", 3 * 1000}},
        {AI_Buddy::AudioType::SleepWoTuiXiaLe,      {"file://spiffs/sleep_wo_tui_xia_le.mp3", 2 * 1000}},
        {AI_Buddy::AudioType::SleepXianZheYangLo,   {"file://spiffs/sleep_xian_zhe_yang_lo.mp3", 3 * 1000}},
        {AI_Buddy::AudioType::InvalidConfig,        {"file://spiffs/invalid_config_file.mp3", 5 * 1000}},
    };
    
    /**
     * @brief 回应音频随机播放列表
     * 定义AI伙伴回应时随机选择的音频和概率
     */
    inline static RandomAudios _response_audios = {
        {0.25, AudioType::ResponseLaiLo},        // 25%概率播放"来咯"
        {0.25, AudioType::ResponseWoZaiTingNe},  // 25%概率播放"我在听呢"
        {0.25, AudioType::ResponseWoZai},        // 25%概率播放"我在"
        {0.25, AudioType::ResponseZaiNe},        // 25%概率播放"在呢"
    };
    
    /**
     * @brief 睡眠音频随机播放列表
     * 定义AI伙伴睡眠时随机选择的音频和概率
     */
    inline static RandomAudios _sleep_audios = {
        {0.25, AudioType::SleepBaiBaiLo},        // 25%概率播放"拜拜咯"
        {0.25, AudioType::SleepHaoDe},           // 25%概率播放"好的"
        {0.25, AudioType::SleepWoTuiXiaLe},      // 25%概率播放"我退下了"
        {0.25, AudioType::SleepXianZheYangLo},   // 25%概率播放"先这样咯"
    };
};

} // namespace esp_brookesia::speaker
