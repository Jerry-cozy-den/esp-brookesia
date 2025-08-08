/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <map>
#include <string>
#include <optional>
#include <functional>
#include <vector>
#include <mutex>
#include "cJSON.h"
#include "thread/esp_utils_thread.hpp"

namespace esp_brookesia::ai_framework {

/**
 * @brief AI函数参数对象，描述单个参数的类型、名称、描述和取值
 */
class FunctionParameter {
public:
    // 参数类型：布尔、数字、字符串
    enum class ValueType {
        Boolean,
        Number,
        String
    };

    // 构造函数
    FunctionParameter(const std::string &name, const std::string &description, ValueType type, bool required = true);

    // 获取参数名称
    const std::string &name() const { return name_; }
    // 获取参数描述
    const std::string &description() const { return description_; }
    // 获取参数类型
    ValueType type() const { return type_; }
    // 是否必填
    bool required() const { return required_; }

    // 获取参数值
    bool boolean() const { return boolean_; }
    int number() const { return number_; }
    const std::string &string() const { return string_; }

    // 设置参数值
    void setBoolean(bool value);
    void setNumber(int value);
    void setString(const std::string &value);

    // 获取参数描述的JSON字符串
    std::string getDescriptorJson();

private:
    std::string name_;         // 参数名称
    std::string description_;  // 参数描述
    ValueType type_;           // 参数类型
    bool required_;            // 是否必填
    bool boolean_;             // 布尔值
    int number_;               // 数值
    std::string string_;       // 字符串值
};

/**
 * @brief AI函数定义对象，描述一个可被AI调用的函数
 * 
 * 功能原理：
 * - 支持注册函数名、描述、参数列表
 * - 支持设置回调（可指定线程配置）
 * - 支持通过JSON参数调用
 * - 用于AI助手等场景下，动态注册和调用功能
 */
class FunctionDefinition {
public:
    using CallbackThreadConfig = esp_utils::ThreadConfig; // 回调线程配置类型
    using Callback = std::function<void(const std::vector<FunctionParameter>&)>; // 回调类型

    // 构造函数，指定函数名-描述
    FunctionDefinition(const std::string &name, const std::string &description);

    // 添加参数定义
    void addParameter(
        const std::string &name, const std::string &description, FunctionParameter::ValueType type, bool required = true
    );
    // 设置回调及线程配置
    void setCallback(Callback callback, std::optional<CallbackThreadConfig> thread_config = std::nullopt);
    // 通过cJSON参数调用函数
    bool invoke(const cJSON *args) const;
    // 获取函数名
    std::string name() const { return name_; }
    // 获取函数定义的JSON字符串（用于AI描述）
    std::string getJson() const;

private:
    std::string name_;         // 函数名
    std::string description_;  // 函数描述
    std::vector<FunctionParameter> parameters_; // 参数列表
    Callback callback_;        // 回调函数
    std::optional<CallbackThreadConfig> thread_config_; // 线程配置
};

/**
 * @brief AI函数定义列表，支持全局注册和查找调用
 * 
 * 功能原理：
 * - 单例模式，所有AI函数集中管理
 * - 支持添加函数、通过JSON查找并调用、导出所有函数定义为JSON
 */
class FunctionDefinitionList {
public:
    FunctionDefinitionList(const FunctionDefinitionList &) = delete;
    FunctionDefinitionList &operator=(const FunctionDefinitionList &) = delete;

    // 获取单例实例
    static FunctionDefinitionList &requestInstance();
    // 添加函数定义
    void addFunction(const FunctionDefinition &func);
    // 通过cJSON查找并调用函数
    bool invokeFunction(const cJSON *function_call) const;
    // 获取所有函数定义的JSON字符串
    std::string getJson() const;

private:
    FunctionDefinitionList() = default;

    std::vector<FunctionDefinition> functions_;           // 所有函数定义
    std::map<std::string, size_t> function_index_;        // 名称到索引的映射
};

} // namespace esp_brookesia::ai_framework