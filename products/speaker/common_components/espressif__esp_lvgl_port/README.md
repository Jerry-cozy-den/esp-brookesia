# LVGL ESP 移植组件（中文翻译）

[![组件注册表](https://components.espressif.com/components/espressif/esp_lvgl_port/badge.svg)](https://components.espressif.com/components/espressif/esp_lvgl_port)
![维护状态](https://img.shields.io/badge/maintenance-actively--developed-brightgreen.svg)

本组件用于在 ESP 设备上结合乐鑫 LCD 和触摸驱动程序使用 LVGL 图形库。适用于任何带 LCD 显示屏的项目。

## 特性
* LVGL 初始化
    * 创建任务和定时器
    * 处理屏幕旋转
    * 电源管理
* 添加/移除显示屏（使用 [`esp_lcd`](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/lcd.html)）
* 添加/移除触摸输入（使用 [`esp_lcd_touch`](https://github.com/espressif/esp-bsp/tree/master/components/lcd_touch)）
* 添加/移除导航按钮输入（使用 [`button`](https://github.com/espressif/esp-iot-solution/tree/master/components/button)）
* 添加/移除编码器输入（使用 [`knob`](https://github.com/espressif/esp-iot-solution/tree/master/components/knob)）
* 添加/移除 USB HID 鼠标/键盘输入（使用 [`usb_host_hid`](https://components.espressif.com/components/espressif/usb_host_hid)）

## LVGL 版本

本组件支持 **LVGL8** 和 **LVGL9**。默认选择最新的 LVGL 版本。如果你想使用特定版本（如最新的 LVGL8），可在项目的 `idf_component.yml` 中这样定义：

```
  lvgl/lvgl:
    version: "^8"
    public: true
```

### LVGL 版本兼容性

本组件完全兼容 LVGL 9。所有类型和函数均来自 LVGL9。部分 LVGL9 类型在 LVGL8 中不支持，已在 [`esp_lvgl_port_compatibility.h`](include/esp_lvgl_port_compatibility.h) 头文件中重新定义。**请注意，部分绘图和对象函数在 LVGL8 与 LVGL9 之间不兼容。**

## 用法

### 初始化
``` c
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    esp_err_t err = lvgl_port_init(&lvgl_cfg);
```

### 添加屏幕

向 LVGL 添加 LCD 屏幕。可多次调用此函数以添加多个 LCD 屏幕。

``` c
    static lv_disp_t * disp_handle;

    /* LCD IO */
    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) 1, &io_config, &io_handle));

    /* LCD 驱动初始化 */
    esp_lcd_panel_handle_t lcd_panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &lcd_panel_handle));

    /* 添加 LCD 屏幕 */
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = lcd_panel_handle,
        .buffer_size = DISP_WIDTH*DISP_HEIGHT,
        .double_buffer = true,
        .hres = DISP_WIDTH,
        .vres = DISP_HEIGHT,
        .monochrome = false,
        .mipi_dsi = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = false,
        }
    };
    disp_handle = lvgl_port_add_disp(&disp_cfg);

    /* ... 其余初始化代码 ... */

    /* 若要反初始化 LVGL 端口，请记得删除所有显示屏： */
    lvgl_port_remove_disp(disp_handle);
```

> [!NOTE]
> 1. 若要添加 RGB 或 MIPI-DSI 屏幕，请使用 `lvgl_port_add_disp_rgb` 或 `lvgl_port_add_disp_dsi`。
> 2. 仅当使用 `LV_COLOR_FORMAT_RGB565` 颜色格式时才可使用 DMA 缓冲区。

### 添加触摸输入

向 LVGL 添加触摸输入。可多次调用以为不同屏幕添加多个触摸输入。
``` c
    /* Touch driver initialization */
    ...
    esp_lcd_touch_handle_t tp;
    esp_err_t err = esp_lcd_touch_new_i2c_gt911(io_handle, &tp_cfg, &tp);

    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp_handle,
        .handle = tp,
    };
    lv_indev_t* touch_handle = lvgl_port_add_touch(&touch_cfg);

    /* ... 其余初始化代码 ... */

    /* 若要反初始化 LVGL 端口，请记得删除所有触摸输入： */
    lvgl_port_remove_touch(touch_handle);
```

> [!NOTE]
> 如果屏幕分辨率与触摸分辨率不同，可在 `lvgl_port_touch_cfg_t` 配置结构体中添加 `.scale.x` 或 `.scale.y` 启用缩放。

### 添加按钮输入

向 LVGL 添加按钮输入。可多次调用以为不同显示屏添加多个按钮输入。此功能仅在项目已添加 `espressif/button` 组件时可用。
``` c
    /* Buttons configuration structure */
    const button_gpio_config_t bsp_button_config[] = {
        {
            .gpio_num = GPIO_NUM_37,
            .active_level = 0,
        },
        {
            .gpio_num = GPIO_NUM_38,
            .active_level = 0,
        },
        {
            .gpio_num = GPIO_NUM_39,
            .active_level = 0,
        },
    };


    const button_config_t btn_cfg = {0};
    button_handle_t prev_btn_handle = NULL;
    button_handle_t next_btn_handle = NULL;
    button_handle_t enter_btn_handle = NULL;
    iot_button_new_gpio_device(&btn_cfg, &bsp_button_config[0], &prev_btn_handle);
    iot_button_new_gpio_device(&btn_cfg, &bsp_button_config[1], &next_btn_handle);
    iot_button_new_gpio_device(&btn_cfg, &bsp_button_config[2], &enter_btn_handle);

    const lvgl_port_nav_btns_cfg_t btns = {
        .disp = disp_handle,
        .button_prev = prev_btn_handle,
        .button_next = next_btn_handle,
        .button_enter = enter_btn_handle
    };

    /* Add buttons input (for selected screen) */
    lv_indev_t* buttons_handle = lvgl_port_add_navigation_buttons(&btns);

    /* ... 其余初始化代码 ... */

    /* 若要反初始化 LVGL 端口，请记得删除所有按钮输入： */
    lvgl_port_remove_navigation_buttons(buttons_handle);
```
> [!NOTE]
> 使用导航按钮控制 LVGL 对象时，这些对象必须加入 LVGL 组。详见 [LVGL 文档](https://docs.lvgl.io/master/overview/indev.html?highlight=lv_indev_get_act#keypad-and-encoder)。

### 添加编码器输入

向 LVGL 添加编码器输入。可多次调用以为不同显示屏添加多个编码器输入。此功能仅在项目已添加 `espressif/knob` 组件时可用。
``` c

    static const button_gpio_config_t encoder_btn_config = {
        .gpio_num = GPIO_BTN_PRESS,
        .active_level = 0,
    };

    const knob_config_t encoder_a_b_config = {
        .default_direction = 0,
        .gpio_encoder_a = GPIO_ENCODER_A,
        .gpio_encoder_b = GPIO_ENCODER_B,
    };

    const button_config_t btn_cfg = {0};
    button_handle_t encoder_btn_handle = NULL;
    BSP_ERROR_CHECK_RETURN_NULL(iot_button_new_gpio_device(&btn_cfg, &encoder_btn_config, &encoder_btn_handle));

    /* Encoder configuration structure */
    const lvgl_port_encoder_cfg_t encoder = {
        .disp = disp_handle,
        .encoder_a_b = &encoder_a_b_config,
        .encoder_enter = encoder_btn_handle
    };

    /* Add encoder input (for selected screen) */
    lv_indev_t* encoder_handle = lvgl_port_add_encoder(&encoder);

    /* ... 其余初始化代码 ... */

    /* 若要反初始化 LVGL 端口，请记得删除所有编码器： */
    lvgl_port_remove_encoder(encoder_handle);
```
> [!NOTE]
> 使用编码器控制 LVGL 对象时，这些对象必须加入 LVGL 组。详见 [LVGL 文档](https://docs.lvgl.io/master/overview/indev.html?highlight=lv_indev_get_act#keypad-and-encoder)。

### 添加 USB HID 键盘和鼠标输入

向 LVGL 添加鼠标和键盘输入。此功能仅在项目已添加组件 [usb_host_hid](https://components.espressif.com/components/espressif/usb_host_hid) 时可用。

``` c
    /* USB initialization */
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));

    ...

    /* Add mouse input device */
    const lvgl_port_hid_mouse_cfg_t mouse_cfg = {
        .disp = display,
        .sensitivity = 1, /* Sensitivity of the mouse moving */
    };
    lvgl_port_add_usb_hid_mouse_input(&mouse_cfg);

    /* Add keyboard input device */
    const lvgl_port_hid_keyboard_cfg_t kb_cfg = {
        .disp = display,
    };
    kb_indev = lvgl_port_add_usb_hid_keyboard_input(&kb_cfg);
```

键盘特殊行为（当对象在组中时）：
- TAB：选择下一个对象
- SHIFT + TAB：选择上一个对象
- ENTER：控制对象（例如点击按钮）
- 方向键 或 HOME 或 END：在文本区域中移动
- DEL 或 Backspace：在文本区域中删除字符

> [!NOTE]
> 使用键盘控制 LVGL 对象时，这些对象必须加入 LVGL 组。详见 [LVGL 文档](https://docs.lvgl.io/master/overview/indev.html?highlight=lv_indev_get_act#keypad-and-encoder)。

### LVGL API 使用

所有对 LVGL 的调用都必须用以下加锁/解锁命令保护：
``` c
	/* Wait for the other task done the screen operation */
    lvgl_port_lock(0);
    ...
    lv_obj_t * screen = lv_disp_get_scr_act(disp_handle);
    lv_obj_t * obj = lv_label_create(screen);
    ...
    /* Screen operation done -> release for the other task */
    lvgl_port_unlock();
```

### 屏幕旋转

LVGL 端口支持显示屏旋转。你可以选择软件旋转或硬件旋转。
使用软件旋转时，无需在 `flush_cb` 回调中加入额外逻辑。

可在 `lvgl_port_display_cfg_t` 结构体中选择旋转模式：
``` c
    const lvgl_port_display_cfg_t disp_cfg = {
        ...
        .flags = {
            ...
            .sw_rotate = true / false, // true: software; false: hardware
        }
    }
```
运行时也可以更改显示方向。

``` c
    lv_disp_set_rotation(disp_handle, LV_DISP_ROT_90);
```

> [!NOTE]
> 软件旋转会消耗更多 RAM。若芯片支持（例如 ESP32P4），软件旋转会使用 [PPA](https://docs.espressif.com/projects/esp-idf/en/latest/esp32p4/api-reference/peripherals/ppa.html)。

> [!NOTE]
> 硬件旋转期间，组件会调用 [`esp_lcd`](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/lcd.html) API。使用软件旋转时，驱动中不可启用 `direct_mode` 和 `full_refresh`。详见 [LVGL 文档](https://docs.lvgl.io/8.3/porting/display.html?highlight=sw_rotate)。

### 使用 PSRAM 画布

当 SRAM 不足时，可以把 PSRAM 用作画布，并使用较小的 trans_buffer 进行传输，这样绘制更高效。
``` c
    const lvgl_port_display_cfg_t disp_cfg = {
        ...
        .buffer_size = DISP_WIDTH * DISP_HEIGHT, // in PSRAM, not DMA-capable
        .trans_size = size, // in SRAM, DMA-capable
        .flags = {
            .buff_spiram = true,
            .buff_dma = false,
            ...
        }
    }
```

### 生成图片（C 数组）

在构建期间可生成图片的 C 数组，只需在主 CMakeLists.txt 末尾添加如下几行：
```
# Generate C array for each image
lvgl_port_create_c_image("images/logo.png" "images/" "ARGB8888" "NONE")
lvgl_port_create_c_image("images/image.png" "images/" "ARGB8888" "NONE")
# Add generated images to build
lvgl_port_add_images(${COMPONENT_LIB} "images/")
```

创建 C 图片函数的用法：
```
lvgl_port_create_c_image(input_image output_folder color_format compression)
```

可用的颜色格式：
L8,I1,I2,I4,I8,A1,A2,A4,A8,ARGB8888,XRGB8888,RGB565,RGB565A8,RGB888,TRUECOLOR,TRUECOLOR_ALPHA,AUTO

可用的压缩算法：
NONE,RLE,LZ4

> [!NOTE]
> 参数 `color_format` 与 `compression` 仅在 LVGL 9 中使用。

## 省电

LVGL 端口可针对省电模式进行优化，主要包括两个功能。

### LVGL 任务休眠

为提升省电效果，当 LVGL 任务无事可做时应进入休眠。将 `task_max_sleep_ms` 设为较大的值，LVGL 任务仅在有事件时被唤醒。

LVGL 任务在以下情况会被唤醒：
* LVGL 显示无效化
* LVGL 动画进行中
* 触摸中断
* 按钮中断
* 旋钮中断
* USB 鼠标/键盘中断
* 超时（由配置结构体中的 `task_max_sleep_ms` 控制）
* 用户唤醒（通过 `lvgl_port_task_wake` 函数）

> [!WARNING]
> 此功能自 LVGL 9 起可用。

> [!NOTE]
> 当你将 `task_max_sleep_ms` 设置为较大值时，别忘了在触摸驱动中配置中断引脚。

### 停止定时器

在 light-sleep 模式下定时器仍可能运行。使用 light-sleep 之前，你可以通过以下函数停止 LVGL 定时器：

```
lvgl_port_stop();
```

唤醒后可通过以下函数恢复 LVGL 定时器：

```
lvgl_port_resume();
```

## 性能

对每个图形应用而言，性能是关键。提升 LCD 性能的推荐设置见单独文档：[这里](docs/performance.md)。

### 性能监视器

若要在 LVGL9 中显示性能监视器，请在 sdkconfig.defaults 中加入以下配置并重新完整构建：

```
CONFIG_LV_USE_OBSERVER=y
CONFIG_LV_USE_SYSMON=y
CONFIG_LV_USE_PERF_MONITOR=y
```
