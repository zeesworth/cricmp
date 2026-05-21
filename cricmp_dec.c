#include "cricmp_dec.h"
#include "types.h"
#include "macros.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "endians.h"

double CMP_version_no;

u8* CMP_file_buffer = NULL;
u32 CMP_file_size = 0;

u8* CMP_file_in;
// bitmask table
static u16 bittbl[] = {
	1 << 0,
	1 << 1,
	1 << 2,
	1 << 3,
	1 << 4,
	1 << 5,
	1 << 6,
	1 << 7,
	1 << 8,
	1 << 9,
	1 << 10,
	1 << 11,
	1 << 12,
	1 << 13,
	1 << 14,
	1 << 15,
};

u8* memmove_byte(u8* dest, u8* src, int count) {
	if (count == 0) return dest;

	u8* copy = dest;
	do {
		count--;
		*copy++ = *src++;
	} while (count != 0);
	return dest;
}

int CMP_DecSdBlkStd(u8** inPtr, u8** outPtr, u8* outEnd, u32 tokenCount) {
	u8 flagsHi = *(*inPtr)++;
	u8 flagsLo = *(*inPtr)++;
	u16 flags = ((u16)flagsHi << 8) | flagsLo;

	u16* mask = bittbl;
	for (size_t i = 0; i < tokenCount; i++, mask++)
	{
		if (*outPtr > outEnd) return -1;
		int remaining = outEnd - *outPtr + 1;

		u8 codeHi = *(*inPtr)++;
		u8 codeLo = *(*inPtr)++;
		u16 code = ((u16)codeHi << 8) | codeLo;
		
		if ((flags & *mask) == 0) {
			long length;
			if ((code & 0xF000) == 0) {
				// repeat data

				length = code + 3;
				if (length > remaining) length = remaining;
				length &= 0xFFFF;

				memset(*outPtr, *(*outPtr - 1), length);
			}
			else {
				// copy data from prev decompressed data
				// length is stored in upper 4 bits,
				// offset in remaining bits

				length = (code >> 12) + 2;
				if (length > remaining) length = remaining;
				length &= 0xFFFF;

				u32 offset = code & 0x0FFF;
				memmove_byte(*outPtr, *outPtr - offset, length);
			}
			*outPtr += length;
		} else {
			u8* out = *outPtr;
			// literal, write one value
			if (out == outEnd) {
				// if there's only one byte remaining 
				// just write one byte and exit
				*out = codeHi;
				*outPtr += 1;
				return 1;
			}
			*out++ = codeHi;
			*out++ = codeLo;
			*outPtr += 2;
		}
	}

	return 0;
}

bool CMP_DecSlideSub(u8* inData, u8** outData, u32 outSize) {
	inData += 2;
	u8* outEnd = *outData + outSize - 1;

	// get the amount of sub-blocks in this file
	u32 blockCount;
	if (CMP_version_no >= 2.0) {
		blockCount = ((u32)inData[0] << 24) | ((u32)inData[1] << 16) | ((u32)inData[2] << 8) | inData[3];
		inData += 4;
	} else {
		blockCount = ((u32)inData[0] << 8) | inData[1];
		inData += 2;
	}

	// read each sub-block from the compressed file
	int result = -1;
	for (u32 i = 0; i < blockCount; i++)
	{
		if (result = CMP_DecSdBlkStd(&inData, outData, outEnd, 16) < 0) return true;
	}

	u8 codeHi = *inData++;
	u8 codeLo = *inData++;
	if (codeLo > 0) {
		if (result = CMP_DecSdBlkStd(&inData, outData, outEnd, codeLo) < 0) return true;
	}

	if (codeHi > result) {
		*outData -= codeHi - result;
	}
	return true;
}

void CMP_ReadFile(u8* inData) {
	int header = strcmp(inData, "CRICMP");
	if (header == 0) {
		CMP_version_no = atof(inData + 8);
	} else {
		CMP_version_no = 1.0;
		trace("This isn't a CRICMP file");
		return;
	}

	CMP_file_size = *(u32*)(inData + 0x14);
	CMP_file_buffer = malloc(CMP_file_size);
	if (CMP_file_buffer == NULL) {
		CMP_file_size = 0;

		trace("out of memory");
		return;
	}

	memset(CMP_file_buffer, 0xEE, CMP_file_size);

	u32 dataStart = *(u32*)(inData + 0x18);
	u8* outData = CMP_file_buffer;

	CMP_file_in = inData;
	CMP_DecSlideSub(inData + dataStart, &outData, CMP_file_size);
}

u8* CMP_GetFileBuffer() {
	return CMP_file_buffer;
}
int CMP_GetFileSize() {
	return CMP_file_size;
}
