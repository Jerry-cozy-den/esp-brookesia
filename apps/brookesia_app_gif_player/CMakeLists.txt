# 设置最低CMake版本要求
cmake_minimum_required(VERSION 3.5)

# 设置组件源文件
set(COMPONENT_SRCS
    "esp_brookesia_app_gif_player.cpp"
    "assets/esp_brookesia_app_icon_gif_player_112_112.c"
)

# 设置包含目录
set(COMPONENT_ADD_INCLUDEDIRS ".")

# 设置组件依赖
set(COMPONENT_REQUIRES 
    esp_brookesia_core
    lvgl
    esp_timer
    spiffs
    fatfs
)

# 注册IDF组件
register_component()

# 设置编译选项
target_compile_options(${COMPONENT_LIB} PRIVATE 
    -Wno-unused-parameter
    -Wno-unused-variable
)

# 添加预处理器定义
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LV_USE_GIF=1
    ESP_BROOKESIA_APP_GIF_PLAYER_ENABLED=1
)
