/*
 * SPDX发牌文本：2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX许可证标识符：CC0-1.0
 * 
 * USB大容量存储设备(MSC)实现文件
 * 
 * 本文件实现了USB Mass Storage Class功能，使ESP32设备能够：
 * - 作为USB存储设备被电脑识别和访问
 * - 通过USB接口暴露内部存储（SD卡等）给主机
 * - 支持标准的文件系统操作（读写、格式化等）
 * 
 * 技术实现：
 * - 基于TinyUSB库的MSC类实现
 * - 支持SCSI命令集（READ10、WRITE10、INQUIRY等）
 * - 集成ESP-IDF的磁盘IO子系统
 * - 实现了完整的USB设备状态管理
 * 
 * 应用场景：
 * - 开发者模式下的文件传输和管理
 * - 音频资源和配置文件的便捷更新
 * - 系统调试和日志文件导出
 */
#include "diskio.h"
#include "tinyusb.h"
#include "tusb_msc_storage.h"
#include "esp_check.h"
#include "usb_msc.h"

// 日志标签
static const char *TAG = "usb_msc";

// USB MSC相关全局变量
static uint8_t s_pdrv = 0;              // 物理驱动器编号
static int s_disk_block_size = 0;       // 磁盘块大小（字节）

// 逻辑磁盘管理
#define LOGICAL_DISK_NUM 1              // 支持的逻辑磁盘数量
static bool ejected[LOGICAL_DISK_NUM] = {true};  // 磁盘弹出状态数组

/**
 * @brief 挂载USB大容量存储设备（实现函数）
 * 
 * 该函数初始化TinyUSB驱动并启用MSC功能，执行以下步骤：
 * 1. 使用默认配置初始化TinyUSB栈
 * 2. 注册MSC类驱动程序
 * 3. 启动USB设备枚举过程
 * 
 * 初始化完成后，设备将：
 * - 在USB总线上枚举为大容量存储设备
 * - 响应主机的SCSI命令
 * - 提供对底层存储介质的访问
 * 
 * @return esp_err_t 
 *         - ESP_OK: USB MSC初始化成功
 *         - 其他: TinyUSB驱动安装失败
 */
esp_err_t usb_msc_mount(void)
{
    ESP_LOGI(TAG, "USB MSC initialization");

    // 使用默认配置初始化TinyUSB
    const tinyusb_config_t tusb_cfg = {0};

    ESP_RETURN_ON_ERROR(tinyusb_driver_install(&tusb_cfg), TAG, "tinyusb_driver_install failed");

    return ESP_OK;
}

//--------------------------------------------------------------------+
// TinyUSB回调函数集合
// 这些函数由TinyUSB库在特定USB事件发生时调用，用于处理USB设备的状态变化
//--------------------------------------------------------------------+

/**
 * @brief USB设备挂载回调函数
 * 
 * 当USB设备成功连接到主机并完成枚举过程时，TinyUSB会调用此函数。
 * 该回调用于重置设备状态，特别是磁盘弹出状态，以确保设备可以被正常访问。
 * 
 * 功能说明：
 * - 每次USB连接时都会重置弹出状态
 * - 支持设备从电池供电模式重新连接后的正常使用
 * - 确保主机能够重新访问存储设备
 */
void tud_mount_cb(void)
{
    // 重置弹出跟踪状态，允许重新连接后正常使用存储设备
    // 这支持设备在电池供电、弹出、拔出后重新插入的场景
    for (uint8_t i = 0; i < LOGICAL_DISK_NUM; i++) {
        ejected[i] = false;
    }

    ESP_LOGI(__func__, "");
}

/**
 * @brief USB设备卸载回调函数
 * 
 * 当USB设备从主机断开连接时，TinyUSB会调用此函数。
 * 可以在此处理断开连接时的清理工作。
 */
void tud_umount_cb(void)
{
    ESP_LOGW(__func__, "");
}

/**
 * @brief USB总线挂起回调函数
 * 
 * 当USB总线进入挂起状态时调用。根据USB规范，
 * 设备必须在7ms内将平均电流降至2.5mA以下。
 * 
 * @param remote_wakeup_en 主机是否允许设备执行远程唤醒
 */
void tud_suspend_cb(bool remote_wakeup_en)
{
    ESP_LOGW(__func__, "");
}

/**
 * @brief USB总线恢复回调函数
 * 
 * 当USB总线从挂起状态恢复时调用，设备可以恢复正常功耗。
 */
void tud_resume_cb(void)
{
    ESP_LOGW(__func__, "");
}

/**
 * @brief WRITE10命令完成回调函数
 * 
 * 当WRITE10 SCSI命令完成（状态被主机接收并接受）时调用。
 * 通常用于刷新任何待处理的缓存数据。
 * 
 * @param lun 逻辑单元号
 */
void tud_msc_write10_complete_cb(uint8_t lun)
{
    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return;
    }

    // 写入完成，可以启动自动重载时钟
    ESP_LOGD(__func__, "");
}

/**
 * @brief 检查逻辑磁盘是否被弹出
 * 
 * 该函数检查所有逻辑磁盘的弹出状态。
 * 只有当所有磁盘都被弹出时才返回true。
 * 
 * @return bool true表示所有磁盘都已弹出，false表示至少有一个磁盘未弹出
 */
static bool _logical_disk_ejected(void)
{
    bool all_ejected = true;

    for (uint8_t i = 0; i < LOGICAL_DISK_NUM; i++) {
        all_ejected &= ejected[i];
    }

    return all_ejected;
}

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return;
    }

    const char vid[] = "ESP";
    const char pid[] = "Mass Storage";
    const char rev[] = "1.0";

    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return false;
    }

    if (_logical_disk_ejected()) {
        // Set 0x3a for media not present.
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
        return false;
    }

    return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return;
    }

    disk_ioctl(s_pdrv, GET_SECTOR_COUNT, block_count);
    disk_ioctl(s_pdrv, GET_SECTOR_SIZE, block_size);
    s_disk_block_size = *block_size;
    ESP_LOGD(__func__, "GET_SECTOR_COUNT = %"PRIu32"，GET_SECTOR_SIZE = %d", *block_count, *block_size);
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return false;
    }

    return true;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    ESP_LOGI(__func__, "");
    (void) power_condition;

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return false;
    }

    if (load_eject) {
        if (!start) {
            // Eject but first flush.
            if (disk_ioctl(s_pdrv, CTRL_SYNC, NULL) != RES_OK) {
                return false;
            } else {
                ejected[lun] = true;
            }
        } else {
            // We can only load if it hasn't been ejected.
            return !ejected[lun];
        }
    } else {
        if (!start) {
            // Stop the unit but don't eject.
            if (disk_ioctl(s_pdrv, CTRL_SYNC, NULL) != RES_OK) {
                return false;
            }
        }

        // Always start the unit, even if ejected. Whether media is present is a separate check.
    }

    return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return 0;
    }

    const uint32_t block_count = bufsize / s_disk_block_size;
    disk_read(s_pdrv, buffer, lba, block_count);
    return block_count * s_disk_block_size;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    ESP_LOGD(__func__, "");
    (void) offset;

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return 0;
    }

    const uint32_t block_count = bufsize / s_disk_block_size;
    disk_write(s_pdrv, buffer, lba, block_count);
    return block_count * s_disk_block_size;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    // read10 & write10 has their own callback and MUST not be handled here
    ESP_LOGD(__func__, "");

    if (lun >= LOGICAL_DISK_NUM) {
        ESP_LOGE(__func__, "invalid lun number %u", lun);
        return 0;
    }

    void const *response = NULL;
    uint16_t resplen = 0;

    // most scsi handled is input
    bool in_xfer = true;

    switch (scsi_cmd[0]) {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        // Host is about to read/write etc ... better not to disconnect disk
        resplen = 0;
        break;

    default:
        // Set Sense = Invalid Command Operation
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

        // negative means error -> tinyusb could stall and/or response with failed status
        resplen = -1;
        break;
    }

    // return resplen must not larger than bufsize
    if (resplen > bufsize) {
        resplen = bufsize;
    }

    if (response && (resplen > 0)) {
        if (in_xfer) {
            memcpy(buffer, response, resplen);
        } else {
            // SCSI output
        }
    }

    return resplen;
}
/*********************************************************************** TinyUSB MSC callbacks*/
