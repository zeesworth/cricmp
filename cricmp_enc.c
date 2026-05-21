#include "cricmp_enc.h"
#include "types.h"
#include "macros.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "endians.h"

u8* CMP_write_buffer = NULL;
u32 CMP_write_size = 0;

u8* CMP_write_in;

#define CMP_HEADER "CRICMP\0\0" "2.10\0\0\0\0"
#define CAST_PTR(type,data,offset) *((type*)(data+offset))

#define MAX_COPY_LEN (0xF+2)
#define MAX_REPE_LEN (0xFFFF+3)
#define MAX_OFFSET (0xFFF)

#pragma pack(push, 1)
struct Header {
	char header[8];
	char version[8];

	u32 align;
	u32 size;
	u32 startAddr;
	u32 pad00;
	u16 unk00;
	u32 blockCount; // this specific value is big endian for some reason
};

struct Block {
	u16 mask;
	u16 tokens[16];
};
#pragma pack(pop)

u32 check_match(u8* a, u8* b, u32 length) {
	u32* a32 = (u32*)a;
	u32* b32 = (u32*)b;

	u32 result = 0;
	while (length > 4) {
		if (*a32 != *b32) break;
		a32++;
		b32++;

		result += 4;
		length -= 4;
	}
	a = (u8*)a32; b = (u8*)b32;
	while (length > 0) {
		if (*a++ != *b++) return result;
		result++;
		length--;
	}
	return result;
}

u8 check_match_repe(u8* a, u8 b, u32 length) {
	u32* a32 = (u32*)a;
	u32 b32 = ((u32)b) * 0x01010101UL; // expand 8bits to 32bits

	while (length > 4) {
		if (*a32++ != b32) return FALSE;
		length -= 4;
	}
	a = (u8*)a32;
	while (length > 0) {
		if (*a++ != b) return FALSE;
		length--;
	}
	return TRUE;
}

void CMP_EncSlideSub(u8* inData, u32 inSize, u8** outData, u32* outSize) {
	*outSize = 38; // header size

	// write the header
	struct Header* header = (struct Header*)*outData;
	memset(header->header, 0, sizeof(header->header));
	memset(header->version, 0, sizeof(header->version));
	strncpy(header->header, "CRICMP", sizeof(header->header));
	strncpy(header->version, "2.10", sizeof(header->version));

	header->align = 0x20;
	header->size = inSize;
	header->startAddr = 0x20;
	header->pad00 = 0;
	header->unk00 = 0x0010;

	u16 blockMask = 0;
	struct Block* block = (struct Block*)(*outData + 0x26);
	u32 cBlock = 1;

	u32 tokenPos = 0;

	*outSize += 2;
	for (u32 iByte = 0; iByte < inSize; ){
		u32 remaining = inSize - iByte;

		if (tokenPos >= 16) {
			block->mask = blockMask;
			
			cBlock++;
			block++;
			*outSize += 2;

			blockMask = 0;
			tokenPos = 0;
		}

		// search for a match
		s32 copy_start = -1;
		u32 copy_len = 0;
		u32 repeat_len = 0;
		if (iByte > 0) {
			u32 matchBase = max(0,(s32)iByte - MAX_OFFSET);
			for (u32 iMatchStart = matchBase; iMatchStart < iByte; iMatchStart++) {
				if (inData[iMatchStart] != inData[iByte]) continue;

				u32 match_len = check_match(
					&inData[iMatchStart], &inData[iByte], 
					min(MAX_REPE_LEN, (s32)inSize - iByte)
				);
				
				// check for a valid copy
				if (match_len > 2 && match_len >= copy_len) {
					// try to find the closest match to the
					// current cursor for hopefully cache locality
					copy_start = iMatchStart;
					copy_len = min(MAX_COPY_LEN,match_len);
				}
				// check for a valid repeat
				if (iMatchStart > 0 && match_len > 3 && match_len > repeat_len
						&& check_match_repe(&inData[iMatchStart], inData[iByte - 1], match_len)) {
					repeat_len = match_len;
				}
			}
			
		}

		// only write a repeat if it will save us space
		// repeat len is stored as len - 3
		if (repeat_len >= 4 && repeat_len >= copy_len) {
			// this is a repeat
			u16 repeat_length = (repeat_len-3) & 0xFFFF;
			block->tokens[tokenPos] = SWAP_16(repeat_length);
			blockMask &= SWAP_16(~(1 << tokenPos)); // modify block mask

			iByte += repeat_len;
			*outSize += 2;
		} 
		// copy len is stored as len - 2
		else if (copy_len >= 3) {
			// this is a copy
			u16 copy_offset = iByte-(u16)copy_start;
			u8 copy_length = (copy_len-2) & 0xF;
			block->tokens[tokenPos] = SWAP_16((copy_offset & 0xFFF) | (copy_length << 12));
			blockMask &= SWAP_16(~(1 << tokenPos)); // modify block mask
			
			iByte += copy_len;
			*outSize += 2;
		} 
		else {
			// this is a literal
			if (remaining >= 2) {
				block->tokens[tokenPos] = (inData[iByte + 1] << 8) | inData[iByte];
				iByte += 2;
				*outSize += 2;
			} else {
				block->tokens[tokenPos] = inData[iByte];
				iByte += 1;
				*outSize += 1;
			}
			blockMask |= SWAP_16(1 << tokenPos); // modify block mask
		}
		tokenPos += 1;
	}
	block->mask = blockMask;
	header->blockCount = SWAP_32(cBlock);
}

void CMP_WriteFile(u8* inData, u32 inSize) {
	u32 allocSize = 40 + inSize + ((inSize / 32) * 2); // worst-case size
	CMP_write_buffer = malloc(allocSize);
	if (CMP_write_buffer == NULL) {
		CMP_write_size = 0;

		trace("out of memory");
		return;
	}

	memset(CMP_write_buffer, 0xEE, allocSize);

	CMP_write_in = inData;
	CMP_EncSlideSub(inData, inSize, &CMP_write_buffer, &CMP_write_size);
}

u8* CMP_GetWriteBuffer() {
	return CMP_write_buffer;
}
int CMP_GetWriteSize() {
	return CMP_write_size;
}
