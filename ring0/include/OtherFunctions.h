#pragma once

#include "global.h"

// By: LyShark.com
PVOID SearchSpecialCode(PVOID pSearchBeginAddr, ULONG ulSearchLength, PUCHAR pSpecialCode, ULONG ulSpecialCodeLength);
PVOID SearchSpecialCode1(PVOID pSearchBeginAddr, ULONG ulSearchLength, PUCHAR pSpecialCode, ULONG ulSpecialCodeLength);
PVOID SearchSpecialCodeWithMask(
    PVOID pSearchBeginAddr,
    ULONG ulSearchLength,
    PUCHAR pSpecialCode,
    PUCHAR pMask,  // 掩码数组，1表示需要匹配，0表示通配符
    ULONG ulSpecialCodeLength);

PVOID FindObpInfoMaskToOffset();

PVOID FindPsGetNextProcessThread();
