# 智能音箱AI语音功能时序图

## 系统架构概述

基于ESP-SR组件的语音识别系统，包含音频前端处理(AFE)、唤醒词检测(WakeNet)、语音活动检测(VAD)和命令词识别(MultiNet)。

## 主要时序图

### 1. 完整语音交互流程

```mermaid
sequenceDiagram
    participant User as 👤 用户
    participant Mic as 🎤 麦克风
    participant AFE as 🔧 音频前端处理
    participant WakeNet as 🎯 唤醒词检测
    participant VAD as 📊 语音活动检测  
    participant MultiNet as 🧠 命令词识别
    participant App as 📱 应用层
    participant Speaker as 🔊 扬声器

    Note over User,Speaker: 系统初始化和待机状态
    AFE->>AFE: 初始化音频前端处理器
    AFE->>WakeNet: 启用唤醒词检测
    AFE->>VAD: 禁用VAD (原始状态保存)
    AFE->>MultiNet: 初始化命令词模型
    
    Note over User,Speaker: 阶段1: 等待唤醒
    loop 持续监听
        User->>Mic: 环境音频输入
        Mic->>AFE: 原始音频数据
        AFE->>AFE: AEC回声消除
        AFE->>AFE: 降噪处理(BSS)
        AFE->>WakeNet: 处理后的音频
        WakeNet->>WakeNet: 检测唤醒词
    end
    
    Note over User,Speaker: 阶段2: 唤醒词触发
    User->>Mic: "嗨，乐鑫"
    Mic->>AFE: 唤醒词音频
    AFE->>WakeNet: 音频帧处理
    WakeNet->>AFE: WAKENET_DETECTED事件
    AFE->>AFE: 状态切换: ST_IDLE → ST_WAKEUP
    AFE->>VAD: 启用VAD检测
    AFE->>AFE: 启动唤醒定时器
    AFE->>App: ESP_GMF_AFE_EVT_WAKEUP_START
    App->>Speaker: 播放提示音 "我在听"
    
    Note over User,Speaker: 阶段3: 语音活动检测
    User->>Mic: 开始说话 "打开空调"
    Mic->>AFE: 语音音频流
    AFE->>VAD: 音频流分析
    VAD->>AFE: VAD_SPEECH检测到
    AFE->>AFE: 状态切换: ST_WAKEUP → ST_SPEECHING
    AFE->>AFE: 停止唤醒定时器
    AFE->>MultiNet: 开始命令词检测
    AFE->>App: ESP_GMF_AFE_EVT_VAD_START
    
    Note over User,Speaker: 阶段4: 命令词识别
    loop 语音处理
        User->>Mic: 持续语音输入
        Mic->>AFE: 音频数据流
        AFE->>MultiNet: 实时音频处理
        MultiNet->>MultiNet: 语音识别分析
    end
    
    MultiNet->>AFE: ESP_MN_STATE_DETECTED
    AFE->>MultiNet: 获取识别结果
    MultiNet->>AFE: 命令词结果 {phrase_id: 25, prob: 0.998, str: "da kai kong tiao"}
    AFE->>App: ESP_GMF_AFE_EVT_VCMD_DETECTED + 识别结果
    
    Note over User,Speaker: 阶段5: 语音结束检测
    User->>User: 停止说话
    Mic->>AFE: 环境噪音
    AFE->>VAD: 噪音分析
    VAD->>AFE: VAD_SILENCE检测到
    AFE->>AFE: 状态切换: ST_SPEECHING → ST_WAIT_FOR_SLEEP
    AFE->>AFE: 启动结束定时器
    AFE->>App: ESP_GMF_AFE_EVT_VAD_END
    
    Note over User,Speaker: 阶段6: 命令执行
    App->>App: 解析命令词 "打开空调"
    App->>App: 执行控制逻辑
    App->>Speaker: 播放反馈 "空调已打开"
    
    Note over User,Speaker: 阶段7: 返回待机
    AFE->>AFE: 结束定时器超时
    AFE->>AFE: 状态切换: ST_WAIT_FOR_SLEEP → ST_IDLE
    AFE->>VAD: 禁用VAD检测
    AFE->>App: ESP_GMF_AFE_EVT_WAKEUP_END
    AFE->>WakeNet: 恢复唤醒词监听
```

### 2. 状态机转换详图

```mermaid
stateDiagram-v2
    [*] --> ST_IDLE : 系统初始化
    
    ST_IDLE --> ST_WAKEUP : 唤醒词检测到<br/>(ET_WWE_DECT)
    ST_IDLE --> ST_SPEECHING : 无唤醒词模式下<br/>语音检测到<br/>(ET_SPEECH_DECT)
    
    ST_WAKEUP --> ST_SPEECHING : 语音活动开始<br/>(ET_SPEECH_DECT)
    ST_WAKEUP --> ST_IDLE : 唤醒超时<br/>(ET_WAKEUP_TIMER_EXPIRED)
    ST_WAKEUP --> ST_WAKEUP : 保持唤醒状态修改<br/>(ET_KEEP_WAKE_MODIFIED)
    
    ST_SPEECHING --> ST_WAIT_FOR_SLEEP : 语音结束<br/>(ET_NOISE_DECT)<br/>且启用唤醒模式
    ST_SPEECHING --> ST_IDLE : 语音结束<br/>(ET_NOISE_DECT)<br/>且禁用唤醒模式
    
    ST_WAIT_FOR_SLEEP --> ST_SPEECHING : 新语音活动<br/>(ET_SPEECH_DECT)
    ST_WAIT_FOR_SLEEP --> ST_IDLE : 等待超时<br/>(ET_WAKEUP_TIMER_EXPIRED)
    ST_WAIT_FOR_SLEEP --> ST_WAKEUP : 保持唤醒启用<br/>(ET_KEEP_WAKE_MODIFIED)
    
    note right of ST_IDLE
        等待唤醒词
        WakeNet活跃
        VAD禁用
    end note
    
    note right of ST_WAKEUP
        唤醒词已触发
        VAD启用
        等待语音输入
    end note
    
    note right of ST_SPEECHING
        正在处理语音
        命令词识别激活
        音频流处理
    end note
    
    note right of ST_WAIT_FOR_SLEEP
        语音结束
        等待新语音或超时
        延迟进入睡眠
    end note
```

### 3. 音频处理管道详图

```mermaid
flowchart TD
    A[🎤 多路麦克风输入<br/>16kHz, 16bit] --> B[音频前端处理器<br/>AFE Manager]
    
    B --> C[回声消除<br/>AEC Engine]
    C --> D[盲源分离降噪<br/>BSS/SE Processing]
    D --> E[语音活动检测<br/>VAD WebRTC]
    
    E --> F{唤醒词检测<br/>WakeNet}
    F -->|检测到唤醒词| G[状态切换<br/>ST_IDLE → ST_WAKEUP]
    
    E --> H{命令词识别<br/>MultiNet}
    H -->|识别成功| I[返回识别结果<br/>phrase_id + 置信度]
    
    G --> J[启用VAD检测]
    J --> K[等待语音输入]
    K --> H
    
    I --> L[应用层处理]
    L --> M[🔊 语音反馈]
    L --> N[📱 控制执行]
    
    style A fill:#e1f5fe
    style B fill:#f3e5f5
    style F fill:#fff3e0
    style H fill:#e8f5e8
    style L fill:#fce4ec
```

### 4. 事件回调机制

```mermaid
sequenceDiagram
    participant AFE as AFE处理器
    participant EventCB as 事件回调
    participant App as 应用层
    participant UI as 用户界面
    
    Note over AFE,UI: 事件驱动的异步处理
    
    AFE->>EventCB: ESP_GMF_AFE_EVT_WAKEUP_START
    EventCB->>App: wakeup_info {wake_word_index, confidence}
    App->>UI: 显示唤醒状态
    App->>UI: 播放提示音
    
    AFE->>EventCB: ESP_GMF_AFE_EVT_VAD_START  
    EventCB->>App: 语音活动开始
    App->>UI: 显示"正在听..."
    
    AFE->>EventCB: ESP_GMF_AFE_EVT_VCMD_DETECTED
    EventCB->>App: vcmd_info {phrase_id, prob, str}
    App->>App: 命令词解析和执行
    App->>UI: 显示识别结果
    
    AFE->>EventCB: ESP_GMF_AFE_EVT_VAD_END
    EventCB->>App: 语音活动结束
    App->>UI: 隐藏"正在听..."
    
    AFE->>EventCB: ESP_GMF_AFE_EVT_WAKEUP_END
    EventCB->>App: 唤醒会话结束
    App->>UI: 返回待机状态
    
    alt 命令词检测超时
        AFE->>EventCB: ESP_GMF_AFE_EVT_VCMD_DECT_TIMEOUT
        EventCB->>App: 识别超时 (默认5760ms)
        App->>UI: 显示"未识别到命令"
    end
```

### 5. 内存管理和数据流

```mermaid
flowchart LR
    A[输入音频缓冲区<br/>Ring Buffer] --> B[AFE处理器<br/>esp_gmf_afe_t]
    B --> C[输出音频缓冲区<br/>Ring Buffer]
    
    B --> D[WakeNet模型<br/>内存加载]
    B --> E[MultiNet模型<br/>内存加载]
    
    F[PSRAM分配<br/>~200KB LVGL<br/>~300KB 音频缓冲] --> A
    F --> C
    F --> D
    F --> E
    
    G[Flash存储<br/>~600KB 模型数据] --> D
    G --> E
    
    H[线程同步<br/>Mutex + Semaphore] --> B
    I[定时器管理<br/>ESP Timer] --> B
    
    style F fill:#e3f2fd
    style G fill:#f1f8e9
    style H fill:#fff8e1
    style I fill:#fce4ec
```

## 关键技术参数

### 音频参数
- **采样率**: 16kHz
- **位深度**: 16bit
- **通道数**: 2个麦克风 + 1个回放通道
- **数据块大小**: 2048字节 (默认)

### 算法配置
- **唤醒词模型**: WakeNet v9 (wn9_hilexin)
- **命令词模型**: MultiNet v6 (mn6_cn) 
- **VAD算法**: WebRTC VAD
- **降噪算法**: 盲源分离 (BSS)

### 超时设置
- **唤醒超时**: 可配置 (默认根据应用设定)
- **命令词超时**: 5760ms (5.76秒)
- **等待睡眠**: 可配置延时

### 内存占用
- **PSRAM使用**: ~500KB (音频缓冲 + LVGL)
- **Flash模型**: ~600KB (语音识别模型)
- **运行内存**: ~50KB (状态管理 + 线程栈)

## 典型使用场景

1. **标准唤醒模式**: "嗨乐鑫" → "打开空调" → 执行命令
2. **连续对话模式**: 保持唤醒状态，无需重复唤醒词
3. **直接命令模式**: 禁用唤醒词，直接语音命令识别
4. **超时处理**: 唤醒后无语音输入或识别超时的处理

这个时序图展示了ESP-SR在智能音箱中的完整工作流程，从音频采集到最终的命令执行，包括所有的状态转换和事件处理机制。
