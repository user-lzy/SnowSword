#pragma once

#include "global.h"

// 修正后：16位限长 + 64位基址（完全匹配SGDT指令输出）
#pragma pack(1)
typedef struct _GDTINFO {
    UINT16 uGdtLimit;    // GDT限长（偏移量，总字节数=uGdtLimit+1）
    UINT64 uGdtBase;     // GDT基址（64位，直接使用无需拼接）
} GDTINFO, * PGDTINFO;
#pragma pack()

//0x8 bytes (sizeof)
typedef struct _GDTENTRY
{
    USHORT LimitLow;                                                        //0x0
    USHORT BaseLow;                                                         //0x2
    union
    {
        struct
        {
            UCHAR BaseMid;                                                  //0x4
            UCHAR Flags1;                                                   //0x5
            UCHAR Flags2;                                                   //0x6
            UCHAR BaseHi;                                                   //0x7
        } Bytes;                                                            //0x4
        struct
        {
            ULONG BaseMid : 8;
            ULONG Type : 4;
            ULONG S : 1;
            ULONG Dpl : 2;
            ULONG Pres : 1;
            ULONG LimitHi : 4;
            ULONG Avl : 1;
            ULONG Reserved_0 : 1;
            ULONG D_B : 1;
            ULONG Granularity : 1;
            ULONG BaseHi : 8;
        } Bits;
    } HighWord;
}GDTENTRY, * PGDTENTRY;

typedef struct _GDT_ITEM {
    ULONG GDT_Limit;
    ULONG64 SEL_Base;
    ULONG SEL_Limit;
    ULONG Present;
    ULONG Granularity;
    ULONG DPL;
    ULONG Type;
    ULONG S;
    //USHORT Segment;
    //WCHAR Attributes[5];
}GDT_ITEM, *PGDT_ITEM;

typedef struct _GDT_INFO {
    //DWORD NumOfItem;
    USHORT Limit;
	ULONG64 Base;
    GDT_ITEM Item[11];
}GDT_INFO, *PGDT_INFO;

ULONG GetGDT(PGDT_INFO Array);
