' udis86_types.bi - FreeBasic 绑定 for libudis86 (types.h)
' 原始头文件: libudis86/types.h
' 转换日期: 2026-04-07

#ifndef __UD_TYPES_BI__
#define __UD_TYPES_BI__
' ---------- 类型别名 ----------
/'Type ud_type_t As ud_type
Type ud_mnemonic_code_t As ud_mnemonic_code
Type ud_t As ud
Type ud_operand_t As ud_operand'/

' ---------- 宏常量 ----------
#define UD_SYN_INTEL   @ud_translate_intel   ' 函数地址
#define UD_SYN_ATT     @ud_translate_att
#define UD_EOI         (-1)
#define UD_INP_CACHE_SZ  32
#define UD_VENDOR_AMD     0
#define UD_VENDOR_INTEL   1
#define UD_VENDOR_ANY     2

#endif  ' __UD_TYPES_BI__