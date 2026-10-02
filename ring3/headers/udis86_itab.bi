' udis86_itab.bi -- 指令表定义，由 udis86:scripts/ud_itab.py 生成
' 原始头文件: itab.h
' 转换日期: 2026-04-07

#ifndef __UD_ITAB_BI__
#define __UD_ITAB_BI__

' ------------------------------------------------------------
' ud_table_type -- 查找表类型（参见 decode.c）
' ------------------------------------------------------------
Enum ud_table_type
    UD_TAB__OPC_VEX
    UD_TAB__OPC_TABLE
    UD_TAB__OPC_X87
    UD_TAB__OPC_MOD
    UD_TAB__OPC_RM
    UD_TAB__OPC_OSIZE
    UD_TAB__OPC_MODE
    UD_TAB__OPC_VEX_L
    UD_TAB__OPC_3DNOW
    UD_TAB__OPC_REG
    UD_TAB__OPC_ASIZE
    UD_TAB__OPC_VEX_W
    UD_TAB__OPC_SSE
    UD_TAB__OPC_VENDOR
End Enum

' ------------------------------------------------------------
' 外部字符串数组: 助记符名称表 (在库中定义)
' ------------------------------------------------------------
Extern ud_mnemonics_str As Const ZString Ptr

#endif  ' __UD_ITAB_BI__