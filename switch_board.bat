@echo off
REM ESP-IDF Brookesia 项目快速切换脚本 (Windows版本)
REM 使用方法: switch_board.bat [board_name]

setlocal EnableDelayedExpansion

set "BOARDS=phone_s3_lcd_ev_board phone_s3_box_3 phone_m5stack_core_s3 phone_p4_function_ev_board speaker"
set "BOARD_PATHS=products\phone\phone_s3_lcd_ev_board products\phone\phone_s3_box_3 products\phone\phone_m5stack_core_s3 products\phone\phone_p4_function_ev_board products\speaker"

REM 转换为数组
SET i=0
for %%a in (%BOARDS%) do (
   SET /A i+=1
   SET "board[!i!]=%%a"
)
SET board_count=%i%

SET i=0
for %%a in (%BOARD_PATHS%) do (
   SET /A i+=1
   SET "path[!i!]=%%a"
)

REM 如果没有提供参数，显示可用的板子
if "%1"=="" (
    echo 可用的开发板:
    for /L %%i in (1,1,%board_count%) do (
        echo   %%i. !board[%%i]!
    )
    echo.
    echo 使用方法: %0 [板子编号或名称]
    echo 例如: %0 1 或 %0 phone_s3_lcd_ev_board
    goto :eof
)

REM 处理输入参数
set BOARD_INDEX=0
echo %1| findstr /r "^[1-9][0-9]*$" >nul
if %errorlevel% equ 0 (
    REM 如果是数字
    set BOARD_INDEX=%1
) else (
    REM 如果是名称，查找对应的索引
    for /L %%i in (1,1,%board_count%) do (
        if "!board[%%i]!"=="%1" (
            set BOARD_INDEX=%%i
            goto :found
        )
    )
)

:found
REM 验证索引
if %BOARD_INDEX% leq 0 (
    echo 错误: 无效的板子选择
    goto :eof
)
if %BOARD_INDEX% gtr %board_count% (
    echo 错误: 无效的板子选择
    goto :eof
)

set BOARD_NAME=!board[%BOARD_INDEX%]!
set BOARD_PATH=!path[%BOARD_INDEX%]!

echo 切换到 %BOARD_NAME%...
echo 路径: %BOARD_PATH%

REM 检查路径是否存在
if not exist "%BOARD_PATH%" (
    echo 错误: 路径不存在 - %BOARD_PATH%
    goto :eof
)

REM 切换到目标目录
cd /d "%BOARD_PATH%"

echo 已切换到 %BOARD_NAME% 目录
echo 当前工作目录: %CD%
echo.
echo 可用命令:
echo   idf.py build        - 构建项目
echo   idf.py flash        - 烧录到设备
echo   idf.py monitor      - 监控串口输出
echo   idf.py flash monitor - 烧录并监控
echo.

REM 启动一个新的命令提示符会话
cmd /k
