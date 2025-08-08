# GIF示例文件目录

这个目录用于存放示例GIF动画文件。

## 文件放置位置

### SPIFFS存储 (推荐)
将GIF文件放置在以下位置：
```
/spiffs/gifs/
├── sample1.gif      # 示例动画1
├── sample2.gif      # 示例动画2
├── animation.gif    # 通用动画
└── demo.gif         # 演示动画
```

### SD卡存储 (可选)
如果使用SD卡，文件路径为：
```
/sdcard/gifs/
├── large_animation.gif   # 大尺寸动画
├── hd_demo.gif          # 高清演示
└── collection/          # 动画合集
    ├── cartoon1.gif
    ├── cartoon2.gif
    └── effects.gif
```

## GIF文件规格要求

### 推荐规格
- **分辨率**: 240x240像素以内
- **颜色数**: 64色以内
- **帧率**: 10-15 FPS
- **文件大小**: 小于500KB
- **格式**: 标准GIF87a/GIF89a

### 创建优化的GIF文件

使用ImageMagick优化GIF：
```bash
# 调整大小和颜色数
magick input.gif -resize 240x240 -colors 64 output.gif

# 降低帧率
magick input.gif -delay 10 output.gif

# 压缩文件大小
magick input.gif -strip -coalesce -layers optimize output.gif
```

## 文件系统烧录

### 使用spiffsgen.py
```bash
# 生成SPIFFS镜像
python $IDF_PATH/components/spiffs/spiffsgen.py \
    0x100000 \
    ./gifs \
    spiffs.bin \
    --page-size=256 \
    --obj-name-len=32

# 烧录到ESP32
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 \
    write_flash -z 0x110000 spiffs.bin
```

### 使用parttool.py  
```bash
# 烧录分区
python $IDF_PATH/components/partition_table/parttool.py \
    --port /dev/ttyUSB0 \
    write_partition \
    --partition-name spiffs \
    --input spiffs.bin
```

## 测试文件建议

创建以下测试文件用于验证功能：

1. **small_test.gif** (50KB) - 快速加载测试
2. **medium_test.gif** (200KB) - 中等大小测试  
3. **loop_test.gif** (循环动画) - 循环播放测试
4. **once_test.gif** (单次动画) - 一次性播放测试

## 注意事项

1. **文件命名**: 使用英文名称，避免特殊字符
2. **路径长度**: 总路径长度不超过255字符
3. **文件权限**: 确保文件具有读取权限
4. **内存限制**: 大文件可能导致内存不足

## 故障排除

### 文件无法显示
```bash
# 检查文件格式
file your_animation.gif

# 验证GIF完整性  
magick identify your_animation.gif
```

### 播放卡顿
- 减少帧数
- 降低分辨率
- 减少颜色数
- 优化帧间压缩
