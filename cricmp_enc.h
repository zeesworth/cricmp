#pragma once
#include "types.h"

void CMP_WriteFile(u8* inData, u32 inSize);

u8* CMP_GetWriteBuffer();
int CMP_GetWriteSize();
