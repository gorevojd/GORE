#ifndef GORE_RENDER_BLUR_H_INCLUDED
#define GORE_RENDER_BLUR_H_INCLUDED

#include "gore_asset_types.h"
#include "gore_asset_common.h"

u32 Calcualte2DGaussianBoxComponentsCount(int Radius);
void Normalize2DGaussianBox(float* Box, int Radius);
void Calculate2DGaussianBox(float* Box, int Radius);

bitmap_info BlurBitmapApproximateGaussian(
	bitmap_info* BitmapToBlur,
	void* ResultBitmapMem,
	void* TempBitmapMem,
	int Width, int Height,
	int BlurRadius);

bitmap_info BlurBitmapExactGaussian(
	bitmap_info* BitmapToBlur,
	void* ResultBitmapMem,
	int Width, int Height,
	int BlurRadius,
	float* GaussianBox);

#endif