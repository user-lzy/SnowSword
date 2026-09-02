# SnowSword

Windows 内核级系统工具 | Ring0 驱动 + Ring3 GUI | 进程/内存/钩子/文件/注册表/内核分析

## 项目介绍

SnowSword 是一款基于 **Ring0 内核驱动 + Ring3 用户层应用**开发的 Windows 系统工具，面向内核开发学习与安全研究。

- Ring0 驱动使用 C 编写，通过 IOCTL 与 Ring3 通信，实现内核层枚举、读写与防护操作
- Ring3 GUI 使用 VisualFreeBasic 编写，提供可视化交互界面，同时支持 CLI/Agent 管道模式
- 内核功能适配 Windows 10 1607 ~ Windows 11 24H2（x64）

## 架构

```
Ring3 GUI (VisualFreeBasic)
    │  IOCTL / DeviceIoControl
    ▼
Ring0 Driver (C / WDK)
    │  内核 API / 未文档化接口
    ▼
Windows Kernel
```

## 功能清单

### 进程与线程
- 进程枚举（检测隐藏进程）、结束/挂起进程
- 进程数字签名校验、DLL 注入
- 模块枚举、线程枚举与调用栈解析
- 窗口定时器枚举、消息钩子枚举（MsgHook / EventHook / Hotkey）

### 内核分析
- 内核模块枚举、内核线程枚举
- 内核表项枚举（SSDT、IDT、GDT、HalDispatch、HalPrivateDispatch 等）
- 内核回调枚举（进程/线程/镜像创建回调、ExCallback、注册表回调）
- 过滤驱动枚举（MiniFilter、Legacy Filter）、WFP 过滤器枚举
- Inline Hook 检测

### 文件系统
- 文件枚举（支持检测隐藏文件）、强制复制、强制删除
- 底层文件读写（绕过文件锁）
- NTFS 解析、FAT32 文件枚举
- 文件占用进程查找

### 注册表
- 注册表枚举与读写
- Hive 文件离线分析

### 内存
- 进程内存读写与编辑
- 内存区域枚举

### 网络
- TCP/UDP 连接枚举（含隐藏连接检测）

### 其他
- 电源管理（强制关机/重启/休眠）
- 对象目录枚举
- 驱动加载/卸载
- CLI 命令行模式（`--cli` / `--c <cmd>` / `--format JSON` / `--agent-pipe <name>`）

## 编译环境

### Ring0 驱动
- Visual Studio 2022
- Windows Driver Kit (WDK)
- 平台：x64

### Ring3 GUI
- VisualFreeBasic 5.9.7
- WinFBX 库（随 VFB 捆绑）
- 平台：x64（`-gen gas64`）

## 构建步骤

### 构建 Ring0 驱动
1. 用 Visual Studio 打开 `ring0/SnowSword.sln`
2. 选择 `x64 / Debug` 或 `x64 / Release` 配置
3. Build → 生成 `SnowSword.sys`

### 构建 Ring3 GUI
1. 用 VisualFreeBasic 5.9.7 打开 `ring3/SnowSword.ffp`
2. 选择 64 位编译配置
3. Build → 生成 `SnowSword.exe`

> 使用时需将 `SnowSword.sys` 与 `SnowSword.exe` 放在同一目录下运行。

## 兼容性

| Windows 版本 | 支持状态 |
|---|---|
| Windows 10 1607 ~ 22H2 | ✅ 支持 |
| Windows 11 21H2 ~ 24H2 | ✅ 支持 |
| 其他版本 | ⚠️ 未经测试 |

> 仅支持 x64 系统。部分内核偏移通过运行时扫描自动适配，无需手动维护版本表。

## 免责声明

本项目仅用于学习 Windows 内核开发与安全研究，请勿用于非法用途，一切后果自行承担。
