/*
 * SPDX发牌文本：2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX许可证标识符：CC0-1.0
 * 
 * LVGL自定义内存管理实现文件
 * 
 * 本文件为LVGL图形库提供自定义的内存分配实现，主要特点：
 * - 使用ESP32的heap_caps_malloc()替代标准malloc()
 * - 优先从SPIRAM（外部PSRAM）分配内存，提高内存利用率
 * - 针对LVGL的内存使用模式进行优化
 * - 支持大内存对象的高效分配（如图像缓冲区、画布等）
 * 
 * 内存分配策略：
 * - MEM_CAPS设置为SPIRAM优先，充分利用外部PSRAM
 * - 支持8位对齐的内存访问要求
 * - 自动处理内存碎片和大块内存需求
 * 
 * 适用场景：
 * - ESP32-S3等具有PSRAM的芯片
 * - 需要大量内存的图形应用
 * - 复杂UI界面和动画效果
 */
/**
 * @file lv_malloc_core.c
 * LVGL内存分配核心实现
 */

/*********************
 *      包含文件
 *********************/
#include "lvgl.h"
#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM

#include "esp_heap_caps.h"

/*********************
 *      宏定义
 *********************/
// 内存分配能力标志：优先使用SPIRAM（外部PSRAM）+ 8位对齐
#define MEM_CAPS (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
// 备选方案：使用内部RAM（注释掉）
// #define MEM_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)

/**********************
 *      类型定义
 **********************/

/**********************
 *  静态函数声明
 **********************/

/**********************
 *  静态变量
 **********************/

/**********************
 *      宏函数
 **********************/

/**********************
 *   全局函数实现
 **********************/

/**
 * @brief LVGL内存管理初始化
 * 
 * 由于使用ESP32的heap_caps系统，无需特殊初始化。
 * PSRAM在系统启动时已由ESP-IDF初始化。
 */
void lv_mem_init(void)
{
    return; /* 无需初始化操作 */
}

/**
 * @brief LVGL内存管理反初始化
 * 
 * 由于使用系统级内存管理，无需特殊清理操作。
 */
void lv_mem_deinit(void)
{
    return; /* 无需反初始化操作 */

}

/**
 * @brief 添加内存池（不支持）
 * 
 * 在当前实现中不支持自定义内存池，直接使用ESP32的heap_caps系统。
 * 
 * @param mem 内存池起始地址
 * @param bytes 内存池大小
 * @return lv_mem_pool_t 总是返回NULL（不支持）
 */
lv_mem_pool_t lv_mem_add_pool(void *mem, size_t bytes)
{
    /* 不支持自定义内存池 */
    LV_UNUSED(mem);
    LV_UNUSED(bytes);
    return NULL;
}

/**
 * @brief 移除内存池（不支持）
 * 
 * 在当前实现中不支持自定义内存池操作。
 * 
 * @param pool 要移除的内存池句柄
 */
void lv_mem_remove_pool(lv_mem_pool_t pool)
{
    /* 不支持自定义内存池 */
    LV_UNUSED(pool);
    return;
}

/**
 * @brief LVGL核心内存分配函数
 * 
 * 使用ESP32的heap_caps_malloc()从SPIRAM分配内存。
 * 这确保了LVGL的内存分配优先使用外部PSRAM，
 * 从而为其他组件保留更多的内部RAM。
 * 
 * @param size 要分配的内存大小（字节）
 * @return void* 分配的内存指针，失败时返回NULL
 */
void *lv_malloc_core(size_t size)
{
    return heap_caps_malloc(size, MEM_CAPS);
}

/**
 * @brief LVGL核心内存重分配函数
 * 
 * 使用ESP32的heap_caps_realloc()重新分配内存大小。
 * 保持与原始分配相同的内存类型属性（SPIRAM优先）。
 * 
 * @param p 原内存指针
 * @param new_size 新的内存大小（字节）
 * @return void* 重新分配的内存指针，失败时返回NULL
 */
void *lv_realloc_core(void *p, size_t new_size)
{
    return heap_caps_realloc(p, new_size, MEM_CAPS);
}

/**
 * @brief LVGL核心内存释放函数
 * 
 * 释放之前分配的内存。由于heap_caps_malloc()分配的内存
 * 可以用标准free()释放，这里直接使用free()。
 * 
 * @param p 要释放的内存指针
 */
void lv_free_core(void *p)
{
    free(p);
}

/**
 * @brief LVGL内存监控函数（不支持）
 * 
 * 在当前实现中不提供详细的内存使用统计。
 * 可以通过ESP32的heap_caps_get_info()获取堆信息。
 * 
 * @param mon_p 内存监控结构体指针
 */
void lv_mem_monitor_core(lv_mem_monitor_t *mon_p)
{
    /* 不支持详细内存监控 */
    LV_UNUSED(mon_p);
    return;
}

/**
 * @brief LVGL内存测试函数（不支持）
 * 
 * 在当前实现中不提供内存完整性测试。
 * ESP32的heap_caps系统已提供基本的内存保护。
 * 
 * @return lv_result_t 总是返回LV_RESULT_OK
 */
lv_result_t lv_mem_test_core(void)
{
    /* 不支持内存测试 */
    return LV_RESULT_OK;
}

/**********************
 *   静态函数实现
 **********************/

#endif /* LV_STDLIB_CUSTOM */
