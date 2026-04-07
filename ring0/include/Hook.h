#include "global.h"
/* 原始IRP派遣函数解析,支持以下指令模式:
mov reg, rcx

lea reg, [rip + imm]

mov[reg + imm], rax

mov rax, imm64
*/

typedef struct _RECOVERED_DISPATCH {
    PVOID MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];
    ULONG HitCount;
    ULONG Confidence;
} RECOVERED_DISPATCH, * PRECOVERED_DISPATCH;

typedef enum _PARSE_STATE {
    ST_INIT = 0,
    ST_HAVE_DOBJECT,
    ST_PARSING,
    ST_DONE,
    ST_ABORT
} PARSE_STATE;

typedef enum _X64_REG {
    REG_INVALID = -1,
    REG_RAX, REG_RBX, REG_RCX, REG_RDX,
    REG_RSI, REG_RDI,
    REG_R8, REG_R9, REG_R10, REG_R11,
    REG_R12, REG_R13, REG_R14, REG_R15
} X64_REG;

typedef struct _WRITE_EVENT {
    X64_REG BaseReg;     // DriverObject 寄存器
    ULONG   Offset;      // 偏移
    PVOID   Target;      // 函数地址
} WRITE_EVENT;

VOID TestNtfsRecover();