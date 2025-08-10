# Speaker 系统说明

* [English Version](./README.md)

## 简介

此文档ai生成，仔细阅读
Speaker 是基于 esp-brookesia 框架开发的多媒体与 AI 语音助手系统 UI，适用于带有显示屏、扬声器、麦克风等多媒体输入输出设备的 ESP 系列开发板。该系统支持多分辨率自适应、AI Buddy 语音助手、多应用管理和丰富的多媒体内容展示。

<div align="center">
    <img src="https://your-image-link.com/speaker_demo.jpg" alt="Speaker 运行效果" width="750">
</div>

## 主要特性

- **多分辨率支持**：UI 元素可根据屏幕自动适配，适合多种尺寸和分辨率的设备。
- **AI 语音助手**：内置 AI Buddy，支持语音识别、语音播报、智能问答等功能。
- **多应用管理**：支持多 App 的安装、卸载、后台管理与切换。
- **多媒体显示**：支持图片、动画、歌词等多媒体内容的展示。
- **模块化设计**：Display、Manager、AI Buddy 等子模块可独立扩展和调用。

> [!NOTE]
> Speaker 的默认样式表已适配常见分辨率，如需适配其它分辨率可参考样式表自定义。

## 快速上手

### 1. 配置环境

1. 安装 ESP-IDF 并配置好开发环境。
2. 克隆 esp-brookesia 仓库并进入项目目录。
3. 运行 `idf.py menuconfig`，导航到 `Component config` > `ESP Brookesia - Core Configurations`，根据实际需求启用或配置 Speaker 相关选项。

### 2. 编译与烧录

1. 连接开发板，配置好串口参数。
2. 运行 `idf.py build` 编译项目。
3. 运行 `idf.py -p <PORT> flash` 烧录固件。

### 3. 运行效果

上电后，Speaker 系统会自动启动，显示主界面并支持多媒体与语音助手功能。

## 如何开发 Speaker App

### 原理及功能

Speaker 系统通过 C++ 继承的方式开发 app。用户可继承 Speaker 的 app 基类（如 `App`），实现必要的虚函数（如 `run()`、`back()`），并可根据需求重写 `close()`、`init()`、`pause()` 等函数。这些接口源自内核 app 基类 `ESP_Brookesia_CoreApp`。

Speaker app 支持如下静态配置功能（通过 `AppData` 结构体）：

- **名称**：设置 app 的名称。
- **图标**：设置 app 的启动图标。
- **屏幕大小**：建议设置为实际屏幕大小，避免显示异常。
- **自动创建默认屏幕**：可配置 app 启动时自动创建默认屏幕。
- **自动资源回收**：可配置 app 退出时自动清理屏幕、动画、定时器等资源。
- **自动调整屏幕大小**：可配置 app 屏幕自动适配可视区域。

Speaker 系统还支持 AI Buddy、Display、Manager 等子模块，开发者可通过 `Speaker` 类的成员变量直接访问和管理。

### 代码示例

```cpp
#include "esp_brookesia_speaker_app.hpp"

class MySpeakerApp : public App {
public:
    MySpeakerApp() {
        // 配置 app 信息
        data.name = "My App";
        data.launcher_icon = my_icon;
        data.screen_size = ESP_BROOKESIA_STYLE_SIZE_RECT_PERCENT(100, 100);
        data.enable_default_screen = 1;
    }
    void run() override {
        // 初始化界面和多媒体内容
    }
    void back() override {
        // 返回处理
    }
};
```

### 注意事项

- **函数重名**：建议 app 内全局变量和函数采用 `<app_name>_` 前缀，避免重名。
- **多媒体资源管理**：如需播放音频、动画等多媒体内容，建议在 `run()` 或 `resume()` 内创建，并配合资源回收机制，避免内存泄漏。
- **AI Buddy 使用**：如需集成语音助手功能，可参考 `AI_Buddy` 子模块的接口文档。
- **UI 适配**：如需自定义 UI，可参考样式表机制，或根据屏幕分辨率调整 UI 元素。

> [!NOTE]
> Speaker 支持手写代码和第三方 UI 工具导出代码两种开发方式。手写代码建议开启自动创建默认屏幕，导出代码需关闭该功能并按需调整 UI 相关文件。

## 内置的 UI 组件

### Display

用于多媒体内容的显示，包括图片、动画、歌词等，支持自适应缩放和多种显示模式。

### Manager

负责 app 的安装、卸载、后台管理等功能，支持多 app 共存与切换。

### AI Buddy

内置 AI 语音助手，支持语音识别、语音播报、智能问答等功能。

## 样式表与分辨率适配

Speaker 提供多种分辨率的 UI 样式表，开发者可根据实际屏幕分辨率选择或自定义样式表，确保最佳显示效果。

## 常见问题

- **如何自定义样式表？**
  参考 `systems/speaker/stylesheets/` 目录下的样式表模板，根据实际需求修改参数。
- **如何集成第三方语音服务？**
  可在 AI Buddy 子模块中扩展语音识别与云端服务接口。
- **如何扩展多媒体功能？**
  可在 Display 子模块中添加自定义多媒体控件或解码器。

## 参考示例

- [apps/brookesia_app_gif_player](../../apps/brookesia_app_gif_player/)
- [apps/brookesia_app_ai_profile](../../apps/brookesia_app_ai_profile/)

## 反馈与支持

如有问题或建议，请在 Github issue 区留言，或联系维护者。
