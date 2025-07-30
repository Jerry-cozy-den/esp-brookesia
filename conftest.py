# SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0

"""
ESP-IDF pytest 配置文件
此文件包含了 ESP-IDF 项目测试的全局配置、fixtures 和钩子函数
用于配置测试环境、过滤测试用例、管理构建目录等功能
"""

import logging
import os
import pathlib
import re
import sys
from datetime import datetime
from typing import Callable, List, Optional, Tuple

import pytest
from pytest import Config, FixtureRequest, Function, Session
from pytest_embedded.plugin import multi_dut_argument, multi_dut_fixture

IDF_VERSION = os.environ.get('IDF_VERSION')  # 获取 IDF 版本环境变量
PYTEST_ROOT_DIR = str(pathlib.Path(__file__).parent)  # pytest 根目录路径
logging.info(f'Pytest root dir: {PYTEST_ROOT_DIR}')


@pytest.fixture(scope='session', autouse=True)
def idf_version() -> str:
    """
    获取 ESP-IDF 版本号的 fixture
    
    Returns:
        str: IDF 版本号，格式为 "主版本.副版本"
    """
    if os.environ.get('IDF_VERSION'):
        return os.environ.get('IDF_VERSION')
    idf_path = os.environ.get('IDF_PATH')  # 获取 IDF 路径
    if not idf_path:
        logging.warning('Failed to get IDF_VERSION!')  # 警告：无法获取 IDF 版本
        return ''
    # 从 version.cmake 文件中解析版本信息
    version_path = os.path.join(os.environ['IDF_PATH'], 'tools/cmake/version.cmake')
    regex = re.compile(r'^\s*set\s*\(\s*IDF_VERSION_([A-Z]{5})\s+(\d+)')  # 匹配版本号的正则表达式
    ver = {}
    with open(version_path) as f:
        for line in f:
            m = regex.match(line)
            if m:
                ver[m.group(1)] = m.group(2)  # 提取主版本号和副版本号
    return '{}.{}'.format(int(ver['MAJOR']), int(ver['MINOR']))  # 返回格式化的版本号


@pytest.fixture(scope='session', autouse=True)
def session_tempdir() -> str:
    """
    创建会话级别的临时目录 fixture
    
    Returns:
        str: 临时目录路径，包含时间戳
    """
    _tmpdir = os.path.join(
        os.path.dirname(__file__),
        'pytest_log',  # 在 pytest_log 目录下创建
        datetime.now().strftime('%Y-%m-%d_%H-%M-%S'),  # 使用时间戳命名
    )
    os.makedirs(_tmpdir, exist_ok=True)  # 创建目录（如果不存在）
    return _tmpdir


@pytest.fixture
@multi_dut_argument
def config(request: FixtureRequest) -> str:
    """
    获取测试配置的 fixture
    
    Args:
        request: pytest 请求对象
        
    Returns:
        str: 配置名称，默认为 'defaults'
    """
    config_marker = list(request.node.iter_markers(name='config'))
    return config_marker[0].args[0] if config_marker else 'defaults'


@pytest.fixture
@multi_dut_argument
def app_path(request: FixtureRequest, test_file_path: str) -> str:
    """
    获取应用程序路径的 fixture
    
    Args:
        request: pytest 请求对象
        test_file_path: 测试文件路径
        
    Returns:
        str: 应用程序路径
    """
    config_marker = list(request.node.iter_markers(name='app_path'))
    if config_marker:
        return config_marker[0].args[0]
    else:
        # 兼容旧的 pytest-embedded 参数化 --app_path
        return request.config.getoption('app_path', None) or os.path.dirname(test_file_path)


@pytest.fixture
def test_case_name(request: FixtureRequest, target: str, config: str) -> str:
    """
    生成测试用例名称的 fixture
    
    Args:
        request: pytest 请求对象
        target: 目标芯片
        config: 配置名称
        
    Returns:
        str: 格式化的测试用例名称 "<target>.<config>.<原始名称>"
    """
    if not isinstance(target, str):
        target = '|'.join(sorted(list(set(target))))  # 多目标时用 | 分隔
    if not isinstance(config, str):
        config = '|'.join(sorted(list(config)))  # 多配置时用 | 分隔
    return f'{target}.{config}.{request.node.originalname}'


@pytest.fixture
@multi_dut_fixture
def build_dir(
    app_path: str,
    target: Optional[str],
    config: Optional[str],
    idf_version: str
) -> Optional[str]:
    """
    检查本地构建目录的 fixture
    按以下优先级顺序查找构建目录：

    1. <app_path>/${IDF_VERSION}/build_<target>_<config>
    2. <app_path>/${IDF_VERSION}/build_<target>
    3. <app_path>/build_<target>_<config>
    4. <app_path>/build
    5. <app_path>

    Args:
        app_path: 应用程序路径
        target: 目标芯片
        config: 配置名称
        idf_version: IDF 版本

    Returns:
        str: 有效的构建目录路径
    """

    assert target
    assert config
    check_dirs = []
    # 优先检查带 IDF 版本的目录
    if idf_version:
        check_dirs.append(os.path.join(idf_version, f'build_{target}_{config}'))
        check_dirs.append(os.path.join(idf_version, f'build_{target}'))
    # 然后检查通用目录
    check_dirs.append(f'build_{target}_{config}')
    check_dirs.append('build')
    check_dirs.append('.')
    
    for check_dir in check_dirs:
        binary_path = os.path.join(app_path, check_dir)
        if os.path.isdir(binary_path):
            logging.info(f'find valid binary path: {binary_path}')  # 找到有效的二进制路径
            return check_dir

        logging.warning(
            f'checking binary path: {binary_path} ... missing ... try another place')  # 路径不存在，尝试其他位置

    # 如果都找不到，输出错误并退出
    logging.error(
        f'no build dir. Please build the binary "python .gitlab/tools/build_apps.py {app_path}" and run pytest again')
    sys.exit(1)


@pytest.fixture(autouse=True)
@multi_dut_fixture
def junit_properties(
    test_case_name: str, record_xml_attribute: Callable[[str, object], None]
) -> None:
    """
    自动使用的 fixture，用于修改 junit 报告中的测试用例名称
    将测试用例名称格式化为 <target>.<config>.<case_name>
    
    Args:
        test_case_name: 格式化后的测试用例名称
        record_xml_attribute: 记录 XML 属性的回调函数
    """
    record_xml_attribute('name', test_case_name)


##################
# 钩子函数部分     #
##################
_idf_pytest_embedded_key = pytest.StashKey['IdfPytestEmbedded']  # 存储 pytest embedded 实例的键


def pytest_addoption(parser: pytest.Parser) -> None:
    """
    添加自定义命令行选项
    
    Args:
        parser: pytest 参数解析器
    """
    base_group = parser.getgroup('idf')  # 获取 idf 选项组
    base_group.addoption(
        '--env',
        help='only run tests matching the environment NAME.',  # 只运行匹配环境名称的测试
    )


def pytest_configure(config: Config) -> None:
    """
    pytest 配置钩子函数
    要求通过 CLI 选项 "--target" 指定目标芯片
    
    Args:
        config: pytest 配置对象
    """
    # 检查是否为帮助命令，如果是则不需要 target 参数
    help_commands = ['--help', '--fixtures', '--markers', '--version']
    for cmd in help_commands:
        if cmd in config.invocation_params.args:
            target = 'unneeded'  # 帮助命令不需要指定 target
            break
    else:
        target = config.getoption('target')  # 获取 target 选项
    if not target:
        raise ValueError('Please specify one target marker via "--target [TARGET]"')  # 必须指定 target

    # 创建并注册 IdfPytestEmbedded 插件实例
    config.stash[_idf_pytest_embedded_key] = IdfPytestEmbedded(
        target=target,
        env_name=config.getoption('env'),  # 获取环境名称选项
    )
    config.pluginmanager.register(config.stash[_idf_pytest_embedded_key])


def pytest_unconfigure(config: Config) -> None:
    """
    pytest 取消配置钩子函数
    清理注册的插件实例
    
    Args:
        config: pytest 配置对象
    """
    _pytest_embedded = config.stash.get(_idf_pytest_embedded_key, None)
    if _pytest_embedded:
        del config.stash[_idf_pytest_embedded_key]  # 从存储中删除
        config.pluginmanager.unregister(_pytest_embedded)  # 取消注册插件


class IdfPytestEmbedded:
    """
    ESP-IDF pytest 嵌入式测试插件类
    用于管理测试用例的过滤、配置和执行
    """
    def __init__(
        self,
        target: Optional[str] = None,
        env_name: Optional[str] = None,
    ):
        """
        初始化插件实例
        
        Args:
            target: 目标芯片名称，用于过滤测试用例
            env_name: 环境名称，用于过滤测试用例
        """
        # CLI 选项，用于过滤测试用例
        self.target = target
        self.env_name = env_name

        # 失败的测试用例列表：(测试用例名称, 是否为已知失败, 是否为预期失败)
        self._failed_cases: List[
            Tuple[str, bool, bool]
        ] = []  # (test_case_name, is_known_failure_cases, is_xfail)

    @pytest.hookimpl(tryfirst=True)
    def pytest_sessionstart(self, session: Session) -> None:
        """
        pytest 会话开始钩子函数
        在测试会话开始时调用，用于初始化配置
        
        Args:
            session: pytest 会话对象
        """
        if self.target:
            self.target = self.target.lower()  # 将目标名称转换为小写
            session.config.option.target = self.target  # 设置会话配置中的 target 选项

    # @pytest.hookimpl(tryfirst=True)  # 注释掉的装饰器
    def pytest_collection_modifyitems(self, items: List[Function]) -> None:
        """
        修改测试用例集合的钩子函数
        用于设置默认超时时间和根据条件过滤测试用例
        
        Args:
            items: 测试用例函数列表
        """
        # 为每个测试用例设置默认超时时间（5分钟）
        for item in items:
            # 默认超时 5 分钟
            if 'timeout' not in item.keywords:
                item.add_marker(pytest.mark.timeout(5 * 60))

        # 根据 "--target" 选项过滤测试用例
        if self.target:
            def item_targets(item): return [m.args[0] for m in item.iter_markers(name='target')]
            items[:] = [item for item in items if self.target in item_targets(item)]

        # 根据 "--env" 选项过滤测试用例
        if self.env_name:
            def item_envs(item): return [m.args[0] for m in item.iter_markers(name='env')]
            items[:] = [item for item in items if self.env_name in item_envs(item)]
