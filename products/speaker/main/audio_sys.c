/*
 * SPDX发牌文本：2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX许可证标识符：CC0-1.0
 * 
 * 音频系统工具集实现文件
 * 
 * 本文件实现了音频系统相关的工具函数，主要包括：
 * - 时间转换工具函数（毫秒到系统tick转换、系统时间获取）
 * - 系统CPU使用率统计功能（任务级别的性能监控）
 * 
 * 功能特点：
 * - 提供毫秒级时间处理接口，适用于音频时序控制
 * - 支持实时任务CPU使用率统计，用于系统性能调优
 * - 兼容FreeRTOS任务管理系统，支持双核处理器
 * - 内存安全检查机制，防止内存泄漏
 */
#include <sys/time.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "audio_sys.h"
#include "soc/soc_memory_layout.h"

// 音频系统日志标签
static const char *TAG = "AUDIO_SYS";

// CPU使用率统计相关定义
#define ARRAY_SIZE_OFFSET                   8   // 如果audio_sys_get_real_time_stats返回ESP_ERR_INVALID_SIZE，增加此值
#define AUDIO_SYS_TASKS_ELAPSED_TIME_MS  1000   // 统计测量周期（毫秒）

// 内存管理宏定义
#define audio_malloc    malloc
#define audio_free      free
#define AUDIO_MEM_CHECK(tag, ptr, check) if (ptr == NULL) { ESP_LOGE(tag, "Failed to allocate memory"); check; }

// 任务状态字符串数组（用于打印任务状态）
const char *task_state[] = {
    "Running",      // 运行中
    "Ready",        // 就绪
    "Blocked",      // 阻塞
    "Suspended",    // 挂起
    "Deleted"       // 已删除
};

/** 
 * @brief 任务堆栈类型标识
 * "Extr": 分配在PSRAM的任务堆栈, "Intr": 分配在内部RAM的任务堆栈
 */
const char *task_stack[] = {"Extr", "Intr"};

/**
 * @brief 将毫秒时间转换为系统tick数
 * 
 * 该函数用于将毫秒时间转换为FreeRTOS系统的tick数，
 * 便于在音频处理中进行精确的时间控制。
 * 
 * @param ms 毫秒时间值
 * @return int 对应的系统tick数
 */
int audio_sys_get_tick_by_time_ms(int ms)
{
    return (ms / portTICK_PERIOD_MS);
}

/**
 * @brief 获取当前系统时间（毫秒级）
 * 
 * 该函数使用gettimeofday()系统调用获取当前的UTC时间，
 * 并将其转换为毫秒级时间戳，用于音频系统的时间同步。
 * 
 * @return int64_t 当前时间的毫秒时间戳
 */
int64_t audio_sys_get_time_ms(void)
{
    struct timeval te;
    gettimeofday(&te, NULL);
    int64_t milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    return milliseconds;
}

/**
 * @brief 获取系统任务CPU使用率统计信息
 * 
 * 该函数通过两次获取系统任务状态（间隔1秒）来计算各任务的CPU使用率。
 * 实现原理：
 * 1. 获取当前所有任务的运行时间统计
 * 2. 延迟指定时间（AUDIO_SYS_TASKS_ELAPSED_TIME_MS）
 * 3. 再次获取任务统计信息
 * 4. 计算时间差值和CPU使用率百分比
 * 5. 按核心ID和使用率排序后打印结果
 * 
 * 输出信息包括：
 * - 任务名称、运行时间、CPU使用率百分比
 * - 任务优先级、高水位标记、任务状态
 * - 核心ID、堆栈类型（内部RAM/PSRAM）
 * 
 * @return esp_err_t 
 *         - ESP_OK: 统计成功
 *         - ESP_FAIL: 统计失败（内存分配失败或配置未启用）
 */
esp_err_t audio_sys_get_real_time_stats(void)
{
#if (CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID && CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS)
    TaskStatus_t *start_array = NULL, *end_array = NULL;
    UBaseType_t start_array_size, end_array_size;
    uint32_t start_run_time, end_run_time;
    uint32_t task_elapsed_time, percentage_time;
    esp_err_t ret;

    // 分配数组存储当前任务状态
    start_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    start_array = audio_malloc(sizeof(TaskStatus_t) * start_array_size);
    AUDIO_MEM_CHECK(TAG, start_array, {
        ret = ESP_FAIL;
        goto exit;
    });
    // 获取当前任务状态
    start_array_size = uxTaskGetSystemState(start_array, start_array_size, &start_run_time);
    if (start_array_size == 0) {
        ESP_LOGE(TAG, "Insufficient array size for uxTaskGetSystemState. Trying increasing ARRAY_SIZE_OFFSET");
        ret = ESP_FAIL;
        goto exit;
    }

    vTaskDelay(pdMS_TO_TICKS(AUDIO_SYS_TASKS_ELAPSED_TIME_MS));

    // 分配数组存储延迟后的任务状态
    end_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    end_array = audio_malloc(sizeof(TaskStatus_t) * end_array_size);
    AUDIO_MEM_CHECK(TAG, start_array, {
        ret = ESP_FAIL;
        goto exit;
    });

    // 获取延迟后的任务状态
    end_array_size = uxTaskGetSystemState(end_array, end_array_size, &end_run_time);
    if (end_array_size == 0) {
        ESP_LOGE(TAG, "Insufficient array size for uxTaskGetSystemState. Trying increasing ARRAY_SIZE_OFFSET");
        ret = ESP_FAIL;
        goto exit;
    }

    // 计算总运行时间
    uint32_t total_elapsed_time = (end_run_time - start_run_time);
    if (total_elapsed_time == 0) {
        ESP_LOGE(TAG, "Delay duration too short. Trying increasing AUDIO_SYS_TASKS_ELAPSED_TIME_MS");
        ret = ESP_FAIL;
        goto exit;
    }

    // 创建一个扩展结构体来存储任务信息和计算的统计数据
    typedef struct {
        TaskStatus_t task;          // 任务状态信息
        uint32_t elapsed_time;      // 任务运行时间
        uint32_t percentage;        // CPU使用率百分比
        BaseType_t core_id;         // 运行的核心ID
        bool is_deleted;            // 任务是否被删除
        bool is_created;            // 任务是否新创建
    } TaskInfoExt_t;

    TaskInfoExt_t *task_info = audio_malloc(sizeof(TaskInfoExt_t) * (start_array_size + end_array_size));
    AUDIO_MEM_CHECK(TAG, task_info, {
        ret = ESP_FAIL;
        goto exit;
    });

    int task_count = 0;

    // 匹配开始和结束数组中的任务，计算CPU使用率
    for (int i = 0; i < start_array_size; i++) {
        bool found = false;
        for (int j = 0; j < end_array_size; j++) {
            if (start_array[i].xHandle == end_array[j].xHandle) {
                // 计算任务在测量期间的运行时间
                task_elapsed_time = end_array[j].ulRunTimeCounter - start_array[i].ulRunTimeCounter;
                // 计算CPU使用率百分比（考虑多核处理器）
                percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time * portNUM_PROCESSORS);

                // 保存任务统计信息
                task_info[task_count].task = start_array[i];
                task_info[task_count].elapsed_time = task_elapsed_time;
                task_info[task_count].percentage = percentage_time;
                task_info[task_count].core_id = start_array[i].xCoreID;
                task_info[task_count].is_deleted = false;
                task_info[task_count].is_created = false;

                task_count++;

                // 标记任务已匹配，避免重复处理
                start_array[i].xHandle = NULL;
                end_array[j].xHandle = NULL;
                found = true;
                break;
            }
        }

        // 处理在测量期间被删除的任务
        if (!found && start_array[i].xHandle != NULL) {
            task_info[task_count].task = start_array[i];
            task_info[task_count].elapsed_time = 0;
            task_info[task_count].percentage = 0;
            task_info[task_count].core_id = start_array[i].xCoreID;
            task_info[task_count].is_deleted = true;
            task_info[task_count].is_created = false;
            task_count++;
        }
    }

    // 处理在测量期间新创建的任务
    for (int i = 0; i < end_array_size; i++) {
        if (end_array[i].xHandle != NULL) {
            task_info[task_count].task = end_array[i];
            task_info[task_count].elapsed_time = 0;
            task_info[task_count].percentage = 0;
            task_info[task_count].core_id = end_array[i].xCoreID;
            task_info[task_count].is_deleted = false;
            task_info[task_count].is_created = true;
            task_count++;
        }
    }

    // 对任务信息进行排序：首先按照CoreId排序，然后按照CPU使用率百分比降序排序
    for (int i = 0; i < task_count - 1; i++) {
        for (int j = 0; j < task_count - i - 1; j++) {
            // 首先按CoreId排序（升序）
            if (task_info[j].core_id > task_info[j + 1].core_id) {
                TaskInfoExt_t temp = task_info[j];
                task_info[j] = task_info[j + 1];
                task_info[j + 1] = temp;
            }
            // 如果CoreId相同，则按CPU使用率百分比降序排序
            else if (task_info[j].core_id == task_info[j + 1].core_id &&
                     task_info[j].percentage < task_info[j + 1].percentage) {
                TaskInfoExt_t temp = task_info[j];
                task_info[j] = task_info[j + 1];
                task_info[j + 1] = temp;
            }
        }
    }

    // 打印表头
    ESP_LOGI(TAG, "| Task              | Run Time    | Per | Prio | HWM       | State   | CoreId   | Stack ");
    
    // 打印排序后的任务信息
    for (int i = 0; i < task_count; i++) {
        if (task_info[i].is_deleted) {
            // 显示已删除的任务
            ESP_LOGI(TAG, "| %s | Deleted", task_info[i].task.pcTaskName);
        } else if (task_info[i].is_created) {
            // 显示新创建的任务
            ESP_LOGI(TAG, "| %s | Created", task_info[i].task.pcTaskName);
        } else {
            // 显示正常任务的详细统计信息
            ESP_LOGI(TAG, "| %-17s | %-11d |%2d%%  | %-4u | %-9u | %-7s | %-8x | %s",
                     task_info[i].task.pcTaskName, (int)task_info[i].elapsed_time, (int)task_info[i].percentage,
                     task_info[i].task.uxCurrentPriority, (int)task_info[i].task.usStackHighWaterMark,
                     task_state[(task_info[i].task.eCurrentState)], task_info[i].core_id,
                     task_stack[esp_ptr_internal(pxTaskGetStackStart(task_info[i].task.xHandle))]);
        }
    }

    printf("\n");
    ret = ESP_OK;

    // 释放任务信息数组内存
    if (task_info) {
        audio_free(task_info);
        task_info = NULL;
    }

exit:    // 统一的错误处理和资源清理路径
    // 释放开始状态数组内存
    if (start_array) {
        audio_free(start_array);
        start_array = NULL;
    }
    // 释放结束状态数组内存
    if (end_array) {
        audio_free(end_array);
        end_array = NULL;
    }
    return ret;
#else
    // 当必要的FreeRTOS配置未启用时的警告信息
    ESP_LOGW(TAG, "请在menuconfig中启用 `CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID` 和 `CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS`");
    return ESP_FAIL;
#endif
}
