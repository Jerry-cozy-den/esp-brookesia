#!/bin/bash

# ESP-IDF Brookesia 项目快速切换脚本
# 使用方法: ./switch_board.sh [board_name]

BOARDS=(
    "phone_s3_lcd_ev_board"
    "phone_s3_box_3" 
    "phone_m5stack_core_s3"
    "phone_p4_function_ev_board"
    "speaker"
)

BOARD_PATHS=(
    "products/phone/phone_s3_lcd_ev_board"
    "products/phone/phone_s3_box_3"
    "products/phone/phone_m5stack_core_s3" 
    "products/phone/phone_p4_function_ev_board"
    "products/speaker"
)

# 如果没有提供参数，显示可用的板子
if [ $# -eq 0 ]; then
    echo "可用的开发板:"
    for i in "${!BOARDS[@]}"; do
        echo "  $((i+1)). ${BOARDS[i]}"
    done
    echo ""
    echo "使用方法: $0 [板子编号或名称]"
    echo "例如: $0 1 或 $0 phone_s3_lcd_ev_board"
    exit 0
fi

# 处理输入参数
BOARD_INDEX=-1
if [[ "$1" =~ ^[0-9]+$ ]]; then
    # 如果是数字，转换为索引
    BOARD_INDEX=$((($1 - 1)))
else
    # 如果是名称，查找对应的索引
    for i in "${!BOARDS[@]}"; do
        if [[ "${BOARDS[i]}" == "$1" ]]; then
            BOARD_INDEX=$i
            break
        fi
    done
fi

# 验证索引
if [ $BOARD_INDEX -lt 0 ] || [ $BOARD_INDEX -ge ${#BOARDS[@]} ]; then
    echo "错误: 无效的板子选择"
    exit 1
fi

BOARD_NAME=${BOARDS[BOARD_INDEX]}
BOARD_PATH=${BOARD_PATHS[BOARD_INDEX]}

echo "切换到 ${BOARD_NAME}..."
echo "路径: ${BOARD_PATH}"

# 检查路径是否存在
if [ ! -d "$BOARD_PATH" ]; then
    echo "错误: 路径不存在 - $BOARD_PATH"
    exit 1
fi

# 切换到目标目录
cd "$BOARD_PATH" || exit 1

echo "已切换到 ${BOARD_NAME} 目录"
echo "当前工作目录: $(pwd)"
echo ""
echo "可用命令:"
echo "  idf.py build        - 构建项目"
echo "  idf.py flash        - 烧录到设备"
echo "  idf.py monitor      - 监控串口输出"
echo "  idf.py flash monitor - 烧录并监控"

# 启动一个新的 shell 会话
bash
