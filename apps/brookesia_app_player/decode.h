// Copyright 2024 Espressif Systems (Shanghai) CO., LTD.
// All rights reserved.

/**
 * @file decode.h
 * @brief ESP JPEG 解码器接口头文件
 * 
 * 本文件定义了ESP JPEG解码库的主要接口函数和数据结构。
 * 提供了三种解码模式：一次性解码、分块解码和流式解码，
 * 适用于不同的应用场景和内存限制。
 */

#pragma once

#include <stdint.h>
#include "esp_jpeg_common.h"    // JPEG通用定义和常量
#include "esp_jpeg_dec.h"       // JPEG解码器核心功能

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * @brief JPEG流解码器结构体
 * 
 * 这个结构体包含了流式解码所需的所有组件：
 * - 解码器句柄：用于执行实际的解码操作
 * - I/O回调：管理输入输出数据缓冲区
 * - 头信息：存储解码后的图像属性信息
 * - 输出格式：指定解码后的像素格式
 */
struct esp_jpeg_stream {
    jpeg_dec_handle_t       jpeg_dec;     // JPEG解码器句柄
    jpeg_dec_io_t          *jpeg_io;      // I/O回调处理结构指针
    jpeg_dec_header_info_t *out_info;     // 解码后图像信息结构指针
    jpeg_pixel_format_t     output_type;  // 输出像素格式（RGB888、RGB565等）
};
typedef struct esp_jpeg_stream *esp_jpeg_stream_handle_t;

/**
 * @brief 解码单张JPEG图片（一次性解码模式）
 * 
 * 这个函数实现了最基本的JPEG解码功能，将完整的JPEG数据
 * 一次性解码为指定格式的原始像素数据。适用于：
 * - 小到中等大小的图像（通常小于1MB）
 * - 内存充足的应用场景
 * - 需要快速解码的情况
 * 
 * @note 输出缓冲区由函数内部分配，调用者需要在使用完毕后手动释放
 * 
 * @param  input_buf   输入缓冲区指针，包含完整的JPEG图像数据
 * @param  len         输入缓冲区的字节长度
 * @param  output_buf  输出缓冲区指针的指针，函数内部会分配内存并返回地址
 * @param  out_len     实际输出数据的字节长度
 *
 * @return
 *       - JPEG_ERR_OK  解码成功
 *       - 其他值        解码失败，参考 jpeg_error_t 枚举定义
 */
jpeg_error_t esp_jpeg_decode_one_picture(uint8_t *input_buf, int len, uint8_t **output_buf, int *out_len);

/**
 * @brief 使用分块解码API解码单张JPEG图片
 * 
 * 这个函数演示了分块解码的使用方法，将大图像分割成多个
 * 小块进行处理。适用于：
 * - 大尺寸图像（如高分辨率照片）
 * - 内存受限的嵌入式系统
 * - 需要边解码边处理的流水线应用
 * 
 * @note 这是一个示例函数，实际应用中可能需要根据具体需求修改
 * 
 * @param  input_buf  输入缓冲区指针，包含完整的JPEG图像数据
 * @param  len        输入缓冲区的字节长度
 *
 * @return
 *       - JPEG_ERR_OK  解码成功
 *       - 其他值        解码失败，参考 jpeg_error_t 枚举定义
 */
jpeg_error_t esp_jpeg_decode_one_picture_block(unsigned char *input_buf, int len);

/**
 * @brief 打开JPEG流解码器句柄
 * 
 * 初始化流式解码器，创建必要的数据结构和解码器实例。
 * 流式解码适用于：
 * - 网络流媒体应用
 * - 需要处理连续JPEG数据的场景
 * - 需要重复使用解码器的情况
 * 
 * @note 使用完毕后必须调用 esp_jpeg_stream_close() 释放资源
 * 
 * @param  jpeg_handle  JPEG流处理句柄，需要预先分配 esp_jpeg_stream 结构体
 *
 * @return
 *       - JPEG_ERR_OK  初始化成功
 *       - 其他值        初始化失败，参考 jpeg_error_t 枚举定义
 */
jpeg_error_t esp_jpeg_stream_open(esp_jpeg_stream_handle_t jpeg_handle);

/**
 * @brief 解码JPEG流数据
 * 
 * 使用已初始化的流解码器处理JPEG数据。可以多次调用
 * 此函数处理不同的JPEG图像，而无需重新初始化解码器。
 * 
 * 典型使用流程：
 * 1. esp_jpeg_stream_open() - 初始化
 * 2. esp_jpeg_stream_decode() - 解码（可多次调用）
 * 3. esp_jpeg_stream_close() - 清理资源
 * 
 * @note 输出缓冲区由函数内部分配，调用者需要在使用完毕后手动释放
 * 
 * @param  jpeg_handle  已初始化的JPEG流处理句柄
 * @param  input_buf    输入缓冲区指针，包含JPEG图像数据
 * @param  len          输入缓冲区的字节长度
 * @param  output_buf   输出缓冲区指针的指针，函数内部会分配内存并返回地址
 * @param  out_len      实际输出数据的字节长度
 *
 * @return
 *       - JPEG_ERR_OK  解码成功
 *       - 其他值        解码失败，参考 jpeg_error_t 枚举定义
 */
jpeg_error_t esp_jpeg_stream_decode(esp_jpeg_stream_handle_t jpeg_handle, uint8_t *input_buf, int len, uint8_t **output_buf, int *out_len);

/**
 * @brief 关闭JPEG流解码器
 * 
 * 释放流解码器占用的所有资源，包括解码器句柄、
 * I/O回调结构和头信息结构。
 * 
 * @note 调用此函数后，jpeg_handle 将变为无效，不能再次使用
 * 
 * @param  jpeg_handle  要关闭的JPEG流处理句柄
 *
 * @return
 *       - JPEG_ERR_OK  关闭成功
 *       - 其他值        关闭过程中出现错误
 */
jpeg_error_t esp_jpeg_stream_close(esp_jpeg_stream_handle_t jpeg_handle);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
