/**
 * @file echoear.c
 * @brief ESP-Brookesia EchoEar 智能音箱板级支持包 (BSP) 实现文件
 * 
 * 该文件实现了 EchoEar 智能音箱开发板的所有硬件接口功能，包括：
 * - I2C总线初始化和管理
 * - LCD显示屏初始化和背光控制
 * - 触控系统初始化和管理
 * - SD卡存储系统支持
 * - PCB版本自动检测
 * - 电源管理功能
 * 
 * 本文件为应用程序提供统一的硬件抽象层接口。
 */

/* ESP-IDF 系统头文件 */
#include "driver/gpio.h"         // GPIO驱动接口
#include "driver/ledc.h"         // LED控制器PWM驱动
#include "driver/spi_master.h"   // SPI主机驱动
#include "esp_err.h"             // ESP错误码定义
#include "esp_log.h"             // ESP日志系统
#include "esp_check.h"           // ESP错误检查宏
#include "esp_lcd_panel_io.h"    // LCD面板IO接口
#include "esp_lcd_panel_vendor.h" // LCD厂商特定驱动
#include "esp_lcd_panel_ops.h"   // LCD面板操作接口

/* BSP 相关头文件 */
#include "bsp/esp-bsp.h"         // BSP主头文件
#include "bsp/display.h"         // 显示系统接口
#include "bsp/touch.h"           // 触控系统接口
#include "esp_lcd_touch_cst816s.h" // CST816S触控驱动
#include "esp_lvgl_port.h"       // LVGL图形库端口
#include "bsp_err_check.h"       // BSP错误检查宏

/* 标准C库头文件 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "driver/uart.h"         // UART驱动
#include "esp_sleep.h"           // 电源管理和睡眠
#include "esp_timer.h"           // 高精度定时器
#include "sdkconfig.h"           // SDK配置头文件

/* 系统配置定义 */
#define GPIO_WAKEUP_LEVEL       0    /*!< GPIO唤醒电平：低电平唤醒 */

static const char *TAG = "BSP";
/* 全局变量定义 - 系统状态管理 */
static lv_disp_t *disp;                          /*!< LVGL显示对象指针 */
static lv_indev_t *disp_indev = NULL;            /*!< LVGL输入设备对象指针（触控） */
static esp_lcd_touch_handle_t tp;                /*!< LCD触控设备句柄 */
static esp_lcd_panel_handle_t panel_handle = NULL; /*!< LCD面板设备句柄 */
static bsp_pcd_diff_info_t pcd_info = {};        /*!< PCB版本差异信息缓存 */
static bool pcd_info_initialized = false;       /*!< PCB版本信息是否已初始化标志 */

static i2c_master_bus_handle_t i2c_handle = NULL; /*!< I2C主机总线句柄 */
static sdmmc_card_t *bsp_sdcard = NULL;          /*!< SD卡设备句柄（全局） */
static bool i2c_initialized = false;            /*!< I2C总线是否已初始化标志 */

/**
 * @brief 初始化I2C总线
 * 
 * 配置I2C主机模式，设置SCL和SDA引脚，初始化I2C总线。
 * 采用重入保护，避免重复初始化。I2C总线用于连接多个外设：
 * - ES8311音频编解码器（配置用）
 * - ES7210音频ADC（配置用）
 * - CST816S触控控制器
 * - ICM-42607-P惯性测量单元（预留）
 * 
 * @return 
 *      - ESP_OK: 初始化成功
 *      - ESP_ERR_*: 初始化失败的具体错误码
 */
esp_err_t bsp_i2c_init(void)
{
    /* 检查I2C是否已经初始化，避免重复初始化 */
    if (i2c_initialized) {
        return ESP_OK;
    }

    /* 配置I2C主机总线参数 */
    const i2c_master_bus_config_t i2c_config = {
        .i2c_port = BSP_I2C_NUM,           // I2C端口号
        .sda_io_num = BSP_I2C_SDA,         // SDA数据线GPIO
        .scl_io_num = BSP_I2C_SCL,         // SCL时钟线GPIO  
        .clk_source = I2C_CLK_SRC_DEFAULT, // 使用默认时钟源
    };
    BSP_ERROR_CHECK_RETURN_ERR(i2c_new_master_bus(&i2c_config, &i2c_handle));

    i2c_initialized = true;  // 标记为已初始化

    return ESP_OK;
}

/**
 * @brief 反初始化I2C总线
 * 
 * 释放I2C总线资源，删除I2C主机总线句柄。
 * 调用后需要重新调用bsp_i2c_init()才能继续使用I2C功能。
 * 
 * @return 
 *      - ESP_OK: 反初始化成功
 *      - ESP_ERR_*: 反初始化失败的具体错误码
 */
esp_err_t bsp_i2c_deinit(void)
{
    BSP_ERROR_CHECK_RETURN_ERR(i2c_del_master_bus(i2c_handle));
    i2c_initialized = false;  // 标记为未初始化
    return ESP_OK;
}

/**
 * @brief 获取I2C总线句柄
 * 
 * 返回I2C主机总线句柄，供其他模块创建I2C设备时使用。
 * 如果I2C未初始化，会自动调用初始化函数。
 * 
 * @return I2C主机总线句柄
 */
i2c_master_bus_handle_t bsp_i2c_get_handle(void)
{
    bsp_i2c_init();  // 确保I2C已初始化
    return i2c_handle;
}

/* LCD显示相关配置常量 */
#define LCD_CMD_BITS           8     /*!< LCD SPI命令位数：8位（ST7789标准） */
#define LCD_PARAM_BITS         8     /*!< LCD SPI参数位数：8位 */
#define LCD_LEDC_CH            CONFIG_BSP_DISPLAY_BRIGHTNESS_LEDC_CH /*!< LCD背光PWM通道号 */

/**
 * @brief 初始化显示屏背光亮度控制
 * 
 * 使用LEDC（LED控制器）生成PWM信号控制LCD背光亮度。
 * 配置PWM频率为4kHz，分辨率为10位（0-1023）。
 * 初始亮度设置为0（关闭状态）。
 * 
 * @return 
 *      - ESP_OK: 初始化成功
 *      - ESP_ERR_*: 初始化失败的具体错误码
 */
esp_err_t bsp_display_brightness_init(void)
{
    /* 配置LEDC PWM通道用于背光控制 */
    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num = BSP_LCD_BACKLIGHT,     // 背光控制GPIO引脚
        .speed_mode = LEDC_LOW_SPEED_MODE, // 使用低速模式
        .channel = LCD_BACKLIIGHT_CHANNEL, // PWM通道号
        .intr_type = LEDC_INTR_DISABLE,    // 禁用PWM中断
        .timer_sel = LEDC_TIMER_0,         // 使用定时器0
        .duty = 0,                         // 初始占空比为0（关闭）
        .hpoint = 0                        // PWM波形起始点
    };
    
    /* 配置LEDC PWM定时器参数 */
    const ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,  // 低速模式
        .duty_resolution = LEDC_TIMER_10_BIT, // 10位分辨率（0-1023）
        .timer_num = LEDC_TIMER_0,          // 定时器编号
        .freq_hz = 4000,                    // PWM频率：4kHz
        .clk_cfg = LEDC_AUTO_CLK            // 自动选择时钟源
    };

    /* 初始化PWM定时器和通道 */
    BSP_ERROR_CHECK_RETURN_ERR(ledc_timer_config(&LCD_backlight_timer));
    BSP_ERROR_CHECK_RETURN_ERR(ledc_channel_config(&LCD_backlight_channel));

    return ESP_OK;
}

/**
 * @brief 设置显示屏背光亮度
 * 
 * 通过调整PWM占空比来控制LCD背光亮度。
 * 亮度值会被限制在0-100%范围内，并转换为对应的PWM占空比。
 * 
 * @param brightness_percent 亮度百分比（0-100）
 *                          0 = 完全关闭背光
 *                          100 = 最大亮度
 * 
 * @return 
 *      - ESP_OK: 设置成功
 *      - ESP_ERR_*: 设置失败的具体错误码
 */
esp_err_t bsp_display_brightness_set(int brightness_percent)
{
    /* 限制亮度值在有效范围内 */
    if (brightness_percent > 100) {
        brightness_percent = 100;
    }
    if (brightness_percent < 0) {
        brightness_percent = 0;
    }

    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);
    
    /* 将百分比转换为PWM占空比值（10位分辨率：100% = 1023） */
    uint32_t duty_cycle = (1023 * brightness_percent) / 100; 
    
    /* 设置PWM占空比并更新输出 */
    BSP_ERROR_CHECK_RETURN_ERR(ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH, duty_cycle));
    BSP_ERROR_CHECK_RETURN_ERR(ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH));

    return ESP_OK;
}

/**
 * @brief 关闭显示屏背光
 * 
 * 将背光亮度设置为0，完全关闭LCD背光。
 * 这是bsp_display_brightness_set(0)的便捷封装函数。
 * 
 * @return 
 *      - ESP_OK: 关闭成功
 *      - ESP_ERR_*: 关闭失败的具体错误码
 */
esp_err_t bsp_display_backlight_off(void)
{
    return bsp_display_brightness_set(0);
}

/**
 * @brief 打开显示屏背光
 * 
 * 将背光亮度设置为100%，开启LCD背光到最大亮度。
 * 这是bsp_display_brightness_set(100)的便捷封装函数。
 * 
 * @return 
 *      - ESP_OK: 打开成功
 *      - ESP_ERR_*: 打开失败的具体错误码
 */
esp_err_t bsp_display_backlight_on(void)
{
    return bsp_display_brightness_set(100);
}

/**
 * @brief ST77916 LCD控制器厂商特定初始化命令序列
 * 
 * 这个命令数组包含了ST77916 LCD控制器的完整初始化序列，包括：
 * - 显示参数配置（亮度、对比度、伽马校正等）  
 * - 电源管理设置
 * - 时序参数配置
 * - 接口模式设置（QSPI模式）
 * - 颜色空间和像素格式配置
 * 
 * 命令格式：{寄存器地址, 参数数组, 参数长度, 延时(ms)}
 * 最后的120ms延时用于等待显示屏完全初始化完成。
 */
static const st77916_lcd_init_cmd_t vendor_specific_init_default[] = {
    {0xF0, (uint8_t []){0x28}, 1, 0},
    {0xF2, (uint8_t []){0x28}, 1, 0},
    {0x73, (uint8_t []){0xF0}, 1, 0},
    {0x7C, (uint8_t []){0xD1}, 1, 0},
    {0x83, (uint8_t []){0xE0}, 1, 0},
    {0x84, (uint8_t []){0x61}, 1, 0},
    {0xF2, (uint8_t []){0x82}, 1, 0},
    {0xF0, (uint8_t []){0x00}, 1, 0},
    {0xF0, (uint8_t []){0x01}, 1, 0},
    {0xF1, (uint8_t []){0x01}, 1, 0},
    {0xB0, (uint8_t []){0x56}, 1, 0},
    {0xB1, (uint8_t []){0x4D}, 1, 0},
    {0xB2, (uint8_t []){0x24}, 1, 0},
    {0xB4, (uint8_t []){0x87}, 1, 0},
    {0xB5, (uint8_t []){0x44}, 1, 0},
    {0xB6, (uint8_t []){0x8B}, 1, 0},
    {0xB7, (uint8_t []){0x40}, 1, 0},
    {0xB8, (uint8_t []){0x86}, 1, 0},
    {0xBA, (uint8_t []){0x00}, 1, 0},
    {0xBB, (uint8_t []){0x08}, 1, 0},
    {0xBC, (uint8_t []){0x08}, 1, 0},
    {0xBD, (uint8_t []){0x00}, 1, 0},
    {0xC0, (uint8_t []){0x80}, 1, 0},
    {0xC1, (uint8_t []){0x10}, 1, 0},
    {0xC2, (uint8_t []){0x37}, 1, 0},
    {0xC3, (uint8_t []){0x80}, 1, 0},
    {0xC4, (uint8_t []){0x10}, 1, 0},
    {0xC5, (uint8_t []){0x37}, 1, 0},
    {0xC6, (uint8_t []){0xA9}, 1, 0},
    {0xC7, (uint8_t []){0x41}, 1, 0},
    {0xC8, (uint8_t []){0x01}, 1, 0},
    {0xC9, (uint8_t []){0xA9}, 1, 0},
    {0xCA, (uint8_t []){0x41}, 1, 0},
    {0xCB, (uint8_t []){0x01}, 1, 0},
    {0xD0, (uint8_t []){0x91}, 1, 0},
    {0xD1, (uint8_t []){0x68}, 1, 0},
    {0xD2, (uint8_t []){0x68}, 1, 0},
    {0xF5, (uint8_t []){0x00, 0xA5}, 2, 0},
    {0xDD, (uint8_t []){0x4F}, 1, 0},
    {0xDE, (uint8_t []){0x4F}, 1, 0},
    {0xF1, (uint8_t []){0x10}, 1, 0},
    {0xF0, (uint8_t []){0x00}, 1, 0},
    {0xF0, (uint8_t []){0x02}, 1, 0},
    {0xE0, (uint8_t []){0xF0, 0x0A, 0x10, 0x09, 0x09, 0x36, 0x35, 0x33, 0x4A, 0x29, 0x15, 0x15, 0x2E, 0x34}, 14, 0},
    {0xE1, (uint8_t []){0xF0, 0x0A, 0x0F, 0x08, 0x08, 0x05, 0x34, 0x33, 0x4A, 0x39, 0x15, 0x15, 0x2D, 0x33}, 14, 0},
    {0xF0, (uint8_t []){0x10}, 1, 0},
    {0xF3, (uint8_t []){0x10}, 1, 0},
    {0xE0, (uint8_t []){0x07}, 1, 0},
    {0xE1, (uint8_t []){0x00}, 1, 0},
    {0xE2, (uint8_t []){0x00}, 1, 0},
    {0xE3, (uint8_t []){0x00}, 1, 0},
    {0xE4, (uint8_t []){0xE0}, 1, 0},
    {0xE5, (uint8_t []){0x06}, 1, 0},
    {0xE6, (uint8_t []){0x21}, 1, 0},
    {0xE7, (uint8_t []){0x01}, 1, 0},
    {0xE8, (uint8_t []){0x05}, 1, 0},
    {0xE9, (uint8_t []){0x02}, 1, 0},
    {0xEA, (uint8_t []){0xDA}, 1, 0},
    {0xEB, (uint8_t []){0x00}, 1, 0},
    {0xEC, (uint8_t []){0x00}, 1, 0},
    {0xED, (uint8_t []){0x0F}, 1, 0},
    {0xEE, (uint8_t []){0x00}, 1, 0},
    {0xEF, (uint8_t []){0x00}, 1, 0},
    {0xF8, (uint8_t []){0x00}, 1, 0},
    {0xF9, (uint8_t []){0x00}, 1, 0},
    {0xFA, (uint8_t []){0x00}, 1, 0},
    {0xFB, (uint8_t []){0x00}, 1, 0},
    {0xFC, (uint8_t []){0x00}, 1, 0},
    {0xFD, (uint8_t []){0x00}, 1, 0},
    {0xFE, (uint8_t []){0x00}, 1, 0},
    {0xFF, (uint8_t []){0x00}, 1, 0},
    {0x60, (uint8_t []){0x40}, 1, 0},
    {0x61, (uint8_t []){0x04}, 1, 0},
    {0x62, (uint8_t []){0x00}, 1, 0},
    {0x63, (uint8_t []){0x42}, 1, 0},
    {0x64, (uint8_t []){0xD9}, 1, 0},
    {0x65, (uint8_t []){0x00}, 1, 0},
    {0x66, (uint8_t []){0x00}, 1, 0},
    {0x67, (uint8_t []){0x00}, 1, 0},
    {0x68, (uint8_t []){0x00}, 1, 0},
    {0x69, (uint8_t []){0x00}, 1, 0},
    {0x6A, (uint8_t []){0x00}, 1, 0},
    {0x6B, (uint8_t []){0x00}, 1, 0},
    {0x70, (uint8_t []){0x40}, 1, 0},
    {0x71, (uint8_t []){0x03}, 1, 0},
    {0x72, (uint8_t []){0x00}, 1, 0},
    {0x73, (uint8_t []){0x42}, 1, 0},
    {0x74, (uint8_t []){0xD8}, 1, 0},
    {0x75, (uint8_t []){0x00}, 1, 0},
    {0x76, (uint8_t []){0x00}, 1, 0},
    {0x77, (uint8_t []){0x00}, 1, 0},
    {0x78, (uint8_t []){0x00}, 1, 0},
    {0x79, (uint8_t []){0x00}, 1, 0},
    {0x7A, (uint8_t []){0x00}, 1, 0},
    {0x7B, (uint8_t []){0x00}, 1, 0},
    {0x80, (uint8_t []){0x48}, 1, 0},
    {0x81, (uint8_t []){0x00}, 1, 0},
    {0x82, (uint8_t []){0x06}, 1, 0},
    {0x83, (uint8_t []){0x02}, 1, 0},
    {0x84, (uint8_t []){0xD6}, 1, 0},
    {0x85, (uint8_t []){0x04}, 1, 0},
    {0x86, (uint8_t []){0x00}, 1, 0},
    {0x87, (uint8_t []){0x00}, 1, 0},
    {0x88, (uint8_t []){0x48}, 1, 0},
    {0x89, (uint8_t []){0x00}, 1, 0},
    {0x8A, (uint8_t []){0x08}, 1, 0},
    {0x8B, (uint8_t []){0x02}, 1, 0},
    {0x8C, (uint8_t []){0xD8}, 1, 0},
    {0x8D, (uint8_t []){0x04}, 1, 0},
    {0x8E, (uint8_t []){0x00}, 1, 0},
    {0x8F, (uint8_t []){0x00}, 1, 0},
    {0x90, (uint8_t []){0x48}, 1, 0},
    {0x91, (uint8_t []){0x00}, 1, 0},
    {0x92, (uint8_t []){0x0A}, 1, 0},
    {0x93, (uint8_t []){0x02}, 1, 0},
    {0x94, (uint8_t []){0xDA}, 1, 0},
    {0x95, (uint8_t []){0x04}, 1, 0},
    {0x96, (uint8_t []){0x00}, 1, 0},
    {0x97, (uint8_t []){0x00}, 1, 0},
    {0x98, (uint8_t []){0x48}, 1, 0},
    {0x99, (uint8_t []){0x00}, 1, 0},
    {0x9A, (uint8_t []){0x0C}, 1, 0},
    {0x9B, (uint8_t []){0x02}, 1, 0},
    {0x9C, (uint8_t []){0xDC}, 1, 0},
    {0x9D, (uint8_t []){0x04}, 1, 0},
    {0x9E, (uint8_t []){0x00}, 1, 0},
    {0x9F, (uint8_t []){0x00}, 1, 0},
    {0xA0, (uint8_t []){0x48}, 1, 0},
    {0xA1, (uint8_t []){0x00}, 1, 0},
    {0xA2, (uint8_t []){0x05}, 1, 0},
    {0xA3, (uint8_t []){0x02}, 1, 0},
    {0xA4, (uint8_t []){0xD5}, 1, 0},
    {0xA5, (uint8_t []){0x04}, 1, 0},
    {0xA6, (uint8_t []){0x00}, 1, 0},
    {0xA7, (uint8_t []){0x00}, 1, 0},
    {0xA8, (uint8_t []){0x48}, 1, 0},
    {0xA9, (uint8_t []){0x00}, 1, 0},
    {0xAA, (uint8_t []){0x07}, 1, 0},
    {0xAB, (uint8_t []){0x02}, 1, 0},
    {0xAC, (uint8_t []){0xD7}, 1, 0},
    {0xAD, (uint8_t []){0x04}, 1, 0},
    {0xAE, (uint8_t []){0x00}, 1, 0},
    {0xAF, (uint8_t []){0x00}, 1, 0},
    {0xB0, (uint8_t []){0x48}, 1, 0},
    {0xB1, (uint8_t []){0x00}, 1, 0},
    {0xB2, (uint8_t []){0x09}, 1, 0},
    {0xB3, (uint8_t []){0x02}, 1, 0},
    {0xB4, (uint8_t []){0xD9}, 1, 0},
    {0xB5, (uint8_t []){0x04}, 1, 0},
    {0xB6, (uint8_t []){0x00}, 1, 0},
    {0xB7, (uint8_t []){0x00}, 1, 0},
    {0xB8, (uint8_t []){0x48}, 1, 0},
    {0xB9, (uint8_t []){0x00}, 1, 0},
    {0xBA, (uint8_t []){0x0B}, 1, 0},
    {0xBB, (uint8_t []){0x02}, 1, 0},
    {0xBC, (uint8_t []){0xDB}, 1, 0},
    {0xBD, (uint8_t []){0x04}, 1, 0},
    {0xBE, (uint8_t []){0x00}, 1, 0},
    {0xBF, (uint8_t []){0x00}, 1, 0},
    {0xC0, (uint8_t []){0x10}, 1, 0},
    {0xC1, (uint8_t []){0x47}, 1, 0},
    {0xC2, (uint8_t []){0x56}, 1, 0},
    {0xC3, (uint8_t []){0x65}, 1, 0},
    {0xC4, (uint8_t []){0x74}, 1, 0},
    {0xC5, (uint8_t []){0x88}, 1, 0},
    {0xC6, (uint8_t []){0x99}, 1, 0},
    {0xC7, (uint8_t []){0x01}, 1, 0},
    {0xC8, (uint8_t []){0xBB}, 1, 0},
    {0xC9, (uint8_t []){0xAA}, 1, 0},
    {0xD0, (uint8_t []){0x10}, 1, 0},
    {0xD1, (uint8_t []){0x47}, 1, 0},
    {0xD2, (uint8_t []){0x56}, 1, 0},
    {0xD3, (uint8_t []){0x65}, 1, 0},
    {0xD4, (uint8_t []){0x74}, 1, 0},
    {0xD5, (uint8_t []){0x88}, 1, 0},
    {0xD6, (uint8_t []){0x99}, 1, 0},
    {0xD7, (uint8_t []){0x01}, 1, 0},
    {0xD8, (uint8_t []){0xBB}, 1, 0},
    {0xD9, (uint8_t []){0xAA}, 1, 0},
    {0xF3, (uint8_t []){0x01}, 1, 0},
    {0xF0, (uint8_t []){0x00}, 1, 0},
    {0x21, (uint8_t []){}, 0, 0},      // 显示反色关闭
    {0x11, (uint8_t []){}, 0, 0},      // 退出睡眠模式
    {0x00, (uint8_t []){}, 0, 120},    // 等待120ms完成初始化
};

/**
 * @brief 创建并初始化LCD显示设备
 * 
 * 完整的LCD显示系统初始化流程，包括：
 * 1. 初始化背光控制
 * 2. 检测PCB版本（用于确定GPIO配置）
 * 3. 初始化I2C总线
 * 4. 配置SPI总线（支持QSPI模式）
 * 5. 创建LCD面板IO接口
 * 6. 初始化ST77916显示控制器
 * 7. 执行厂商特定的初始化命令序列
 * 
 * @param config 显示配置参数，包含传输缓冲区大小等
 * @param ret_panel 返回的LCD面板句柄指针
 * @param ret_io 返回的LCD面板IO句柄指针
 * 
 * @return 
 *      - ESP_OK: 初始化成功
 *      - ESP_ERR_*: 初始化失败的具体错误码
 */
esp_err_t bsp_display_new(const bsp_display_config_t *config, esp_lcd_panel_handle_t *ret_panel, esp_lcd_panel_io_handle_t *ret_io)
{
    esp_err_t ret = ESP_OK;
    assert(config != NULL && config->max_transfer_sz > 0);

    /* 初始化显示背光控制 */
    ESP_RETURN_ON_ERROR(bsp_display_brightness_init(), TAG, "Brightness init failed");
    
    /* 检测PCB版本，获取版本相关的GPIO配置 */
    BSP_ERROR_CHECK_RETURN_ERR(bsp_pcb_version_detect(NULL));

    /* 初始化I2C总线（用于触控等外设） */
    BSP_ERROR_CHECK_RETURN_ERR(bsp_i2c_init());

    ESP_LOGD(TAG, "Initialize SPI bus");
    /* 配置SPI总线，支持QSPI四线模式 */
    const spi_bus_config_t buscfg = {
        .sclk_io_num = BSP_LCD_PCLK,        // SPI时钟线
        .data0_io_num = BSP_LCD_DATA0,      // QSPI数据线0 (MISO)
        .data1_io_num = BSP_LCD_DATA1,      // QSPI数据线1 (MOSI)
        .data2_io_num = BSP_LCD_DATA2,      // QSPI数据线2
        .data3_io_num = BSP_LCD_DATA3,      // QSPI数据线3
        .max_transfer_sz = config->max_transfer_sz, // 最大传输大小
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    ESP_LOGD(TAG, "Install panel IO");
    /* 配置LCD面板SPI接口参数 */
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_DC,          // 数据/命令选择引脚
        .cs_gpio_num = BSP_LCD_CS,          // SPI片选引脚
        .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,  // SPI时钟频率：40MHz
        .lcd_cmd_bits = LCD_CMD_BITS,       // 命令位数：32位
        .lcd_param_bits = LCD_PARAM_BITS,   // 参数位数：8位
        .spi_mode = 0,                      // SPI模式0
        .trans_queue_depth = 1,             // 传输队列深度
        .flags = {
            .quad_mode = true,              // 启用QSPI四线模式
        },
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM, &io_config, ret_io), err, TAG, "New panel IO failed");

    ESP_LOGD(TAG, "Install LCD driver");
    /* 配置ST77916厂商特定参数 */
    const st77916_vendor_config_t vendor_config = {
        .init_cmds = vendor_specific_init_default,                              // 初始化命令数组
        .init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(vendor_specific_init_default[0]), // 命令数量
        .flags = {
            .use_qspi_interface = 1, // 使用QSPI接口模式
        },
    };
    
    /* 配置LCD面板设备参数 */
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = pcd_info.lcd.rst_pin,     // 复位引脚（版本相关）
        .color_space = BSP_LCD_COLOR_SPACE,         // 颜色空间
        .bits_per_pixel = BSP_LCD_BITS_PER_PIXEL,   // 每像素位数
        .flags = {
            .reset_active_high = pcd_info.lcd.rst_active_level, // 复位信号有效电平（版本相关）
        },
        .vendor_config = (void *) &vendor_config,   // 厂商特定配置
    };

    ESP_LOGI(TAG, "Install st77916 panel driver");
    /* 创建ST77916 LCD面板驱动实例 */
    ESP_ERROR_CHECK(esp_lcd_new_panel_st77916(*ret_io, (const esp_lcd_panel_dev_config_t *)&panel_config, ret_panel));

    /* 执行LCD初始化序列 */
    esp_lcd_panel_reset(*ret_panel);        // 硬件复位
    esp_lcd_panel_init(*ret_panel);         // 执行初始化命令
    esp_lcd_panel_disp_on_off(*ret_panel, true); // 开启显示输出
    
    return ret;

/* 错误处理：清理已分配的资源 */
err:
    if (*ret_panel) {
        esp_lcd_panel_del(*ret_panel);  // 删除LCD面板
    }
    if (*ret_io) {
        esp_lcd_panel_io_del(*ret_io);  // 删除面板IO
    }
    spi_bus_free(BSP_LCD_SPI_NUM);      // 释放SPI总线
    return ret;
}

/**
 * @brief 初始化LVGL LCD显示系统
 * 
 * 创建LVGL显示对象，配置显示缓冲区和渲染参数。
 * 包括分辨率、颜色格式、旋转设置、缓冲区配置等。
 * 
 * @param cfg LVGL显示配置参数
 * 
 * @return 成功时返回LVGL显示对象指针，失败时返回NULL
 */
static lv_disp_t *bsp_display_lcd_init(const bsp_display_cfg_t *cfg)
{
    assert(cfg != NULL);
    esp_lcd_panel_io_handle_t io_handle = NULL;
    
    /* 配置BSP显示参数 */
    const bsp_display_config_t bsp_disp_cfg = {
        .max_transfer_sz = (BSP_LCD_H_RES * CONFIG_BSP_LCD_DRAW_BUF_HEIGHT) * sizeof(uint16_t),
    };
    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_new(&bsp_disp_cfg, &panel_handle, &io_handle));

    /* 开启LCD显示 */
    esp_lcd_panel_disp_on_off(panel_handle, true);

    /* 添加LCD屏幕到LVGL系统 */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,              // LCD面板IO句柄
        .panel_handle = panel_handle,        // LCD面板句柄
        .buffer_size = cfg->buffer_size,     // 显示缓冲区大小
        .double_buffer = cfg->double_buffer, // 是否启用双缓冲
        .hres = BSP_LCD_H_RES,              // 水平分辨率
        .vres = BSP_LCD_V_RES,              // 垂直分辨率
        .monochrome = false,                 // 彩色显示（非单色）
        /* 旋转设置必须与esp_lcd的初始设置保持一致 */
        .rotation = {
            .swap_xy = false,    // 不交换X-Y轴
            .mirror_x = false,   // X轴不镜像
            .mirror_y = false,   // Y轴不镜像
        },
        .flags = {
            .buff_dma = cfg->flags.buff_dma,           // 是否使用DMA缓冲区
            .buff_spiram = cfg->flags.buff_spiram,     // 是否使用PSRAM缓冲区
            .swap_bytes = true,                        // 交换字节序（RGB565格式需要）
            .default_dummy_draw = cfg->flags.default_dummy_draw, // 是否使用虚拟绘制
        }
    };

    return lvgl_port_add_disp(&disp_cfg);  // 添加显示设备到LVGL端口
}

/**
 * @brief 创建并初始化触控设备
 * 
 * 初始化CST816S电容触控芯片，配置触控区域、中断引脚等参数。
 * 触控芯片通过I2C总线进行通信，中断引脚用于检测触摸事件。
 * 
 * @param config 触控配置参数（当前未使用，传入NULL即可）
 * @param ret_touch 返回的触控设备句柄指针
 * 
 * @return 
 *      - ESP_OK: 初始化成功
 *      - ESP_ERR_*: 初始化失败的具体错误码
 */
esp_err_t bsp_touch_new(const bsp_touch_config_t *config, esp_lcd_touch_handle_t *ret_touch)
{
    /* 初始化I2C总线（用于触控芯片通信） */
    BSP_ERROR_CHECK_RETURN_ERR(bsp_i2c_init());

    /* 配置触控设备参数 */
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = BSP_LCD_H_RES,        // 触控区域最大X坐标
        .y_max = BSP_LCD_V_RES,        // 触控区域最大Y坐标
        .rst_gpio_num = GPIO_NUM_NC,   // 复位引脚（与LCD共用，这里不配置）
        .int_gpio_num = BSP_LCD_TOUCH_INT, // 触控中断引脚
        .levels = {
            .reset = 0,                // 复位信号有效电平
            .interrupt = 0,            // 中断信号有效电平
        },
        .flags = {
            .swap_xy = 0,              // 不交换X-Y坐标
            .mirror_x = 0,             // X坐标不镜像
            .mirror_y = 0,             // Y坐标不镜像
        },
    };

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    /* 配置触控芯片I2C接口参数 */
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();

    /* 创建I2C面板IO */
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c_v2(i2c_handle, &tp_io_config, &tp_io_handle), TAG, "");

    /* 创建CST816S触控设备 */
    ESP_RETURN_ON_ERROR(esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, ret_touch), TAG, "New CST816S failed");

    return ESP_OK;
}

/**
 * @brief 初始化LVGL输入设备（触控）
 * 
 * 创建触控设备并将其添加到LVGL输入系统中。
 * 触控事件将被转换为LVGL输入事件，用于UI交互。
 * 
 * @param disp LVGL显示对象指针
 * 
 * @return 成功时返回LVGL输入设备指针，失败时返回NULL
 */
static lv_indev_t *bsp_display_indev_init(lv_disp_t *disp)
{
    BSP_ERROR_CHECK_RETURN_NULL(bsp_touch_new(NULL, &tp));
    assert(tp);

    /* 添加触控输入设备（针对指定的显示屏） */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,     // 关联的显示设备
        .handle = tp,     // 触控设备句柄
    };

    return lvgl_port_add_touch(&touch_cfg);  // 添加到LVGL触控系统
}

/**
 * @brief 启动显示系统（使用默认配置）
 * 
 * 使用预设的默认配置启动完整的显示系统，包括：
 * - LVGL端口初始化
 * - LCD显示器初始化
 * - 触控输入设备初始化
 * - 背光控制初始化
 * 
 * 默认配置包括DMA缓冲区、单/双缓冲设置等。
 * 
 * @return 成功时返回LVGL显示对象指针，失败时返回NULL
 */
lv_disp_t *bsp_display_start(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(), // LVGL端口默认配置
        .buffer_size = BSP_LCD_H_RES * CONFIG_BSP_LCD_DRAW_BUF_HEIGHT, // 缓冲区大小
#if CONFIG_BSP_LCD_DRAW_BUF_DOUBLE
        .double_buffer = 1,  // 启用双缓冲
#else
        .double_buffer = 0,  // 单缓冲模式
#endif
        .flags = {
            .buff_dma = true,     // 使用DMA缓冲区
            .buff_spiram = false, // 不使用PSRAM缓冲区
        }
    };
    return bsp_display_start_with_config(&cfg);  // 使用配置启动显示系统
}

/**
 * @brief 使用自定义配置启动显示系统
 * 
 * 使用用户提供的配置参数启动完整的显示系统。
 * 这是显示系统的主要初始化函数，会依次进行：
 * 1. 初始化LVGL端口
 * 2. 初始化显示背光控制
 * 3. 初始化LCD显示器和LVGL显示对象
 * 4. 初始化触控输入设备
 * 
 * @param cfg 显示配置参数结构体
 * 
 * @return 成功时返回LVGL显示对象指针，失败时返回NULL
 */
lv_disp_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg)
{
    assert(cfg != NULL);
    /* 初始化LVGL端口 */
    BSP_ERROR_CHECK_RETURN_NULL(lvgl_port_init(&cfg->lvgl_port_cfg));
    /* 初始化显示背光控制 */
    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_brightness_init());
    /* 初始化LCD显示器 */
    BSP_NULL_CHECK(disp = bsp_display_lcd_init(cfg), NULL);
    lv_display_set_user_data(disp, (void *)panel_handle);  // 关联LCD面板句柄
    /* 初始化触控输入设备 */
    BSP_NULL_CHECK(disp_indev = bsp_display_indev_init(disp), NULL);
    return disp;  // 返回LVGL显示对象
}

/**
 * @brief 获取显示输入设备句柄
 * 
 * 返回当前已初始化的LVGL输入设备（触控）句柄。
 * 可用于直接操作触控设备或获取触控状态。
 * 
 * @return LVGL输入设备句柄，未初始化时返回NULL
 */
lv_indev_t *bsp_display_get_input_dev(void)
{
    return disp_indev;
}

/**
 * @brief 获取LVGL显示互斥锁
 * 
 * 由于LVGL不是线程安全的，在多线程环境中调用LVGL API前
 * 必须先获取此互斥锁，使用完毕后调用bsp_display_unlock()释放。
 * 
 * @param timeout_ms 超时时间（毫秒），0表示无限等待
 * 
 * @return 
 *      - true: 成功获取互斥锁
 *      - false: 获取互斥锁失败（超时）
 */
bool bsp_display_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

/**
 * @brief 释放LVGL显示互斥锁
 * 
 * 释放之前通过bsp_display_lock()获取的LVGL互斥锁。
 * 必须与bsp_display_lock()成对使用。
 */
void bsp_display_unlock(void)
{
    lvgl_port_unlock();
}

/**
 * @brief 初始化电源管理
 * 
 * 配置电源控制GPIO，可以控制系统的整体电源开关。
 * 主要用于系统的软件关机功能。
 * @param power_en 电源控制标志
 *                 1 = 启用电源（系统正常运行）
 *                 0 = 关闭电源（系统关机）
 * @return 
 *      - ESP_OK: 初始化成功
 *      - ESP_ERR_*: 初始化失败的具体错误码
 */
esp_err_t bsp_power_init(uint8_t power_en)
{
    /* 配置电源控制GPIO为输出模式 */
    gpio_config_t power_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,              // 输出模式
        .pin_bit_mask = BIT64(BSP_POWER_OFF),  // 电源控制引脚
    };
    ESP_ERROR_CHECK(gpio_config(&power_gpio_config));

    /* 设置电源控制信号电平 */
    gpio_set_level(BSP_POWER_OFF, power_en);

    return ESP_OK;
}

/**
 * @brief 检测PCB版本并获取版本差异信息
 * 
 * 通过I2C总线探测特定地址的设备来自动检测PCB版本。
 * 不同版本的PCB在GPIO引脚分配上有差异，检测结果用于
 * 自动配置正确的引脚映射。
 * 
 * 检测逻辑：
 * 1. 先尝试探测I2C地址0x18，如果成功则为V1.0版本
 * 2. 如果失败，则配置GPIO48并重试，成功则为V1.2版本
 * 3. 如果都失败，则报告检测错误
 * 
 * @param info 返回PCB版本信息的指针，可以为NULL
 * 
 * @return 
 *      - ESP_OK: 检测成功
 *      - ESP_FAIL: 检测失败
 */
esp_err_t bsp_pcb_version_detect(bsp_pcd_diff_info_t *info)
{
    /* 如果已经检测过版本，直接返回缓存的结果 */
    if (pcd_info_initialized) {
        if (info) {
            *info = pcd_info;
        }
        return ESP_OK;
    }

    /* 确保I2C总线已初始化 */
    BSP_ERROR_CHECK_RETURN_ERR(bsp_i2c_init());

    bsp_pcd_diff_info_t temp_info = {0};
    
    /* 第一步：直接探测I2C设备（V1.0版本测试） */
    esp_err_t ret = i2c_master_probe(bsp_i2c_get_handle(), 0x18, 100);
    if (ret == ESP_OK) {
        /* 检测到V1.0版本PCB */
        ESP_LOGI(TAG, "Detect PCB version V1.0");
        temp_info.version = BSP_PCB_VERSION_V1_0;
        temp_info.audio.i2s_din_pin = BSP_I2S_DSIN_V1_0;    // V1.0音频输入引脚
        temp_info.audio.pa_pin = BSP_POWER_AMP_IO_V1_0;     // V1.0功放控制引脚
        temp_info.touch.pad2_pin = BSP_TOUCH_PAD2_V1_0;     // V1.0触控扩展引脚
        temp_info.uart.tx_pin = BSP_UART1_TX_V1_0;          // V1.0串口发送引脚
        temp_info.uart.rx_pin = BSP_UART1_RX_V1_0;          // V1.0串口接收引脚
        temp_info.lcd.rst_pin = BSP_LCD_RST_V1_0;           // V1.0 LCD复位引脚
        temp_info.lcd.rst_active_level = 0;                 // V1.0复位信号低电平有效
    } else {
        /* V1.0检测失败，尝试V1.2版本检测 */
        /* 配置GPIO48为输出模式并拉高（V1.2版本需要） */
        gpio_config_t gpio_conf = {
            .pin_bit_mask = (1ULL << GPIO_NUM_48),  // GPIO48引脚
            .mode = GPIO_MODE_OUTPUT,               // 输出模式
            .pull_up_en = GPIO_PULLUP_DISABLE,      // 禁用上拉
            .pull_down_en = GPIO_PULLDOWN_DISABLE,  // 禁用下拉
            .intr_type = GPIO_INTR_DISABLE          // 禁用中断
        };
        ESP_ERROR_CHECK(gpio_config(&gpio_conf));
        ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_48, 1));  // 拉高GPIO48

        vTaskDelay(pdMS_TO_TICKS(100));  // 等待100ms稳定

        /* 重新探测I2C设备（V1.2版本测试） */
        ret = i2c_master_probe(bsp_i2c_get_handle(), 0x18, 100);
        if (ret == ESP_OK) {
            /* 检测到V1.2版本PCB */
            ESP_LOGI(TAG, "Detect PCB version V1.2");
            temp_info.version = BSP_PCB_VERSION_V1_2;
            temp_info.audio.i2s_din_pin = BSP_I2S_DSIN_V1_2;    // V1.2音频输入引脚
            temp_info.audio.pa_pin = BSP_POWER_AMP_IO_V1_2;     // V1.2功放控制引脚
            temp_info.touch.pad2_pin = BSP_TOUCH_PAD2_V1_2;     // V1.2触控扩展引脚
            temp_info.uart.tx_pin = BSP_UART1_TX_V1_2;          // V1.2串口发送引脚
            temp_info.uart.rx_pin = BSP_UART1_RX_V1_2;          // V1.2串口接收引脚
            temp_info.lcd.rst_pin = BSP_LCD_RST_V1_2;           // V1.2 LCD复位引脚
            temp_info.lcd.rst_active_level = 1;                 // V1.2复位信号高电平有效
        } else {
            /* 两个版本都检测失败 */
            ESP_LOGE(TAG, "PCB version detection error");
        }
    }

    /* 保存检测结果 */
    if (info) {
        *info = temp_info;  // 返回检测到的版本信息
    }
    pcd_info = temp_info;         // 缓存到全局变量
    pcd_info_initialized = true;  // 标记为已初始化

    return ESP_OK;
}

/**
 * @brief 获取SD卡设备句柄
 * 
 * 返回当前已挂载的SD卡设备句柄，可用于访问SD卡信息。
 * 
 * @return SD卡设备句柄，未挂载时返回NULL
 */
sdmmc_card_t *bsp_sdcard_get_handle(void)
{
    return bsp_sdcard;
}

/**
 * @brief 获取SD卡SDMMC主机配置
 * 
 * 填充SDMMC主机的默认配置参数，用于SD卡挂载。
 * 
 * @param slot SD卡插槽号（当前未使用）
 * @param config 将被填充的SDMMC主机配置结构体指针
 */
void bsp_sdcard_get_sdmmc_host(const int slot, sdmmc_host_t *config)
{
    assert(config);

    /* 获取SDMMC主机默认配置 */
    sdmmc_host_t host_config = SDMMC_HOST_DEFAULT();

    /* 复制配置到用户提供的结构体 */
    memcpy(config, &host_config, sizeof(sdmmc_host_t));
}

/**
 * @brief 获取SD卡SDMMC插槽配置
 * 
 * 配置SDMMC插槽的GPIO引脚分配和工作参数。
 * EchoEar使用1位数据宽度模式连接SD卡。
 * 
 * @param slot SD卡插槽号（固定使用插槽0）
 * @param config 将被填充的SDMMC插槽配置结构体指针
 */
void bsp_sdcard_sdmmc_get_slot(const int slot, sdmmc_slot_config_t *config)
{
    assert(config);

    /* 获取SDMMC插槽默认配置 */
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    /* 配置SD卡连接到插槽0的引脚，插槽0使用IO MUX，不需要单独指定引脚 */
    slot_config.cmd = BSP_SD_CMD;   // SD卡命令线
    slot_config.clk = BSP_SD_CLK;   // SD卡时钟线
    slot_config.d0 = BSP_SD_D0;     // SD卡数据线0
    slot_config.width = 1;          // 1位数据宽度模式
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP; // 启用内部上拉电阻

    *config = slot_config;
}

/**
 * @brief 使用自定义配置挂载SD卡（SDMMC模式）
 * 
 * 使用用户提供的配置参数将microSD卡挂载到虚拟文件系统。
 * 支持自定义挂载点、主机参数、插槽配置等。
 * 
 * @param cfg SD卡配置参数结构体，包含挂载、主机、插槽配置
 * 
 * @return 
 *      - ESP_OK: 挂载成功
 *      - ESP_ERR_*: 挂载失败的具体错误码
 */
esp_err_t bsp_sdcard_sdmmc_mount(bsp_sdcard_cfg_t *cfg)
{
    sdmmc_host_t sdhost = {0};
    sdmmc_slot_config_t sdslot = {0};
    
    /* 默认挂载配置 */
    const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_BSP_SD_FORMAT_ON_MOUNT_FAIL
        .format_if_mount_failed = true,   // 挂载失败时格式化SD卡
#else
        .format_if_mount_failed = false,  // 挂载失败时不格式化
#endif
        .max_files = 5,                   // 最大同时打开文件数
        .allocation_unit_size = 16 * 1024 // 分配单元大小：16KB
    };
    assert(cfg);

    /* 如果用户未提供挂载配置，使用默认配置 */
    if (!cfg->mount) {
        cfg->mount = &mount_config;
    }

    /* 如果用户未提供主机配置，使用默认配置 */
    if (!cfg->host) {
        bsp_sdcard_get_sdmmc_host(SDMMC_HOST_SLOT_0, &sdhost);
        cfg->host = &sdhost;
    }

    /* 如果用户未提供插槽配置，使用默认配置 */
    if (!cfg->slot.sdmmc) {
        bsp_sdcard_sdmmc_get_slot(SDMMC_HOST_SLOT_0, &sdslot);
        cfg->slot.sdmmc = &sdslot;
    }

#if CONFIG_FATFS_LFN_NONE
    ESP_LOGW(TAG, "Warning: Long filenames on SD card are disabled in menuconfig!");
#endif

    /* 执行SD卡挂载 */
    return esp_vfs_fat_sdmmc_mount(BSP_SD_MOUNT_POINT, cfg->host, cfg->slot.sdmmc, cfg->mount, &bsp_sdcard);
}

/**
 * @brief 挂载SD卡（使用默认配置）
 * 
 * 使用预设的默认配置将microSD卡挂载到虚拟文件系统。
 * 这是bsp_sdcard_sdmmc_mount()的简化版本。
 * 
 * @return 
 *      - ESP_OK: 挂载成功
 *      - ESP_ERR_*: 挂载失败的具体错误码
 */
esp_err_t bsp_sdcard_mount(void)
{
    bsp_sdcard_cfg_t cfg = {0};  // 使用零初始化，函数内部会填充默认值
    return bsp_sdcard_sdmmc_mount(&cfg);
}

/**
 * @brief 卸载SD卡
 * 
 * 从虚拟文件系统中卸载microSD卡，释放相关资源。
 * 卸载后SD卡将无法访问，直到重新挂载。
 * 
 * @return 
 *      - ESP_OK: 卸载成功
 *      - ESP_ERR_*: 卸载失败的具体错误码
 */
esp_err_t bsp_sdcard_unmount(void)
{
    esp_err_t ret = ESP_OK;

    /* 卸载SD卡文件系统 */
    ret |= esp_vfs_fat_sdcard_unmount(BSP_SD_MOUNT_POINT, bsp_sdcard);
    bsp_sdcard = NULL;  // 清空全局SD卡句柄

    return ret;
}
