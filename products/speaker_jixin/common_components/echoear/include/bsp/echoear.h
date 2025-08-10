/*
 * SPDX-FileCopyrightText: 2021-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file echoear.h
 * @brief ESP-Brookesia EchoEar 智能音箱板级支持包 (BSP) 头文件
 *
 * 该文件定义了 EchoEar 智能音箱开发板的所有硬件接口、GPIO定义、
 * 外设配置和功能接口，包括显示、触控、音频、SD卡、I2C等模块的支持。
 */

#pragma once

#include "sdkconfig.h"
#include "soc/usb_pins.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "bsp/display.h"

/**************************************************************************************************
 *  BSP 硬件功能支持定义
 *
 *  定义 EchoEar 开发板支持的各种硬件功能模块
 *  1 = 支持该功能，0 = 不支持该功能
 **************************************************************************************************/

#define BSP_CAPS_DISPLAY 1       /*!< 显示屏支持：支持LCD显示功能 */
#define BSP_CAPS_TOUCH 1         /*!< 触控支持：支持电容触摸屏 */
#define BSP_CAPS_BUTTONS 0       /*!< 按键支持：当前版本不支持物理按键 */
#define BSP_CAPS_AUDIO 0         /*!< 音频支持：当前版本音频功能未在BSP层启用 */
#define BSP_CAPS_AUDIO_SPEAKER 0 /*!< 扬声器支持：当前版本扬声器功能未在BSP层启用 */
#define BSP_CAPS_AUDIO_MIC 0     /*!< 麦克风支持：当前版本麦克风功能未在BSP层启用 */
#define BSP_CAPS_SDCARD 1        /*!< SD卡支持：支持microSD卡存储 */
#define BSP_CAPS_IMU 0           /*!< 惯性测量单元支持：当前版本IMU功能未启用 */

/* I2C 总线配置
 * EchoEar开发板使用I2C总线连接多个外设设备
 */
#define BSP_I2C_SCL (GPIO_NUM_1) /*!< I2C时钟线 (SCL) GPIO引脚 */
#define BSP_I2C_SDA (GPIO_NUM_2) /*!< I2C数据线 (SDA) GPIO引脚 */

/* 音频系统接口配置
 * EchoEar智能音箱的音频系统采用I2S数字音频接口
 * 连接ES8311 DAC编解码器和ES7210 ADC模数转换器
 */
#define BSP_I2S_SCLK (GPIO_NUM_40)          /*!< I2S位时钟 (BCLK) - 数字音频位同步时钟 */
#define BSP_I2S_MCLK (GPIO_NUM_42)          /*!< I2S主时钟 (MCLK) - 音频编解码器主时钟 */
#define BSP_I2S_LCLK (GPIO_NUM_39)          /*!< I2S帧时钟 (WS/LRCLK) - 左右声道选择时钟 */
#define BSP_I2S_DOUT (GPIO_NUM_41)          /*!< I2S数据输出 - 连接到ES8311编解码器 */
#define BSP_I2S_DSIN_V1_0 (GPIO_NUM_15)     /*!< I2S数据输入 (V1.0版本) - 来自ES7210 ADC */
#define BSP_I2S_DSIN_V1_2 (GPIO_NUM_3)      /*!< I2S数据输入 (V1.2版本) - 来自ES7210 ADC */
#define BSP_POWER_AMP_IO_V1_0 (GPIO_NUM_4)  /*!< 功率放大器使能 (V1.0版本) - 控制扬声器放大器 */
#define BSP_POWER_AMP_IO_V1_2 (GPIO_NUM_15) /*!< 功率放大器使能 (V1.2版本) - 控制扬声器放大器 */

/* 显示系统接口配置
 * EchoEar开发板配备2.4寸彩色LCD显示屏，使用SPI接口通信
 * 支持触控功能，采用CST816S触控芯片
 */
#define BSP_LCD_DATA3 (GPIO_NUM_12)           /*!< LCD SPI数据线3 (SPI QSPI模式) */
#define BSP_LCD_DATA2 (GPIO_NUM_11)           /*!< LCD SPI数据线2 (SPI QSPI模式) */
#define BSP_LCD_DATA1 (GPIO_NUM_13)           /*!< LCD SPI数据线1 (MOSI) */
#define BSP_LCD_DATA0 (GPIO_NUM_46)           /*!< LCD SPI数据线0 (MISO) */
#define BSP_LCD_PCLK (GPIO_NUM_18)            /*!< LCD SPI时钟线 (SCLK) */
#define BSP_LCD_CS (GPIO_NUM_14)              /*!< LCD SPI片选信号 (CS) */
#define BSP_LCD_DC (GPIO_NUM_45)              /*!< LCD数据/命令选择信号 (DC) */
#define BSP_LCD_RST_V1_0 (GPIO_NUM_3)         /*!< LCD复位信号 (V1.0版本) */
#define BSP_LCD_RST_V1_2 (GPIO_NUM_47)        /*!< LCD复位信号 (V1.2版本) */
#define LCD_BACKLIIGHT_CHANNEL LEDC_CHANNEL_1 /*!< LCD背光PWM通道 (LEDC通道1) */
#define BSP_LCD_BACKLIGHT (GPIO_NUM_44)       /*!< LCD背光控制GPIO - PWM亮度调节 */
#define BSP_LCD_TOUCH_INT (GPIO_NUM_10)       /*!< 触控中断信号 - 检测触摸事件 */

/* 电源管理配置 */
#define BSP_POWER_OFF (GPIO_NUM_9) /*!< 系统电源关闭控制信号 */

/* SD卡存储接口配置
 * EchoEar支持microSD卡扩展存储，使用SDMMC接口
 * 可存储音乐文件、配置文件等数据
 */
#define BSP_SD_D0 (GPIO_NUM_17)  /*!< SD卡数据线0 (SDMMC_D0) */
#define BSP_SD_CMD (GPIO_NUM_38) /*!< SD卡命令线 (SDMMC_CMD) */
#define BSP_SD_CLK (GPIO_NUM_16) /*!< SD卡时钟线 (SDMMC_CLK) */

/* 其他接口配置
 * 包括调试UART、触控扩展接口等
 */
#define BSP_UART1_TX_V1_0 (GPIO_NUM_6)    /*!< UART1发送引脚 (V1.0版本) - 调试串口 */
#define BSP_UART1_TX_V1_2 (GPIO_NUM_5)    /*!< UART1发送引脚 (V1.2版本) - 调试串口 */
#define BSP_UART1_RX_V1_0 (GPIO_NUM_5)    /*!< UART1接收引脚 (V1.0版本) - 调试串口 */
#define BSP_UART1_RX_V1_2 (GPIO_NUM_4)    /*!< UART1接收引脚 (V1.2版本) - 调试串口 */
#define BSP_TOUCH_PAD2_V1_0 (GPIO_NUM_NC) /*!< 触控扩展接口2 (V1.0版本) - 未连接 */
#define BSP_TOUCH_PAD2_V1_2 (GPIO_NUM_6)  /*!< 触控扩展接口2 (V1.2版本) - 扩展触控 */

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief BSP 显示配置结构体
     *
     * 用于配置LVGL显示系统的各项参数，包括缓冲区大小、
     * 双缓冲设置、内存分配选项等。
     */
    typedef struct{
        lvgl_port_cfg_t lvgl_port_cfg; /*!< LVGL端口配置参数 */
        uint32_t buffer_size;          /*!< 屏幕缓冲区大小（以像素为单位） */
        bool double_buffer;            /*!< 是否启用双缓冲：true=双缓冲，false=单缓冲 */
        struct{
            unsigned int buff_dma : 1;           /*!< 分配的LVGL缓冲区是否支持DMA：1=支持DMA访问 */
            unsigned int buff_spiram : 1;        /*!< 分配的LVGL缓冲区是否使用PSRAM：1=使用外部PSRAM */
            unsigned int default_dummy_draw : 1; /*!< 使用虚拟绘制绕过显示驱动：1=启用虚拟绘制 */
        } flags;                                 /*!< 显示配置标志位 */
    } bsp_display_cfg_t;

/**************************************************************************************************
 *
 * I2C 总线接口
 *
 * EchoEar开发板通过I2C总线连接多个外设设备：
 *  - 音频编解码器 ES8311 (仅用于配置)
 *  - 音频ADC ES7210 (仅用于配置)
 *  - 加密芯片 ATECC608A (大多数板子未贴装)
 *  - LCD触控控制器 CST816S
 *  - 惯性测量单元 ICM-42607-P (硬件预留，软件未启用)
 *
 * I2C初始化完成后，使用 BSP_I2C_NUM 宏创建I2C设备驱动，例如：
 * icm42670_handle_t imu = icm42670_create(BSP_I2C_NUM, ICM42670_I2C_ADDRESS);
 **************************************************************************************************/
#define BSP_I2C_NUM CONFIG_BSP_I2C_NUM /*!< I2C总线号，从配置文件中获取 */

    /**
     * @brief 初始化I2C驱动
     * 初始化I2C主机模式驱动，配置SCL和SDA引脚，设置I2C总线参数。
     * 初始化成功后，可以通过I2C总线与各种外设设备通信。
     * @return
     *      - ESP_OK                成功
     *      - ESP_ERR_INVALID_ARG   I2C参数错误
     *      - ESP_FAIL              I2C驱动安装失败
     */
    esp_err_t bsp_i2c_init(void);

    /**
     * @brief 反初始化I2C驱动并释放资源
     *
     * 停止I2C驱动并释放相关的内存和硬件资源。
     * 调用此函数后需要重新调用bsp_i2c_init()才能再次使用I2C功能。
     *
     * @return
     *      - ESP_OK                成功
     *      - ESP_ERR_INVALID_ARG   I2C参数错误
     *
     */
    esp_err_t bsp_i2c_deinit(void);

    /**
     * @brief 获取I2C驱动句柄
     *
     * 返回已初始化的I2C主机总线句柄，可用于创建I2C设备驱动。
     * 在调用此函数前必须先调用bsp_i2c_init()进行初始化。
     *
     * @return I2C主机总线句柄
     */
    i2c_master_bus_handle_t bsp_i2c_get_handle(void);

/**************************************************************************************************
 * LCD 显示接口
 * EchoEar智能音箱配备2.4寸ST7789显示控制器的彩色LCD屏幕。
 * 显示规格：16位色彩，320x240分辨率，支持电容触控功能。
 * 图形库使用LVGL，注意LVGL不是线程安全的，因此用户在调用任何LVGL API (lv_...)
 * 之前必须调用bsp_display_lock()获取LVGL互斥锁，使用完毕后调用bsp_display_unlock()释放互斥锁。
 * 显示屏背光需要通过调用bsp_display_brightness_set()显式启用。
 **************************************************************************************************/
#define BSP_LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000) /*!< LCD像素时钟频率：40MHz */
#define BSP_LCD_SPI_NUM (SPI2_HOST)               /*!< LCD使用的SPI主机编号：SPI2 */

    /**
     * @brief 初始化显示系统
     * 该函数初始化SPI接口、显示控制器并启动LVGL处理任务。
     * LCD背光需要单独通过bsp_display_brightness_set()函数启用。
     * @return 成功时返回LVGL显示对象指针，失败时返回NULL
     */
    lv_disp_t *bsp_display_start(void);

    /**
     * @brief 使用自定义配置初始化显示系统
     * 该函数初始化SPI接口、显示控制器并启动LVGL处理任务。
     * LCD背光需要单独通过bsp_display_brightness_set()函数启用。
     * @param cfg 显示配置参数结构体
     * @return 成功时返回LVGL显示对象指针，失败时返回NULL
     */
    lv_disp_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);

    /**
     * @brief 获取输入设备指针（触控、按键等）
     * 获取LVGL输入设备对象，用于处理触控事件。
     * 注意：LVGL输入设备在bsp_display_start()函数中初始化。

     * @return 成功时返回LVGL输入设备指针，未初始化时返回NULL
     */
    lv_indev_t *bsp_display_get_input_dev(void);

    /**
     * @brief 获取LVGL互斥锁
     *
     * 在调用任何LVGL API之前必须先获取此互斥锁，以确保线程安全。
     * 使用完毕后必须调用bsp_display_unlock()释放锁。
     *
     * @param timeout_ms 超时时间（毫秒）。0表示无限等待。
     * @return true  成功获取互斥锁
     * @return false 未能获取互斥锁（超时）
     */
    bool bsp_display_lock(uint32_t timeout_ms);

    /**
     * @brief 释放LVGL互斥锁
     *
     * 释放之前通过bsp_display_lock()获取的LVGL互斥锁。
     */
    void bsp_display_unlock(void);

    /**
     * @brief 初始化显示屏亮度控制
     *
     * 初始化LCD背光亮度控制功能，使用PWM（LEDC）控制背光亮度。
     * 调用此函数后可以通过相应的亮度设置函数调节屏幕亮度。
     *
     * @return
     *      - ESP_OK                成功
     *      - ESP_ERR_INVALID_ARG   参数错误
     *      - ESP_FAIL              初始化失败
     *
     */
    esp_err_t bsp_display_brightness_init(void);

    /**
     * @brief 初始化电源管理
     *
     * 初始化系统电源管理功能，配置电源控制GPIO。
     * 可以控制系统的电源开关状态。
     *
     * @param power_en 电源使能标志：1=启用电源，0=关闭电源
     *
     * @return
     *      - ESP_OK                成功
     *      - ESP_ERR_INVALID_ARG   参数错误
     *      - ESP_FAIL              初始化失败
     *
     */
    esp_err_t bsp_power_init(uint8_t power_en);

/**************************************************************************************************
 *
 * SD卡存储接口
 *
 * EchoEar智能音箱支持microSD卡扩展存储功能。SD卡挂载到虚拟文件系统后，
 * 可以使用标准的stdio函数进行文件操作，例如：
 * \code{.c}
 * FILE* f = fopen(BSP_SD_MOUNT_POINT"/hello.txt", "w");
 * fprintf(f, "Hello %s!\n", bsp_sdcard->cid.name);
 * fclose(f);
 * \endcode
 *
 * @attention 某些GPIO引脚可能与RGB LED和按键共用，使用时需注意冲突
 **************************************************************************************************/
#define BSP_SD_MOUNT_POINT CONFIG_BSP_SD_MOUNT_POINT /*!< SD卡在虚拟文件系统中的挂载点 */

    /**
     * @brief BSP SD卡配置结构体
     *
     * 用于配置SD卡的挂载参数、主机接口和插槽配置。
     */
    typedef struct
    {
        const esp_vfs_fat_sdmmc_mount_config_t *mount; /*!< VFS FAT挂载配置参数 */
        sdmmc_host_t *host;                            /*!< SDMMC主机配置 */
        union
        {
            const sdmmc_slot_config_t *sdmmc; /*!< SDMMC插槽配置参数 */
        } slot;                               /*!< 插槽配置联合体 */
    } bsp_sdcard_cfg_t;

    /**
     * @brief 挂载microSD卡到虚拟文件系统
     *
     * 使用默认配置将microSD卡挂载到虚拟文件系统。
     * 挂载成功后可以使用标准文件操作函数访问SD卡。
     *
     * @return
     *      - ESP_OK 成功
     *      - ESP_ERR_INVALID_STATE 如果esp_vfs_fat_sdmmc_mount已经被调用
     *      - ESP_ERR_NO_MEM 如果无法分配内存
     *      - ESP_FAIL 如果分区无法挂载
     *      - 其他来自SDMMC或SPI驱动、SDMMC协议或FATFS驱动的错误代码
     */
    esp_err_t bsp_sdcard_mount(void);

    /**
     * @brief 从虚拟文件系统卸载microSD卡
     *
     * 卸载之前挂载的microSD卡，释放相关资源。
     *
     * @return
     *      - ESP_OK 成功
     *      - ESP_ERR_NOT_FOUND 如果分区表不包含指定标签的FATFS分区
     *      - ESP_ERR_INVALID_STATE 如果esp_vfs_fat_spiflash_mount已经被调用
     *      - ESP_ERR_NO_MEM 如果无法分配内存
     *      - ESP_FAIL 如果分区无法挂载
     *      - 其他来自磨损均衡库、SPI flash驱动或FATFS驱动的错误代码
     */
    esp_err_t bsp_sdcard_unmount(void);

    /**
     * @brief 获取SD卡句柄
     *
     * 返回当前已挂载的SD卡设备句柄，可用于获取SD卡信息。
     *
     * @return SD卡设备句柄
     */
    sdmmc_card_t *bsp_sdcard_get_handle(void);

    /**
     * @brief 获取SD卡MMC主机配置
     *
     * 获取指定插槽的SDMMC主机配置参数。
     *
     * @param slot SD卡插槽号
     * @param config 将被填充的配置结构体指针
     */
    void bsp_sdcard_get_sdmmc_host(const int slot, sdmmc_host_t *config);

    /**
     * @brief 获取SD卡MMC插槽配置
     *
     * 获取指定插槽的SDMMC插槽配置参数。
     *
     * @param slot SD卡插槽号
     * @param config 将被填充的配置结构体指针
     */
    void bsp_sdcard_sdmmc_get_slot(const int slot, sdmmc_slot_config_t *config);

    /**
     * @brief 使用自定义配置挂载microSD卡到虚拟文件系统（MMC模式）
     *
     * 使用用户提供的配置参数将microSD卡挂载到虚拟文件系统。
     * 支持更灵活的配置选项，如自定义挂载点、主机参数等。
     *
     * @param cfg SD卡配置参数结构体
     *
     * @return
     *      - ESP_OK 成功
     *      - ESP_ERR_INVALID_STATE 如果esp_vfs_fat_sdmmc_mount已经被调用
     *      - ESP_ERR_NO_MEM 如果无法分配内存
     *      - ESP_FAIL 如果分区无法挂载
     *      - 其他来自SDMMC或SPI驱动、SDMMC协议或FATFS驱动的错误代码
     */
    esp_err_t bsp_sdcard_sdmmc_mount(bsp_sdcard_cfg_t *cfg);

    /**
     * @brief 获取SD卡句柄（重复定义）
     *
     * 注意：这是一个重复的函数声明，与上面的函数功能相同。
     * 返回当前已挂载的SD卡设备句柄。
     *
     * @return SD卡设备句柄
     */
    sdmmc_card_t *bsp_sdcard_get_handle(void);

    /**************************************************************************************************
     *
     * PCB 版本检测
     *
     * EchoEar开发板存在不同的PCB版本（V1.0和V1.2），各版本间某些GPIO引脚分配不同。
     * 通过版本检测功能可以自动识别当前使用的PCB版本，并应用相应的引脚配置。
     *
     **************************************************************************************************/
    /**
     * @brief PCB版本枚举
     *
     * 定义支持的PCB版本类型。
     */
    typedef enum
    {
        BSP_PCB_VERSION_V1_0 = 0, /*!< PCB版本V1.0 */
        BSP_PCB_VERSION_V1_2,     /*!< PCB版本V1.2 */
    } bsp_pcb_version_t;

    /**
     * @brief PCB版本差异信息结构体
     *
     * 包含不同PCB版本间的GPIO引脚差异配置，
     * 用于根据检测到的版本自动配置正确的引脚。
     */
    typedef struct
    {
        bsp_pcb_version_t version; /*!< 检测到的PCB版本 */
        struct
        {
            int i2s_din_pin; /*!< I2S数据输入引脚（版本相关） */
            int pa_pin;      /*!< 功率放大器控制引脚（版本相关） */
        } audio;             /*!< 音频相关引脚配置 */
        struct
        {
            int pad2_pin; /*!< 触控扩展引脚2（版本相关） */
        } touch;          /*!< 触控相关引脚配置 */
        struct
        {
            int tx_pin; /*!< UART发送引脚（版本相关） */
            int rx_pin; /*!< UART接收引脚（版本相关） */
        } uart;         /*!< UART相关引脚配置 */
        struct
        {
            int rst_pin;          /*!< LCD复位引脚（版本相关） */
            int rst_active_level; /*!< LCD复位信号有效电平（版本相关） */
        } lcd;                    /*!< LCD相关引脚配置 */
    } bsp_pcd_diff_info_t;

    /**
     * @brief 获取PCB版本信息
     *
     * 自动检测当前使用的PCB版本，并返回对应的引脚配置信息。
     * 检测成功后，应用程序可以根据返回的信息使用正确的GPIO引脚。
     *
     * @param info 指向PCB版本信息结构体的指针，将被填充检测结果
     *
     * @return
     *      - ESP_OK                成功检测到PCB版本
     *      - ESP_ERR_INVALID_ARG   参数无效
     *      - ESP_FAIL              PCB版本检测失败
     *
     */
    esp_err_t bsp_pcb_version_detect(bsp_pcd_diff_info_t *info);

#ifdef __cplusplus
}
#endif
