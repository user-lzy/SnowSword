#pragma once

#include "GDT.h"

ULONG GetGDT(PGDT_INFO Array)
{
    // 获取 CPU 核心数
    int i_cpuNum = KeNumberProcessors;
    //DbgPrint("CPU核心数: %d \n", i_cpuNum);

    for (KAFFINITY i = 0; i < i_cpuNum; i++)
    {
        //DbgPrint("CPU核心: %Iu \n", i);
        // 线程绑定特定 CPU
        KeSetSystemAffinityThread(i + 1);

        GDTINFO stcGDT = { 0 };

        _sgdt(&stcGDT);

        // 取消绑定 CPU
        KeRevertToUserAffinityThread();

        PGDTENTRY pGdtEntry = NULL;
        unsigned int nGdtEntry = 0;
        ULONG uData = 0;

        // 2. 直接赋值64位基址，无需拼接
        pGdtEntry = (PGDTENTRY)stcGDT.uGdtBase;

        //DbgPrint("-------------GDT---------------\n");
        //DbgPrint("GDT Addr: 0x%p\n", pGdtEntry);

        _try{
            Array[i].Limit = stcGDT.uGdtLimit;
            nGdtEntry = (stcGDT.uGdtLimit + 1) / 8;
			Array[i].Base = stcGDT.uGdtBase;
            for (ULONG j = 0; j < nGdtEntry; ++j)
            {
                //if (!(pGdtEntry[j].HighWord.Bits.Pres)) continue;
                Array[i].Item[j].Present = pGdtEntry[j].HighWord.Bits.Pres;
                Array[i].Item[j].GDT_Limit = pGdtEntry[j].LimitLow;
                //Array[i].Item[j].pGDTDescriptor = &pGdtEntry[j];
                //DbgPrint("pGdtEntry[j]:0x%p, Array[i].Item[j].pGDTDescriptor:0x%p", &pGdtEntry[j], Array[i].Item[j].pGDTDescriptor);

                Array[i].Item[j].SEL_Base = (ULONG64)pGdtEntry[j].BaseLow
                    + ((ULONG64)(pGdtEntry[j].HighWord.Bits.BaseMid) << 16)
                    + ((ULONG64)(pGdtEntry[j].HighWord.Bits.BaseHi) << 24);
                //DbgPrint("SEL_Base: 0x%lluX\n", Array[i].Item[j].SEL_Base);

                uData = pGdtEntry[j].LimitLow
                    + ((ULONG)(pGdtEntry[j].HighWord.Bits.LimitHi) << 16);
                //DbgPrint("Segment Limit: 0x%08X ", uData);

                Array[i].Item[j].SEL_Limit = uData;

                //(pGdtEntry[j].HighWord.Bits.Granularity)
                //    ? DbgPrint("pages\n")
                //    : DbgPrint("bytes\n");

                Array[i].Item[j].Granularity = pGdtEntry[j].HighWord.Bits.Granularity;

                //DbgPrint("DPL: %d\n", pGdtEntry[j].HighWord.Bits.Dpl);

                Array[i].Item[j].DPL = pGdtEntry[j].HighWord.Bits.Dpl;
                Array[i].Item[j].S = pGdtEntry[j].HighWord.Bits.S;
                Array[i].Item[j].Type = pGdtEntry[j].HighWord.Bits.Type;
            }

        }
            _except(EXCEPTION_EXECUTE_HANDLER) {
            //DbgPrint("EnumGDT except.status: %X", GetExceptionCode());
        }

    }
    return i_cpuNum;
}