'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModTools.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' Enum SHUTDOWN_ACTION '已经搬到定义文件处
'     ShutdownNoReboot '关闭系统 '已经搬到定义文件处
'     ShutdownReboot '重启 '已经搬到定义文件处
'     ShutdownPowerOff '关闭系统并关闭电源 '已经搬到定义文件处
'     ShutdownForceReboot '强制重启 '已经搬到定义文件处
' End Enum '已经搬到定义文件处

Declare Function ZwShutdownSystem Lib "ntdll.dll"(ByVal ShutdownAction As SHUTDOWN_ACTION) As NTSTATUS

