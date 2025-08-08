# ESP-Brookesia GIF播放器App

这是一个基于ESP-Brookesia框架开发的GIF动画播放器应用程序。

## 功能特性

- 🎬 支持播放GIF动画文件
- ⏯️ 播放/暂停控制
- 🔄 循环播放模式
- 📁 文件浏览功能
- 📱 适配手机界面布局
- 🎨 现代化UI设计

## 技术架构

### 基础架构
```
GifPlayer (继承自 ESP_Brookesia_PhoneApp)
├── 生命周期管理 (init/run/pause/resume/close/deinit)
├── UI组件管理 (容器/按钮/列表/标签)
├── GIF播放控制 (播放/暂停/停止/循环)
└── 文件系统交互 (扫描/加载GIF文件)
```

### UI布局
```
┌─────────────────────────────────┐
│           Status Bar            │ (可选)
├─────────────────────────────────┤
│                                 │
│         GIF Display Area        │ (70%高度)
│         (gif_container)         │
│                                 │
├─────────────────────────────────┤
│  [播放] [暂停] [循环]           │ (控制面板)
├─────────────────────────────────┤
│  📁 file1.gif                   │
│  📁 file2.gif                   │ (文件列表)
│  📁 animation.gif               │
└─────────────────────────────────┘
│        Navigation Bar           │ (可选)
└─────────────────────────────────┘
```

## 使用方法

### 1. 基本集成
```cpp
#include "esp_brookesia_app_gif_player.hpp"

// 在主程序中注册App
void app_main() {
    // 初始化ESP-Brookesia核心
    esp_brookesia_core_init();
    
    // 获取GIF播放器实例
    auto gif_player = esp_brookesia::apps::GifPlayer::requestInstance(
        true,  // 显示状态栏
        true   // 显示导航栏
    );
    
    // 注册到应用管理器
    esp_brookesia_app_manager_install_app(gif_player);
}
```

### 2. 播放GIF文件
```cpp
// 播放指定GIF文件
gif_player->playGif("/spiffs/animations/demo.gif");

// 停止播放
gif_player->stopGif();

// 切换暂停状态
gif_player->togglePause();

// 设置循环播放
gif_player->setLoop(true);
```

## 文件系统配置

### SPIFFS配置
在`partitions.csv`中添加SPIFFS分区：
```
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x6000,
phy_init, data, phy,     0xf000,  0x1000,
factory,  app,  factory, 0x10000, 0x100000,
spiffs,   data, spiffs,  0x110000, 0x100000,
```

### menuconfig配置
```bash
idf.py menuconfig
```

在menuconfig中配置：
```
Component config → SPIFFS Configuration
├── [*] Enable SPIFFS
├── Maximum Number of Partitions (3)
└── [*] Use magic numbers in SPIFFS

Component config → LVGL configuration  
├── [*] Enable GIF decoder
└── Color settings → Color depth (16: RGB565)
```

## GIF文件要求

### 支持格式
- **文件格式**: 标准GIF87a/GIF89a
- **颜色深度**: 最高256色
- **分辨率**: 建议不超过400x400像素
- **文件大小**: 建议小于1MB

### 性能优化建议
```cpp
// 推荐的GIF规格
Resolution: 320x240 或更小
Colors: 64色以内  
Frame Rate: 10-15 FPS
File Size: < 500KB
```

## API参考

### 核心方法
```cpp
class GifPlayer {
public:
    // 单例模式
    static GifPlayer* requestInstance(bool status_bar = true, bool nav_bar = true);
    
    // GIF播放控制
    bool playGif(const char* gif_path);     // 播放GIF
    void stopGif(void);                     // 停止播放  
    void togglePause(void);                 // 暂停/继续
    void setLoop(bool loop);                // 设置循环模式
    
    // 生命周期回调
    bool run(void) override;                // App启动
    bool back(void) override;               // 返回事件
    bool close(void) override;              // App关闭
};
```

### 事件回调
```cpp
// 按钮事件
static void _play_btn_event_cb(lv_event_t *e);    // 播放按钮
static void _loop_btn_event_cb(lv_event_t *e);    // 循环按钮  
static void _file_list_event_cb(lv_event_t *e);   // 文件选择
```

## 内存使用

### 典型内存占用
```
App实例:        ~2KB
UI组件:         ~8KB  
GIF缓冲区:      根据文件大小 (通常10-100KB)
LVGL缓存:       ~20KB
总计:           ~30-130KB
```

### 优化建议
1. **压缩GIF**: 减少颜色数和帧率
2. **分块加载**: 大文件分块读取
3. **缓存管理**: 及时释放不用的资源
4. **内存监控**: 定期检查内存使用情况

## 故障排除

### 常见问题

1. **编译错误**
```bash
# 确保LVGL启用GIF支持
CONFIG_LV_USE_GIF=y
```

2. **文件无法播放**
```cpp
// 检查文件路径和权限
ESP_LOGI("GIF", "File exists: %s", access(path, F_OK) == 0 ? "Yes" : "No");
```

3. **内存不足**
```cpp
// 监控堆内存
ESP_LOGI("MEM", "Free heap: %d", esp_get_free_heap_size());
```

### 调试方法
```cpp
// 启用详细日志
#define ESP_UTILS_LOG_TAG "BS:GifPlayer"
ESP_UTILS_LOGD("Debug message");
ESP_UTILS_LOGI("Info message"); 
ESP_UTILS_LOGW("Warning message");
ESP_UTILS_LOGE("Error message");
```

## 扩展功能

### 添加自定义控件
```cpp
// 在_create_ui()中添加新控件
void _create_custom_controls(void) {
    // 速度控制滑块
    lv_obj_t *speed_slider = lv_slider_create(_control_panel);
    lv_slider_set_range(speed_slider, 50, 200);  // 50%-200%速度
    
    // 亮度控制
    lv_obj_t *brightness_slider = lv_slider_create(_control_panel);
    lv_slider_set_range(brightness_slider, 0, 100);
}
```

### 添加网络下载功能
```cpp
// 从URL下载GIF
bool downloadGif(const char* url, const char* local_path) {
    // HTTP客户端实现
    // 下载并保存到本地
    return true;
}
```

## 许可证

Apache License 2.0 - 详见LICENSE文件

## 贡献

欢迎提交Issue和Pull Request来改进这个应用程序！
