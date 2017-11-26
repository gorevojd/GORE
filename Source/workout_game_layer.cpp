#include "workout_game_layer.h"

#include <stdio.h>

data_buffer ReadFileToDataBuffer(char* FileName) {
	data_buffer Result = {};

	FILE* fp = fopen(FileName, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		u64 FileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		Result.Size = FileSize;
		Result.Data = (u8*)calloc(FileSize, 1);

		fread(Result.Data, 1, FileSize, fp);

		fclose(fp);
	}

	return(Result);
}

void FreeDataBuffer(data_buffer* DataBuffer) {
	if (DataBuffer->Data) {
		free(DataBuffer->Data);
	}
}

#ifndef GET_ALIGN_OFFSET
#define GET_ALIGN_OFFSET(val, align) (((align) - ((size_t)val & ((align) - 1))) & (Align - 1))
#endif

rgba_buffer AllocateRGBABuffer(u32 Width, u32 Height, u32 Align) {
	rgba_buffer Result = {};

	Result.Width = Width;
	Result.Height = Height;
	Result.Pitch = 4 * Width;

	Result.Align.x = 0.0f;
	Result.Align.y = 0.0f;

	u32 MemoryForBufRequired = Width * Height * 4;
	u32 AlignedMemoryBufSize = MemoryForBufRequired + Align;
	Result.Pixels = (u8*)calloc(AlignedMemoryBufSize, 1);

	u32 AlignOffset = GET_ALIGN_OFFSET(Result.Pixels, Align);
	Result.Pixels += AlignOffset;

	return(Result);
}

void DeallocateRGBABuffer(rgba_buffer* Buffer) {
	if (Buffer->Pixels) {
		free(Buffer->Pixels);
	}
}