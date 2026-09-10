# SnowSword

**Windows 10/11 x64 Kernel Analysis & Anti-Rootkit Toolkit**

[English](README_EN.md) | **简体中文**

![Platform](https://img.shields.io/badge/platform-Windows%2010%20%2F%2011-lightgrey)
![Architecture](https://img.shields.io/badge/architecture-x64-lightgrey)
![Ring0](https://img.shields.io/badge/Ring0-C%20%2F%20WDK-blue)
![Ring3](https://img.shields.io/badge/Ring3-VisualFreeBasic-blue)
![License](https://img.shields.io/badge/license-MIT-green)

SnowSword 是一款面向 **Windows 10/11 x64** 的内核分析与 Anti-Rootkit 工具，由 **Ring0 内核驱动 + Ring3 GUI/CLI** 组成，主要用于 Windows 内核开发学习、安全研究、Rootkit 检测与系统底层分析。

项目覆盖隐藏进程/驱动检测、Kernel Callback、SSDT / Shadow SSDT、IDT / GDT、Inline Hook、Minifilter、WFP、NTFS / FAT32、进程内存与内核对象等 Windows Internals 相关能力。

> **定位：** Windows Kernel Analysis / Anti-Rootkit / Rootkit Detection / Windows Internals / Kernel Security Research

## 核心能力

| 领域 | 功能 |
|---|---|
| **Rootkit Detection** | 隐藏进程检测、隐藏文件检测、隐藏网络连接检测、Inline Hook 检测 |
| **Process & Thread** | 进程/线程枚举、结束/挂起/恢复、模块枚举、线程调用栈、进程签名信息 |
| **Kernel Analysis** | 内核模块、内核线程、SSDT / Shadow SSDT、IDT、GDT、HalDispatch / HalPrivateDispatch |
| **Kernel Callback** | 进程/线程/镜像回调、ExCallback、注册表回调等 |
| **Driver Analysis** | Driver Object、IRP Dispatch、Attached Device、Minifilter / Legacy Filter |
| **Win32k Analysis** | Message Hook、Event Hook、Hotkey、Window Timer |
| **Network** | TCP/UDP 连接、隐藏连接检测、WFP Filter / Callout、NDIS Miniport |
| **File System** | 文件枚举、强制复制/删除、底层文件读写、文件占用分析 |
| **Disk & Filesystem** | NTFS 解析、FAT32 文件枚举、Sector I/O |
| **Registry** | 注册表枚举与读写、Hive 离线分析 |
| **Memory** | 进程内存读写、内存区域枚举、内核内存分析 |
| **Interface** | GUI、CLI、JSON 输出、Agent Pipe 模式 |

## 架构

```text
┌─────────────────────────────────────────┐
│ Ring3                                    │
│ VisualFreeBasic GUI / CLI / Agent       │
└───────────────────┬─────────────────────┘
                    │ DeviceIoControl / IOCTL
                    ▼
┌─────────────────────────────────────────┐
│ Ring0                                    │
│ Windows Kernel Driver (C / WDK)         │
└───────────────────┬─────────────────────┘
                    │ Kernel APIs / internals
                    ▼
┌─────────────────────────────────────────┐
│ Windows Kernel / Drivers / File System  │
└─────────────────────────────────────────┘
```

## 功能详情

### 进程与线程

- 进程枚举与隐藏进程检测
- 结束、强制结束、挂起与恢复进程
- 进程数字签名校验
- 模块枚举与 DLL 注入
- 线程枚举、线程控制与调用栈解析
- 窗口定时器、消息钩子、事件钩子与热键枚举

### 内核分析

- 内核模块与内核线程枚举
- SSDT / Shadow SSDT 枚举
- IDT / GDT 枚举
- HalDispatchTable / HalPrivateDispatchTable 分析
- 进程、线程、镜像创建等 Kernel Callback 枚举
- ExCallback 与注册表回调枚举
- Driver Object / IRP Dispatch / Attached Device 分析
- Minifilter、Legacy Filter、WFP Filter / Callout 枚举
- Inline Hook 检测

### 文件系统与磁盘

- 普通文件枚举与隐藏文件检测
- 强制复制、强制删除
- 底层文件读写与 Sector I/O
- NTFS 文件系统解析
- FAT32 文件枚举
- 文件占用进程查找

### 注册表

- 注册表枚举与读写
- Registry Hive 文件离线分析

### 内存

- 进程内存读写与编辑
- 内存区域枚举
- 内核内存分析

### 网络

- TCP / UDP 连接枚举
- 隐藏网络连接检测
- WFP Filter / Callout 分析
- NDIS Miniport 枚举

### CLI / Agent

SnowSword 除 GUI 外还支持命令行和 Agent 管道模式，可用于脚本化查询和自动化调用。

```text
SnowSword.exe --cli
SnowSword.exe -c "process list"
SnowSword.exe -c "thread list --pid <PID>"
SnowSword.exe --format JSON ...
SnowSword.exe --agent-pipe <name>
```

> CLI 接口仍在持续完善，具体可用命令以当前版本为准。

## 系统兼容性

| Windows 版本 | 支持状态 |
|---|---|
| Windows 10 1607 ~ 22H2 | ✅ 支持 |
| Windows 11 21H2 ~ 24H2 | ✅ 支持 |
| 其他 Windows 版本 | ⚠️ 未经充分测试 |

- 当前项目仅面向 **x64**。
- 部分内核功能依赖未文档化结构或运行时特征扫描，Windows 更新可能影响个别模块。
- 如果发现特定 Windows Build 上的兼容性问题，欢迎提交 Issue，并附上系统 Build Number 与复现信息。

## 编译环境

### Ring0 Driver

- Visual Studio 2022
- Windows Driver Kit (WDK)
- Target: x64

构建步骤：

1. 打开 `ring0/SnowSword.sln`
2. 选择 `x64 / Debug` 或 `x64 / Release`
3. Build 生成 `SnowSword.sys`

### Ring3 GUI / CLI

- VisualFreeBasic 5.9.7
- WinFBX（随 VisualFreeBasic 提供）
- Target: x64 (`-gen gas64`)

构建步骤：

1. 使用 VisualFreeBasic 打开 `ring3/SnowSword.ffp`
2. 选择 64 位编译配置
3. Build 生成 `SnowSword.exe`

运行时需确保 `SnowSword.exe` 与 `SnowSword.sys` 位于正确的部署位置，并以满足驱动加载要求的权限和签名环境运行。

## 项目结构

```text
SnowSword/
├─ ring0/                 # Windows kernel driver (C / WDK)
│  ├─ include/
│  └─ sources/
├─ ring3/                 # GUI / CLI (VisualFreeBasic)
│  ├─ forms/
│  ├─ modules/
│  └─ images/
├─ bin/                   # Published binary snapshot
├─ README.md
├─ README_EN.md
└─ LICENSE
```

## 适合关注本项目的人

如果你正在研究以下主题，SnowSword 中可能有可参考的实现：

- Windows Kernel Development
- Windows Internals
- Anti-Rootkit / Rootkit Detection
- Kernel Security / Kernel Integrity
- Windows Driver Development
- Reverse Engineering
- Malware Analysis
- Digital Forensics
- NTFS / FAT32 Internals
- Minifilter / WFP / NDIS
- Kernel Callback / SSDT / IDT / GDT

## 状态与反馈

SnowSword 仍处于持续开发和重构阶段。部分功能涉及 Windows 内核未文档化实现，不同系统版本上的行为可能存在差异。

如果遇到问题，建议在 Issue 中提供：

- Windows 版本与完整 Build Number
- SnowSword 版本或 commit SHA
- 相关功能模块
- 错误码 / 日志 / WinDbg 信息
- 最小复现步骤

## License

本项目采用 [MIT License](LICENSE)。

## 免责声明

SnowSword 仅用于 **Windows 内核开发学习、安全研究、恶意软件分析和授权环境中的系统诊断**。请勿将本项目用于未经授权的系统操作或其他违法用途。使用者应自行承担因测试内核级功能而产生的数据损坏、系统崩溃或其他风险。
