#pragma once

#include "global.h"

// By: LyShark.com
PVOID SearchSpecialCode(PVOID pSearchBeginAddr, ULONG ulSearchLength, PUCHAR pSpecialCode, ULONG ulSpecialCodeLength);
PVOID SearchSpecialCode1(PVOID pSearchBeginAddr, ULONG ulSearchLength, PUCHAR pSpecialCode, ULONG ulSpecialCodeLength);

PVOID FindObpInfoMaskToOffset();

PVOID FindPsGetNextProcessThread();
