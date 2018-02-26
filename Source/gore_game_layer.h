#ifndef GAME_LAYER_H
#define GAME_LAYER_H

#include "gore_platform.h"

struct data_buffer {
	u8* Data;
	u64 Size;
};

struct rgba_buffer {
	u8* Pixels;

	u32 Width;
	u32 Height;
	v2 Align;

	u32 Pitch;

	float WidthOverHeight;

	void* TextureHandle;
};

extern data_buffer ReadFileToDataBuffer(char* FileName);
extern void FreeDataBuffer(data_buffer* DataBuffer);
extern rgba_buffer AllocateRGBABuffer(u32 Width, u32 Height, u32 Align = 16);
extern void CopyRGBABuffer(rgba_buffer* Dst, rgba_buffer* Src);
extern void DeallocateRGBABuffer(rgba_buffer* Buffer);

#endif