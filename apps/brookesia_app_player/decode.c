// Copyright 2024 Espressif Systems (Shanghai) CO., LTD.
// All rights reserved.

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
// #include "image_io.h"

#include "decode.h"
// 全局配置变量
static jpeg_pixel_format_t j_type     = JPEG_PIXEL_FORMAT_RGB888;  // 输出像素格式：RGB888 (每像素3字节)
static jpeg_rotate_t       j_rotation = JPEG_ROTATE_0D;            // 图像旋转角度：0度 (不旋转)

/**
 * @brief 解码一张完整JPEG图片的示例函数
 * 
 * 这个函数演示了如何使用 ESP JPEG 解码器将 JPEG 格式的图像数据
 * 解码为指定格式的原始像素数据。整个JPEG数据会一次性传递给解码器处理。
 * 
 * @param input_buf  输入缓冲区，包含JPEG图像数据
 * @param len        输入JPEG数据的字节长度
 * @param output_buf 输出缓冲区指针，用于接收解码后的像素数据（由函数分配内存）
 * @param out_len    输出数据的字节长度
 * @return jpeg_error_t 返回解码结果状态码
 */
jpeg_error_t esp_jpeg_decode_one_picture(uint8_t *input_buf, int len, uint8_t **output_buf, int *out_len)
{
    uint8_t *out_buf = NULL;                       // 输出缓冲区
    jpeg_error_t ret = JPEG_ERR_OK;                // 返回值初始化
    jpeg_dec_io_t *jpeg_io = NULL;                 // I/O回调处理结构
    jpeg_dec_header_info_t *out_info = NULL;       // JPEG头信息结构

    // 生成默认解码器配置
    jpeg_dec_config_t config = DEFAULT_JPEG_DEC_CONFIG();
    config.output_type = j_type;                   // 设置输出像素格式 (RGB888)
    config.rotate = j_rotation;                    // 设置旋转角度 (0度)
    // config.scale.width       = 0;              // 可选：设置输出图像宽度缩放 (0表示不缩放)
    // config.scale.height      = 0;              // 可选：设置输出图像高度缩放 (0表示不缩放)
    // config.clipper.width     = 0;              // 可选：设置裁剪区域宽度 (0表示不裁剪)
    // config.clipper.height    = 0;              // 可选：设置裁剪区域高度 (0表示不裁剪)

    // 创建 JPEG 解码器句柄
    jpeg_dec_handle_t jpeg_dec = NULL;
    ret = jpeg_dec_open(&config, &jpeg_dec);
    if (ret != JPEG_ERR_OK) {
        return ret;  // 解码器初始化失败
    }

    // 创建 I/O 回调句柄，用于管理输入输出数据
    jpeg_io = calloc(1, sizeof(jpeg_dec_io_t));
    if (jpeg_io == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 内存分配失败
        goto jpeg_dec_failed;
    }

    // 创建输出信息句柄，用于存储解码后的图像信息
    out_info = calloc(1, sizeof(jpeg_dec_header_info_t));
    if (out_info == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 内存分配失败
        goto jpeg_dec_failed;
    }

    // 设置输入数据缓冲区和长度到I/O回调结构
    jpeg_io->inbuf = input_buf;          // 输入JPEG数据缓冲区
    jpeg_io->inbuf_len = len;            // 输入数据长度

    // 解析JPEG头信息，获取图像的基本属性（宽度、高度、颜色格式等）
    ret = jpeg_dec_parse_header(jpeg_dec, jpeg_io, out_info);
    if (ret != JPEG_ERR_OK) {
        goto jpeg_dec_failed;  // 头解析失败，可能是损坏的JPEG文件
    }

    // 计算输出缓冲区所需的内存大小，默认为RGB888格式（每像素3字节）
    *out_len = out_info->width * out_info->height * 3;
    
    // 根据配置的输出格式调整缓冲区大小
    if (config.output_type == JPEG_PIXEL_FORMAT_RGB565_LE
        || config.output_type == JPEG_PIXEL_FORMAT_RGB565_BE
        || config.output_type == JPEG_PIXEL_FORMAT_CbYCrY) {
        // RGB565和CbYCrY格式：每像素2字节
        *out_len = out_info->width * out_info->height * 2;
    } else if (config.output_type == JPEG_PIXEL_FORMAT_RGB888) {
        // RGB888格式：每像素3字节
        *out_len = out_info->width * out_info->height * 3;
    } else {
        // 不支持的像素格式
        ret = JPEG_ERR_INVALID_PARAM;
        goto jpeg_dec_failed;
    }
    
    // 分配对齐的输出缓冲区（16字节对齐，提高内存访问效率）
    out_buf = jpeg_calloc_align(*out_len, 16);
    if (out_buf == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 输出缓冲区内存分配失败
        goto jpeg_dec_failed;
    }
    jpeg_io->outbuf = out_buf;  // 设置输出缓冲区
    *output_buf = out_buf;      // 返回输出缓冲区地址

    // 开始JPEG解码处理
    // 这是主要的解码步骤，将JPEG数据转换为指定格式的原始像素数据
    ret = jpeg_dec_process(jpeg_dec, jpeg_io);
    if (ret != JPEG_ERR_OK) {
        goto jpeg_dec_failed;  // 解码处理失败
    }

    // 解码器反初始化和资源清理
jpeg_dec_failed:
    jpeg_dec_close(jpeg_dec);    // 关闭并释放解码器句柄
    if (jpeg_io) {
        free(jpeg_io);           // 释放I/O回调结构
    }
    if (out_info) {
        free(out_info);          // 释放头信息结构
    }
    return ret;
}

/**
 * @brief 解码单张JPEG图片并返回图像尺寸信息的增强函数
 * 
 * 这个函数在原有解码功能基础上，额外返回图像的宽度和高度信息，
 * 方便调用者创建LVGL图像描述符或进行其他需要尺寸信息的操作。
 * 
 * @param input_buf  输入缓冲区，包含JPEG图像数据
 * @param len        输入JPEG数据的字节长度
 * @param output_buf 输出缓冲区指针，用于接收解码后的像素数据（由函数分配内存）
 * @param out_len    输出数据的字节长度
 * @param width      输出图像宽度
 * @param height     输出图像高度
 * @return jpeg_error_t 返回解码结果状态码
 */
jpeg_error_t esp_jpeg_decode_one_picture_with_info(uint8_t *input_buf, int len, uint8_t **output_buf, int *out_len, int *width, int *height)
{
    uint8_t *out_buf = NULL;                       // 输出缓冲区
    jpeg_error_t ret = JPEG_ERR_OK;                // 返回值初始化
    jpeg_dec_io_t *jpeg_io = NULL;                 // I/O回调处理结构
    jpeg_dec_header_info_t *out_info = NULL;       // JPEG头信息结构

    // 生成默认解码器配置
    jpeg_dec_config_t config = DEFAULT_JPEG_DEC_CONFIG();
    config.output_type = j_type;                   // 设置输出像素格式 (RGB888)
    config.rotate = j_rotation;                    // 设置旋转角度 (0度)

    // 创建 JPEG 解码器句柄
    jpeg_dec_handle_t jpeg_dec = NULL;
    ret = jpeg_dec_open(&config, &jpeg_dec);
    if (ret != JPEG_ERR_OK) {
        return ret;  // 解码器初始化失败
    }

    // 创建 I/O 回调句柄，用于管理输入输出数据
    jpeg_io = calloc(1, sizeof(jpeg_dec_io_t));
    if (jpeg_io == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 内存分配失败
        goto jpeg_dec_failed_with_info;
    }

    // 创建输出信息句柄，用于存储解码后的图像信息
    out_info = calloc(1, sizeof(jpeg_dec_header_info_t));
    if (out_info == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 内存分配失败
        goto jpeg_dec_failed_with_info;
    }

    // 设置输入数据缓冲区和长度到I/O回调结构
    jpeg_io->inbuf = input_buf;          // 输入JPEG数据缓冲区
    jpeg_io->inbuf_len = len;            // 输入数据长度

    // 解析JPEG头信息，获取图像的基本属性（宽度、高度、颜色格式等）
    ret = jpeg_dec_parse_header(jpeg_dec, jpeg_io, out_info);
    if (ret != JPEG_ERR_OK) {
        goto jpeg_dec_failed_with_info;  // 头解析失败，可能是损坏的JPEG文件
    }

    // 返回图像尺寸信息
    *width = out_info->width;
    *height = out_info->height;

    // 计算输出缓冲区所需的内存大小，默认为RGB888格式（每像素3字节）
    *out_len = out_info->width * out_info->height * 3;
    
    // 根据配置的输出格式调整缓冲区大小
    if (config.output_type == JPEG_PIXEL_FORMAT_RGB565_LE
        || config.output_type == JPEG_PIXEL_FORMAT_RGB565_BE
        || config.output_type == JPEG_PIXEL_FORMAT_CbYCrY) {
        // RGB565和CbYCrY格式：每像素2字节
        *out_len = out_info->width * out_info->height * 2;
    } else if (config.output_type == JPEG_PIXEL_FORMAT_RGB888) {
        // RGB888格式：每像素3字节
        *out_len = out_info->width * out_info->height * 3;
    } else {
        // 不支持的像素格式
        ret = JPEG_ERR_INVALID_PARAM;
        goto jpeg_dec_failed_with_info;
    }
    
    // 分配对齐的输出缓冲区（16字节对齐，提高内存访问效率）
    out_buf = jpeg_calloc_align(*out_len, 16);
    if (out_buf == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 输出缓冲区内存分配失败
        goto jpeg_dec_failed_with_info;
    }
    jpeg_io->outbuf = out_buf;  // 设置输出缓冲区
    *output_buf = out_buf;      // 返回输出缓冲区地址

    // 开始JPEG解码处理
    // 这是主要的解码步骤，将JPEG数据转换为指定格式的原始像素数据
    ret = jpeg_dec_process(jpeg_dec, jpeg_io);
    if (ret != JPEG_ERR_OK) {
        goto jpeg_dec_failed_with_info;  // 解码处理失败
    }

    // 解码器反初始化和资源清理
jpeg_dec_failed_with_info:
    jpeg_dec_close(jpeg_dec);    // 关闭并释放解码器句柄
    if (jpeg_io) {
        free(jpeg_io);           // 释放I/O回调结构
    }
    if (out_info) {
        free(out_info);          // 释放头信息结构
    }
    return ret;
}

/**
 * @brief 分块解码JPEG图片的示例函数
 * 
 * 这个函数演示了如何使用分块(block)模式解码JPEG图像。
 * 分块解码适用于大图像或内存受限的场景，可以逐块处理图像数据，
 * 而不需要一次性分配整个图像的内存空间。
 * 
 * @param input_buf 输入缓冲区，包含JPEG图像数据
 * @param len       输入JPEG数据的字节长度
 * @return jpeg_error_t 返回解码结果状态码
 */
jpeg_error_t esp_jpeg_decode_one_picture_block(unsigned char *input_buf, int len)
{
    unsigned char *output_block = NULL;            // 分块输出缓冲区
    jpeg_error_t ret = JPEG_ERR_OK;                // 返回值初始化
    jpeg_dec_io_t *jpeg_io = NULL;                 // I/O回调处理结构
    jpeg_dec_header_info_t *out_info = NULL;       // JPEG头信息结构
    FILE *f_out = NULL;                            // 输出文件句柄

    // 生成默认解码器配置
    jpeg_dec_config_t config = DEFAULT_JPEG_DEC_CONFIG();
    config.block_enable = true;                    // 启用分块解码模式

    // 创建JPEG解码器句柄
    jpeg_dec_handle_t jpeg_dec = NULL;
    ret = jpeg_dec_open(&config, &jpeg_dec);
    if (ret != JPEG_ERR_OK) {
        return ret;  // 解码器初始化失败
    }

    // 创建I/O回调句柄，用于管理输入输出数据
    jpeg_io = calloc(1, sizeof(jpeg_dec_io_t));
    if (jpeg_io == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 内存分配失败
        goto jpeg_dec_failed;
    }

    // 创建输出信息句柄，用于存储解码后的图像信息
    out_info = calloc(1, sizeof(jpeg_dec_header_info_t));
    if (out_info == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 内存分配失败
        goto jpeg_dec_failed;
    }

    // 设置输入数据缓冲区和长度到I/O回调结构
    jpeg_io->inbuf = input_buf;          // 输入JPEG数据缓冲区
    jpeg_io->inbuf_len = len;            // 输入数据长度

    // 解析JPEG头信息，获取图像的基本属性（宽度、高度、颜色格式等）
    ret = jpeg_dec_parse_header(jpeg_dec, jpeg_io, out_info);
    if (ret != JPEG_ERR_OK) {
        goto jpeg_dec_failed;  // 头解析失败
    }

    // 分配分块输出数据缓冲区
    // 在分块模式下，每次只处理图像的一部分，因此缓冲区较小
    int output_len = 0;
    ret = jpeg_dec_get_outbuf_len(jpeg_dec, &output_len);
    if (ret != JPEG_ERR_OK || output_len == 0) {
        goto jpeg_dec_failed;  // 无法获取输出缓冲区长度
    }

    // 分配对齐的分块输出缓冲区（16字节对齐）
    output_block = jpeg_calloc_align(output_len, 16);
    if (output_block == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 分块缓冲区内存分配失败
        goto jpeg_dec_failed;
    }
    jpeg_io->outbuf = output_block;

    // 获取需要处理的分块数量
    // 图像会被分成多个块，每个块独立处理
    int process_count = 0;
    ret = jpeg_dec_get_process_count(jpeg_dec, &process_count);
    if (ret != JPEG_ERR_OK || process_count == 0) {
        goto jpeg_dec_failed;  // 无法获取处理块数量
    }

#if TEST_USE_SDCARD
    // 在SD卡上打开输出文件（需要先初始化SD卡）
    f_out = fopen("/sdcard/esp_jpeg_decode_one_picture_block.bin", "wb");
    if (f_out == NULL) {
        ret = JPEG_ERR_FAIL;  // 文件打开失败
        goto jpeg_dec_failed;
    }
#endif  /* TEST_USE_SDCARD */

    // 逐块解码JPEG数据
    // 这是分块解码的核心循环，每次迭代处理一个图像块
    for (int block_cnt = 0; block_cnt < process_count; block_cnt++) {
        ret = jpeg_dec_process(jpeg_dec, jpeg_io);
        if (ret != JPEG_ERR_OK) {
            goto jpeg_dec_failed;  // 当前块解码失败
        }

#if TEST_USE_SDCARD
        // 将解码后的块数据写入SD卡文件
        int written_data = fwrite(jpeg_io->outbuf, 1, jpeg_io->out_size, f_out);
        if (written_data != jpeg_io->out_size) {
            ret = JPEG_ERR_FAIL;  // 文件写入失败
            goto jpeg_dec_failed;
        }
#endif  /* TEST_USE_SDCARD */
    }

    // 分块解码完成，进行资源清理
jpeg_dec_failed:
    jpeg_dec_close(jpeg_dec);           // 关闭并释放解码器句柄
    if (jpeg_io) {
        free(jpeg_io);                  // 释放I/O回调结构
    }
    if (out_info) {
        free(out_info);                 // 释放头信息结构
    }
    jpeg_free_align(output_block);      // 释放对齐分配的分块缓冲区
#if TEST_USE_SDCARD
    if (f_out) {
        fclose(f_out);                  // 关闭输出文件
    }
#endif  /* TEST_USE_SDCARD */
    return ret;
}

/**
 * @brief 打开JPEG流解码器的示例函数
 * 
 * 这个函数演示了如何初始化流式解码器。流式解码适用于需要
 * 处理连续JPEG数据流的场景，如网络传输或实时图像处理。
 * 
 * @param jpeg_handle JPEG流处理句柄
 * @return jpeg_error_t 返回初始化结果状态码
 */
jpeg_error_t esp_jpeg_stream_open(esp_jpeg_stream_handle_t jpeg_handle)
{
    jpeg_error_t ret = JPEG_ERR_OK;

    // 生成默认解码器配置
    jpeg_dec_config_t config = DEFAULT_JPEG_DEC_CONFIG();
    config.output_type = j_type;           // 设置输出像素格式
    config.rotate = j_rotation;            // 设置旋转角度
    // config.scale.width       = 0;      // 可选：设置输出图像宽度缩放
    // config.scale.height      = 0;      // 可选：设置输出图像高度缩放
    // config.clipper.width     = 0;      // 可选：设置裁剪区域宽度
    // config.clipper.height    = 0;      // 可选：设置裁剪区域高度
    jpeg_handle->output_type = j_type;     // 保存输出格式到句柄中

    // 创建JPEG解码器句柄
    ret = jpeg_dec_open(&config, &jpeg_handle->jpeg_dec);
    if (ret != JPEG_ERR_OK) {
        return ret;  // 解码器初始化失败
    }

    // 创建I/O回调句柄，用于管理输入输出数据
    jpeg_handle->jpeg_io = calloc(1, sizeof(jpeg_dec_io_t));
    if (jpeg_handle->jpeg_io == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 内存分配失败
        goto jpeg_dec_failed;
    }

    // 创建输出信息句柄，用于存储解码后的图像信息
    jpeg_handle->out_info = calloc(1, sizeof(jpeg_dec_header_info_t));
    if (jpeg_handle->out_info == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 内存分配失败
        goto jpeg_dec_failed;
    }
    return JPEG_ERR_OK;  // 流解码器初始化成功

    // 流解码器初始化失败时的资源清理
jpeg_dec_failed:
    jpeg_dec_close(jpeg_handle->jpeg_dec);  // 关闭解码器句柄
    if (jpeg_handle->jpeg_io) {
        free(jpeg_handle->jpeg_io);          // 释放I/O回调结构
    }
    if (jpeg_handle->out_info) {
        free(jpeg_handle->out_info);         // 释放头信息结构
    }
    return ret;
}

/**
 * @brief 流式解码JPEG数据的示例函数
 * 
 * 这个函数演示了如何使用已初始化的流解码器处理JPEG数据。
 * 适用于网络流、文件流等连续数据源的实时解码场景。
 * 
 * @param jpeg_handle 已初始化的JPEG流处理句柄
 * @param input_buf   输入缓冲区，包含JPEG图像数据
 * @param len         输入JPEG数据的字节长度
 * @param output_buf  输出缓冲区指针，用于接收解码后的像素数据
 * @param out_len     输出数据的字节长度
 * @return jpeg_error_t 返回解码结果状态码
 */
jpeg_error_t esp_jpeg_stream_decode(esp_jpeg_stream_handle_t jpeg_handle, uint8_t *input_buf, int len, uint8_t **output_buf, int *out_len)
{
    jpeg_error_t ret = JPEG_ERR_OK;
    unsigned char *out_buf = NULL;

    // 设置输入数据缓冲区和长度到I/O回调结构
    jpeg_handle->jpeg_io->inbuf = input_buf;      // 输入JPEG数据缓冲区
    jpeg_handle->jpeg_io->inbuf_len = len;        // 输入数据长度

    // 解析JPEG头信息，获取图像的基本属性（宽度、高度、颜色格式等）
    ret = jpeg_dec_parse_header(jpeg_handle->jpeg_dec, jpeg_handle->jpeg_io, jpeg_handle->out_info);
    if (ret != JPEG_ERR_OK) {
        return ret;  // 头解析失败，可能是损坏的JPEG数据
    }

    // 计算输出缓冲区所需的内存大小，默认为RGB888格式（每像素3字节）
    *out_len = jpeg_handle->out_info->width * jpeg_handle->out_info->height * 3;
    
    // 根据配置的输出格式调整缓冲区大小
    if (jpeg_handle->output_type == JPEG_PIXEL_FORMAT_RGB565_LE
        || jpeg_handle->output_type == JPEG_PIXEL_FORMAT_RGB565_BE
        || jpeg_handle->output_type == JPEG_PIXEL_FORMAT_CbYCrY) {
        // RGB565和CbYCrY格式：每像素2字节
        *out_len = jpeg_handle->out_info->width * jpeg_handle->out_info->height * 2;
    } else if (jpeg_handle->output_type == JPEG_PIXEL_FORMAT_RGB888) {
        // RGB888格式：每像素3字节
        *out_len = jpeg_handle->out_info->width * jpeg_handle->out_info->height * 3;
    } else {
        // 不支持的像素格式
        ret = JPEG_ERR_INVALID_PARAM;
        return ret;
    }
    
    // 分配对齐的输出缓冲区（16字节对齐，提高内存访问效率）
    out_buf = jpeg_calloc_align(*out_len, 16);
    if (out_buf == NULL) {
        ret = JPEG_ERR_NO_MEM;  // 输出缓冲区内存分配失败
        return ret;
    }
    jpeg_handle->jpeg_io->outbuf = out_buf;  // 设置输出缓冲区
    *output_buf = out_buf;                   // 返回输出缓冲区地址

    // 开始流式JPEG解码处理
    // 这是流解码的主要步骤，将JPEG数据转换为指定格式的原始像素数据
    ret = jpeg_dec_process(jpeg_handle->jpeg_dec, jpeg_handle->jpeg_io);
    if (ret != JPEG_ERR_OK) {
        return ret;  // 解码处理失败
    }
    return ret;  // 解码成功
}

/**
 * @brief 关闭JPEG流解码器的示例函数
 * 
 * 这个函数演示了如何正确关闭流解码器并释放相关资源。
 * 在完成所有流解码操作后，必须调用此函数进行清理。
 * 
 * @param jpeg_handle JPEG流处理句柄
 * @return jpeg_error_t 返回关闭操作的结果状态码
 */
jpeg_error_t esp_jpeg_stream_close(esp_jpeg_stream_handle_t jpeg_handle)
{
    jpeg_error_t ret = JPEG_ERR_OK;

    // 关闭并释放JPEG解码器句柄
    ret = jpeg_dec_close(jpeg_handle->jpeg_dec);
    
    // 释放I/O回调结构内存
    if (jpeg_handle->jpeg_io) {
        free(jpeg_handle->jpeg_io);
    }
    
    // 释放头信息结构内存
    if (jpeg_handle->out_info) {
        free(jpeg_handle->out_info);
    }
    
    return ret;  // 返回关闭操作结果
}
