#ifndef GORE_ASSET_COMMON_H_INCLUDED
#define GORE_ASSET_COMMON_H_INCLUDED

#include "gore_asset_identifiers.h"
#include "gore_math.h"

bitmap_info AllocateBitmap(u32 Width, u32 Height);
void CopyBitmap(bitmap_info* Dst, bitmap_info* Src);
void DeallocateBitmap(bitmap_info* Buffer);
bitmap_info LoadIMG(char* Path);

#endif