/*
 * SPDX发牌文本：2022-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX许可证标识符：Unlicense OR CC0-1.0
 * 
 * Coze智能代理配置管理实现文件
 * 
 * 本文件实现了Coze AI平台智能代理配置的加载和管理功能，主要包括：
 * - 从SD卡读取机器人配置JSON文件和私钥PEM文件
 * - 解析JSON格式的机器人配置信息（ID、语音、名称、描述等）
 * - 管理多个AI机器人的配置参数
 * - 提供内存安全的资源释放机制
 * 
 * 配置文件结构：
 * - /sdcard/bot_setting.json: 包含机器人基本信息和平台认证参数
 * - /sdcard/private_key.pem: Coze平台的私钥文件，用于API认证
 * 
 * 支持特性：
 * - 动态解析任意数量的机器人配置（最多MAX_BOT_NUM个）
 * - 错误处理和资源清理
 * - 字符串安全处理，防止空指针访问
 */

#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include "esp_partition.h"
#include "esp_check.h"
#include "mbedtls/aes.h"
#include "cJSON.h"
#include "bsp/esp-bsp.h"
#include "coze_agent_config.h"

// 配置文件路径定义
#define BASE_PATH                  BSP_SPIFFS_MOUNT_POINT          // SPIFFS挂载点基础路径
#define PRIVATE_KEY_PATH           BASE_PATH "/private_key.pem"    // 私钥文件路径
#define BOT_SETTING_PATH           BASE_PATH "/bot_setting.json"   // 机器人配置文件路径

// 日志标签
static const char *TAG = "coze_agent_config";

/**
 * @brief 字符串安全复制函数
 * 
 * 该函数确保即使输入字符串为NULL，也会返回一个有效的空字符串，
 * 避免在后续处理中出现空指针访问错误。
 * 
 * @param s 输入字符串指针
 * @return char* 复制后的字符串指针（如果输入为NULL则返回空字符串）
 */
static char *strdup_or_empty(const char *s)
{
    return s ? strdup(s) : strdup("");
}

/**
 * @brief 解析机器人配置JSON文件
 * 
 * 该函数读取并解析包含机器人配置信息的JSON文件，提取以下信息：
 * - 平台认证信息：public_key（公钥）、appid（应用ID）
 * - 机器人数组：每个机器人的ID、语音ID、名称和描述
 * 
 * JSON文件格式示例：
 * {
 *   "public_key": "your_public_key",
 *   "appid": "your_app_id",
 *   "bots": [
 *     {
 *       "bot_id": "bot1_id",
 *       "voice_id": "voice1_id", 
 *       "bot_name": "机器人1",
 *       "description": "第一个机器人的描述"
 *     }
 *   ]
 * }
 * 
 * @param filename JSON配置文件的完整路径
 * @param cfg 用于存储解析结果的配置结构体指针
 * @return esp_err_t 
 *         - ESP_OK: 解析成功
 *         - ESP_ERR_NOT_FOUND: 文件不存在或读取失败
 *         - ESP_ERR_INVALID_ARG: JSON格式错误
 */
static esp_err_t parse_bot_json(const char *filename, coze_agent_config_t *cfg)
{
    // 打开并读取JSON文件
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return ESP_ERR_NOT_FOUND;
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    // 分配内存并读取文件内容
    char *json_data = malloc(len + 1);
    if (!json_data) {
        fclose(fp);
        return ESP_ERR_NOT_FOUND;
    }
    fread(json_data, 1, len, fp);
    fclose(fp);

    json_data[len] = '\0';  // 添加字符串结束符

    // 解析JSON数据
    cJSON *root = cJSON_Parse(json_data);
    free(json_data);

    if (!root) {
        fprintf(stderr, "JSON parse error: %s\n", cJSON_GetErrorPtr());
        return ESP_ERR_INVALID_ARG;
    }

    // 解析平台认证信息
    cJSON *public_key = cJSON_GetObjectItem(root, "public_key");
    cfg->public_key = strdup_or_empty(cJSON_IsString(public_key) ? public_key->valuestring : "");

    cJSON *appid = cJSON_GetObjectItem(root, "appid");
    cfg->appid = strdup_or_empty(cJSON_IsString(appid) ? appid->valuestring : "");
    
    // 解析机器人数组
    cJSON *bots = cJSON_GetObjectItem(root, "bots");
    if (!cJSON_IsArray(bots)) {
        cJSON_Delete(root);
        return ESP_ERR_INVALID_ARG;
    }

    // 检查机器人数量限制
    int count = cJSON_GetArraySize(bots);
    if (count > MAX_BOT_NUM) {
        count = MAX_BOT_NUM;
        ESP_LOGW(TAG, "Too many bots, only %d bots will be used", count);
    }
    cfg->bot_num = count;

    // 逐个解析机器人配置
    for (int i = 0; i < count; i++) {
        cJSON *bot = cJSON_GetArrayItem(bots, i);
        if (!cJSON_IsObject(bot)) {
            continue;
        }

        // 提取机器人的各项配置信息
        cJSON *id    = cJSON_GetObjectItem(bot, "bot_id");       // 机器人ID
        cJSON *vid   = cJSON_GetObjectItem(bot, "voice_id");     // 语音ID
        cJSON *name  = cJSON_GetObjectItem(bot, "bot_name");     // 机器人名称
        cJSON *desc  = cJSON_GetObjectItem(bot, "description");  // 机器人描述

        // 安全地复制字符串数据到配置结构体
        cfg->bot[i].bot_id         = strdup_or_empty(cJSON_IsString(id)   ? id->valuestring   : "");
        cfg->bot[i].voice_id       = strdup_or_empty(cJSON_IsString(vid)  ? vid->valuestring  : "");
        cfg->bot[i].bot_name       = strdup_or_empty(cJSON_IsString(name) ? name->valuestring : "");
        cfg->bot[i].bot_description = strdup_or_empty(cJSON_IsString(desc) ? desc->valuestring : "");
    }

    // 清理JSON解析器资源
    cJSON_Delete(root);

    return ESP_OK;
}

/**
 * @brief 解析私钥PEM文件
 * 
 * 该函数读取Coze平台的私钥文件，用于API认证和安全通信。
 * 私钥文件通常为PEM格式，包含RSA或其他加密算法的私钥信息。
 * 
 * @param file_path 私钥文件的完整路径
 * @param config 用于存储私钥内容的配置结构体指针
 * @return esp_err_t 
 *         - ESP_OK: 私钥读取成功
 *         - ESP_ERR_NOT_FOUND: 文件不存在
 *         - ESP_ERR_NO_MEM: 内存分配失败
 */
static esp_err_t parse_private_key(const char *file_path, coze_agent_config_t *config)
{
    // 打开私钥文件
    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        ESP_LOGE(TAG, "Failed to open private key file: %s", file_path);
        return ESP_ERR_NOT_FOUND;
    }
    
    // 获取文件大小
    int private_key_len = 0;
    fseek(fp, 0, SEEK_END);
    private_key_len = ftell(fp);

    rewind(fp);
    
    // 分配内存并读取私钥内容
    char *private_key = malloc(private_key_len + 1);
    if (!private_key) {
        fclose(fp);
        return ESP_ERR_NO_MEM;
    }
    fread(private_key, 1, private_key_len, fp);
    fclose(fp);

    private_key[private_key_len] = '\0';  // 添加字符串结束符
    config->private_key = strdup_or_empty(private_key);
    free(private_key);

    return ESP_OK;
}

/**
 * @brief 检查文件是否存在
 * 
 * 该函数通过尝试打开文件来判断文件是否存在且可读。
 * 
 * @param path 要检查的文件路径
 * @return bool true表示文件存在，false表示文件不存在
 */
static bool check_if_file_exists(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return false;
    }
    fclose(fp);
    return true;
}

/**
 * @brief 读取Coze智能代理配置信息（实现函数）
 * 
 * 该函数是配置读取的主入口，执行以下步骤：
 * 1. 检查SD卡上是否存在必要的配置文件
 * 2. 解析机器人配置JSON文件
 * 3. 读取私钥PEM文件
 * 4. 验证配置完整性
 * 
 * 配置文件要求：
 * - /sdcard/bot_setting.json: 机器人配置文件
 * - /sdcard/private_key.pem: Coze平台私钥文件
 * 
 * @param config 用于存储配置信息的结构体指针
 * @return esp_err_t 
 *         - ESP_OK: 配置读取成功
 *         - ESP_ERR_NOT_FOUND: 配置文件不存在或机器人数量为0
 */
esp_err_t coze_agent_config_read(coze_agent_config_t *config)
{
    const char *bot_setting_path = NULL;
    const char *private_key_path = NULL;

    // 检查配置文件是否存在
    if (check_if_file_exists(BOT_SETTING_PATH) && check_if_file_exists(PRIVATE_KEY_PATH)) {
        bot_setting_path = BOT_SETTING_PATH;
        private_key_path = PRIVATE_KEY_PATH;
        ESP_LOGI(TAG, "Using bot setting and private key files");
    } else {
        ESP_LOGE(TAG, "Missing bot setting or private key file");
        return ESP_ERR_NOT_FOUND;
    }

    // 解析机器人配置文件
    esp_err_t err = parse_bot_json(bot_setting_path, config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to parse bot setting: %s", bot_setting_path);
        return err;
    }

    // 验证机器人配置数量
    if (config->bot_num == 0) {
        ESP_LOGW(TAG, "No bot config found");
        return ESP_ERR_NOT_FOUND;
    }

    // 读取私钥文件
    err = parse_private_key(private_key_path, config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to parse private key: %s", private_key_path);
        return err;
    }

    // 调试输出（已注释）- 用于开发阶段验证配置读取结果
    // ESP_LOGI(TAG, "appid: %s\n", config->appid);
    // ESP_LOGI(TAG, "public_key: %s\n", config->public_key);
    // ESP_LOGI(TAG, "bot_num: %d\n", config->bot_num);
    // for (int i = 0; i < config->bot_num; i++) {
    //     ESP_LOGI(
    //         TAG, "bot[%d]:bot_id %s, voice_id %s, bot_name %s\n", i, config->bot[i].bot_id, config->bot[i].voice_id,
    //         config->bot[i].bot_name
    //     );
    //     ESP_LOGI(TAG, "bot[%d]:bot_description %s\n", i, config->bot[i].bot_description);
    // }

    return ESP_OK;
}

/**
 * @brief 释放Coze智能代理配置资源（实现函数）
 * 
 * 该函数释放coze_agent_config_read()函数分配的所有内存资源，
 * 包括各种字符串缓冲区。按照以下顺序进行资源清理：
 * 1. 释放每个机器人的配置字符串（ID、语音ID、名称、描述）
 * 2. 释放平台认证信息字符串（公钥、私钥）
 * 3. 将所有指针设置为NULL，防止悬挂指针
 * 
 * 内存安全保证：
 * - 检查指针有效性后再释放
 * - 释放后立即置空指针
 * - 避免重复释放和内存泄漏
 * 
 * @param config 需要释放资源的配置结构体指针
 * @return esp_err_t 始终返回ESP_OK
 */
esp_err_t coze_agent_config_release(coze_agent_config_t *config)
{
    // 释放每个机器人的配置资源
    for (int i = 0; i < config->bot_num; i++) {
        if (config->bot[i].bot_id) {
            free(config->bot[i].bot_id);
            config->bot[i].bot_id = NULL;
        }
        if (config->bot[i].voice_id) {
            free(config->bot[i].voice_id);
            config->bot[i].voice_id = NULL;
        }
        if (config->bot[i].bot_name) {
            free(config->bot[i].bot_name);
            config->bot[i].bot_name = NULL;
        }
        if (config->bot[i].bot_description) {
            free(config->bot[i].bot_description);
            config->bot[i].bot_description = NULL;
        }
    }
    
    // 释放平台认证信息资源
    if (config->public_key) {
        free(config->public_key);
        config->public_key = NULL;
    }
    if (config->private_key) {
        free(config->private_key);
        config->private_key = NULL;
    }
    
    return ESP_OK;
}
