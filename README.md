# SnowSword
Windows 内核级系统工具 | 内核防护 + 进程管理 + 内存编辑 + 驱动交互

## 项目介绍
SnowSword 是一款基于 **Ring0 内核驱动 + Ring3 用户层应用** 开发的 Windows 系统工具，
支持内核层回调枚举、内存编辑、进程/线程管理、SSDT Hook、驱动通信等功能。

## 架构
- **Ring0**：Windows 内核驱动 (C/C++)，实现内核层操作
- **Ring3**：用户层GUI应用 (VisualFreeBasic)，提供交互界面

## 编译环境
- 内核驱动：Visual Studio + WDK
- 应用层：VisualFreeBasic 6.0
- 平台：Windows x64

## 功能清单
- 进程管理(结束/挂起进程,检测隐藏进程,校验进程数字签名,注入dll,枚举模块/线程/窗口定时器/消息钩子等)
- 枚举内核模块,内核线程内核表项(如SSDT,IDT,HalDispatch等)
- 枚举内核回调(如进程创建回调/ExCallback/MiniFilter/过滤驱动/WfpFilters等)
- 枚举文件(检测隐藏文件,强行复制文件,强删一切文件)
- 枚举注册表(支持hive分析)
- 内存读写与编辑
- 电源管理(包括暴力关机/重启等等)

## 免责声明
本项目仅用于学习 Windows 内核开发与安全研究，
请勿用于非法用途，一切后果自行承担。
