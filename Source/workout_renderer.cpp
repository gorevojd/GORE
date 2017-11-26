#include "workout_renderer.h"

#define MM(mm, i) (mm).m128_f32[i]
#define MMI(mm, i) (mm).m128i_u32[i]

#define MM_UNPACK_COLOR_CHANNEL(texel, shift) _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(texel, shift), mmFF)), mmOneOver255)
#define MM_UNPACK_COLOR_CHANNEL0(texel) _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(texel, mmFF)), mmOneOver255)
#define MM_LERP(a, b, t) _mm_add_ps(a, _mm_mul_ps(_mm_sub_ps(b, a), t))

static void RenderClear(rgba_buffer* Buffer, v3 Color) {
	v4 ResColor = V4(Color.x, Color.y, Color.z, 1.0f);
	u32 OutColor = PackRGBA(ResColor);

	for (u32 DestY = 0; DestY < Buffer->Height; DestY++) {
		for (u32 DestX = 0; DestX < Buffer->Width; DestX++) {
			u32* OutDest = (u32*)Buffer->Pixels + DestY * Buffer->Width + DestX;

			*OutDest = OutColor;
		}
	}
}

static void RenderClearFast(rgba_buffer* Buffer, v3 Color) {
	v4 ResColor = V4(Color.x, Color.y, Color.z, 1.0f);
	u32 OutColor = PackRGBA(ResColor);

	__m128i mmOutColor;

	__m128 mm255 = _mm_set1_ps(255.0f);

	__m128 mmOutColor_r = _mm_set1_ps(Color.r);
	__m128 mmOutColor_g = _mm_set1_ps(Color.g);
	__m128 mmOutColor_b = _mm_set1_ps(Color.b);
	__m128 mmOutColor_a = _mm_set1_ps(1.0f);

	__m128i mmOutColorSh_r = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(mmOutColor_r, mm255)), 24);
	__m128i mmOutColorSh_g = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(mmOutColor_g, mm255)), 16);
	__m128i mmOutColorSh_b = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(mmOutColor_b, mm255)), 8);
	__m128i mmOutColorSh_a = _mm_cvtps_epi32(_mm_mul_ps(mmOutColor_a, mm255));

	mmOutColor = _mm_or_si128(
		_mm_or_si128(mmOutColorSh_r, mmOutColorSh_g),
		_mm_or_si128(mmOutColorSh_b, mmOutColorSh_a));

	for (u32 DestY = 0; DestY < Buffer->Height; DestY++) {
		for (u32 DestX = 0; DestX < Buffer->Width; DestX += 4) {
			u32* OutDest = (u32*)Buffer->Pixels + DestY * Buffer->Width + DestX;

			_mm_storeu_si128((__m128i*)OutDest, mmOutColor);
		}
	}
}

static void RenderGradient(rgba_buffer* Buffer, v3 Color) {
	float DeltaU;
	float DeltaV;

	float OneOverWidth = 1.0f / (float)Buffer->Width;
	float OneOverHeight = 1.0f / (float)Buffer->Height;

	for (int VertIndex = 0; VertIndex < Buffer->Height; VertIndex++) {

		DeltaV = (float)VertIndex * OneOverHeight;

		for (int HorzIndex = 0; HorzIndex < Buffer->Width; HorzIndex++) {
			DeltaU = (float)HorzIndex * OneOverWidth;

			u32* Pixel = (u32*)(Buffer->Pixels + VertIndex * Buffer->Width * 4);

			v4 OutColor;
			OutColor.x = DeltaU * Color.x;
			OutColor.y = DeltaU * Color.y;
			OutColor.z = DeltaU * Color.z;
			OutColor.w = 1.0f;

			u32 ColorByteRep = PackRGBA(OutColor);

			*Pixel++ = ColorByteRep;
		}
	}
}

static void RenderGradientFast(rgba_buffer* Buffer, v3 Color) {
	float DeltaU;
	float DeltaV;

	__m128 mmOne = _mm_set1_ps(1.0f);
	__m128 mm255 = _mm_set1_ps(255.0f);

	__m128 mmColor_r = _mm_set1_ps(Color.r);
	__m128 mmColor_g = _mm_set1_ps(Color.g);
	__m128 mmColor_b = _mm_set1_ps(Color.b);

	float OneOverWidth = 1.0f / (float)Buffer->Width;
	float OneOverHeight = 1.0f / (float)Buffer->Height;

	__m128 mmOneOverWidth = _mm_set1_ps(OneOverWidth);
	__m128 mmOneOverHeight = _mm_set1_ps(OneOverHeight);

	for (int VertIndex = 0; VertIndex < Buffer->Height; VertIndex++) {

		DeltaV = (float)VertIndex * OneOverHeight;

		for (int HorzIndex = 0; HorzIndex < Buffer->Width; HorzIndex += 4) {
			
			u32* Pixel = (u32*)(Buffer->Pixels + VertIndex * Buffer->Width * 4 + HorzIndex * 4);
			
			__m128i mmHorzIndex = _mm_setr_epi32(HorzIndex, HorzIndex + 1, HorzIndex + 2, HorzIndex + 3);
			__m128 mmDeltaU = _mm_mul_ps(_mm_cvtepi32_ps(mmHorzIndex), mmOneOverWidth);

			__m128 mmOutColor_r = _mm_mul_ps(mmDeltaU, mmColor_r);
			__m128 mmOutColor_g = _mm_mul_ps(mmDeltaU, mmColor_g);
			__m128 mmOutColor_b = _mm_mul_ps(mmDeltaU, mmColor_b);
			__m128 mmOutColor_a = mmOne;

			__m128i mmOutColorShifted_r = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(mmOutColor_r, mm255)), 24);
			__m128i mmOutColorShifted_g = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(mmOutColor_g, mm255)), 16);
			__m128i mmOutColorShifted_b = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(mmOutColor_b, mm255)), 8);
			__m128i mmOutColorShifted_a = _mm_cvtps_epi32(_mm_mul_ps(mmOutColor_a, mm255));

			__m128i mmOutColor = _mm_or_si128(
				_mm_or_si128(mmOutColorShifted_a, mmOutColorShifted_b),
				_mm_or_si128(mmOutColorShifted_g, mmOutColorShifted_r));

			_mm_storeu_si128((__m128i*)Pixel, mmOutColor);
		}
	}
}


static void RenderBitmapFast(
	rgba_buffer* Buffer,
	rgba_buffer* Bitmap,
	v2 P,
	float TargetBitmapPixelHeight,
	v4 ModulationColor01 = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
	float TargetScaling = (float)TargetBitmapPixelHeight / (float)Bitmap->Height;
	u32 TargetWidth = (float)Bitmap->Width * TargetScaling;
	u32 TargetHeight = TargetBitmapPixelHeight;

	__m128 mmTargetScaling = _mm_set1_ps(TargetScaling);
	__m128 mmTargetWidth = _mm_set1_ps((float)TargetWidth);
	__m128 mmTargetHeight = _mm_set1_ps(TargetBitmapPixelHeight);

	__m128 mmZero = _mm_set1_ps(0.0f);
	__m128 mmOne = _mm_set1_ps(1.0f);
	__m128i mmOneI = _mm_set1_epi32(1);
	__m128 mm255 = _mm_set1_ps(255.0f);
	__m128 mmOneOver255 = _mm_set1_ps(1.0f / 255.0f);
	__m128i mmFF = _mm_set1_epi32(0xFF);

	__m128 mmSourceWidth = _mm_set1_ps((float)Bitmap->Width);
	__m128 mmSourceHeight = _mm_set1_ps((float)Bitmap->Height);
	__m128i mmSourcePitch = _mm_set1_epi32(Bitmap->Pitch);

	__m128i mmSourceWidthI = _mm_set1_epi32(Bitmap->Width);
	__m128i mmSourceHeightI = _mm_set1_epi32(Bitmap->Height);

	__m128i mmSourceWidthMinusOneI = _mm_sub_epi32(mmSourceWidthI, mmOneI);
	__m128i mmSourceHeightMinusOneI = _mm_sub_epi32(mmSourceHeightI, mmOneI);

	__m128 mmOneOverSourceWidth = _mm_set1_ps(1.0f / (float)Bitmap->Width);
	__m128 mmOneOverSourceHeight = _mm_set1_ps(1.0f / (float)Bitmap->Height);

	__m128 mmOneOverWidth = _mm_div_ps(mmOne, mmTargetWidth);
	__m128 mmOneOverHeight = _mm_div_ps(mmOne, mmTargetHeight);

	__m128i mmInitX = _mm_cvttps_epi32(_mm_set1_ps(P.x));
	__m128i mmInitY = _mm_cvttps_epi32(_mm_set1_ps(P.y));

	int InitX = P.x;
	int InitY = P.y;

	int MinX = InitX;
	int MaxX = MinX + TargetWidth;

	int MinY = InitY;
	int MaxY = MinY + TargetHeight;

	if (MinX < 0) {
		MinX = 0;
	}

	if (MaxX > Buffer->Width) {
		MaxX = Buffer->Width;
	}

	if (MinY < 0) {
		MinY = 0;
	}

	if (MaxY > Buffer->Height) {
		MaxY = Buffer->Height;
	}
	
	__m128 mmModulationColor_r = _mm_set1_ps(ModulationColor01.r);
	__m128 mmModulationColor_g = _mm_set1_ps(ModulationColor01.g);
	__m128 mmModulationColor_b = _mm_set1_ps(ModulationColor01.b);
	__m128 mmModulationColor_a = _mm_set1_ps(ModulationColor01.a);

	mmModulationColor_r = _mm_mul_ps(mmModulationColor_r, mmModulationColor_a);
	mmModulationColor_g = _mm_mul_ps(mmModulationColor_g, mmModulationColor_a);
	mmModulationColor_b = _mm_mul_ps(mmModulationColor_b, mmModulationColor_a);

	mmModulationColor_r = _mm_min_ps(mmOne, _mm_max_ps(mmZero, mmModulationColor_r));
	mmModulationColor_g = _mm_min_ps(mmOne, _mm_max_ps(mmZero, mmModulationColor_g));
	mmModulationColor_b = _mm_min_ps(mmOne, _mm_max_ps(mmZero, mmModulationColor_b));
	mmModulationColor_a = _mm_min_ps(mmOne, _mm_max_ps(mmZero, mmModulationColor_a));

	for (int DestY = MinY; DestY < MaxY; DestY++) {

		__m128i mmDestY = _mm_set1_epi32(DestY);
		__m128 mmPixelV = _mm_mul_ps(_mm_cvtepi32_ps(_mm_sub_epi32(mmDestY, mmInitY)), mmOneOverHeight);
		mmPixelV = _mm_min_ps(mmOne, _mm_max_ps(mmZero, mmPixelV));
		__m128 mmSourceY = _mm_mul_ps(mmPixelV, mmSourceHeight);

		for (int DestX = MinX; DestX < MaxX; DestX += 4) {

			__m128i mmDestX = _mm_setr_epi32(DestX, DestX + 1, DestX + 2, DestX + 3);
			__m128 mmPixelU = _mm_mul_ps(_mm_cvtepi32_ps(_mm_sub_epi32(mmDestX, mmInitX)), mmOneOverWidth);
			mmPixelU = _mm_min_ps(mmOne, _mm_max_ps(mmZero, mmPixelU));
			__m128 mmSourceX = _mm_mul_ps(mmPixelU, mmSourceWidth);

			__m128i mmMinSourceX_ = _mm_cvttps_epi32(mmSourceX);
			__m128i mmMinSourceY_ = _mm_cvttps_epi32(mmSourceY);

			/*This check is for min values. If min value overlap then we clamp it*/
			mmMinSourceX_ = _mm_min_epi32(mmMinSourceX_, mmSourceWidthMinusOneI);
			mmMinSourceY_ = _mm_min_epi32(mmMinSourceY_, mmSourceHeightMinusOneI);

			__m128 mmDeltaX = _mm_sub_ps(mmSourceX, _mm_cvtepi32_ps(mmMinSourceX_));
			__m128 mmDeltaY = _mm_sub_ps(mmSourceY, _mm_cvtepi32_ps(mmMinSourceY_));
			
			/*Here we clamp max value*/
			__m128i mmMaxSourceX_ = _mm_min_epi32(_mm_add_epi32(mmMinSourceX_, mmOneI), mmSourceWidthMinusOneI);
			__m128i mmMaxSourceY_ = _mm_min_epi32(_mm_add_epi32(mmMinSourceY_, mmOneI), mmSourceHeightMinusOneI);

			__m128i mmMinSrcXx4 = _mm_slli_epi32(mmMinSourceX_, 2);
			__m128i mmMaxSrcXx4 = _mm_slli_epi32(mmMaxSourceX_, 2);

			__m128i mmPitchByMinSrcY = _mm_or_si128(_mm_mullo_epi16(mmMinSourceY_, mmSourcePitch), _mm_slli_epi32(_mm_mulhi_epi16(mmMinSourceY_, mmSourcePitch), 16));
			__m128i mmPitchByMaxSrcY = _mm_or_si128(_mm_mullo_epi16(mmMaxSourceY_, mmSourcePitch), _mm_slli_epi32(_mm_mulhi_epi16(mmMaxSourceY_, mmSourcePitch), 16));

			__m128i mmTopLTexelSrcOffset = _mm_add_epi32(mmPitchByMinSrcY, mmMinSrcXx4);
			__m128i mmTopRTexelSrcOffset = _mm_add_epi32(mmPitchByMinSrcY, mmMaxSrcXx4);
			__m128i mmBotLTexelSrcOffset = _mm_add_epi32(mmPitchByMaxSrcY, mmMinSrcXx4);
			__m128i mmBotRTexelSrcOffset = _mm_add_epi32(mmPitchByMaxSrcY, mmMaxSrcXx4);

			__m128i mmTopLTexel = _mm_setr_epi32(
				*(u32*)(Bitmap->Pixels + MMI(mmTopLTexelSrcOffset, 0)),
				*(u32*)(Bitmap->Pixels + MMI(mmTopLTexelSrcOffset, 1)),
				*(u32*)(Bitmap->Pixels + MMI(mmTopLTexelSrcOffset, 2)),
				*(u32*)(Bitmap->Pixels + MMI(mmTopLTexelSrcOffset, 3)));

			__m128i mmTopRTexel = _mm_setr_epi32(
				*(u32*)(Bitmap->Pixels + MMI(mmTopRTexelSrcOffset, 0)),
				*(u32*)(Bitmap->Pixels + MMI(mmTopRTexelSrcOffset, 1)),
				*(u32*)(Bitmap->Pixels + MMI(mmTopRTexelSrcOffset, 2)),
				*(u32*)(Bitmap->Pixels + MMI(mmTopRTexelSrcOffset, 3)));

			__m128i mmBotLTexel = _mm_setr_epi32(
				*(u32*)(Bitmap->Pixels + MMI(mmBotLTexelSrcOffset, 0)),
				*(u32*)(Bitmap->Pixels + MMI(mmBotLTexelSrcOffset, 1)),
				*(u32*)(Bitmap->Pixels + MMI(mmBotLTexelSrcOffset, 2)),
				*(u32*)(Bitmap->Pixels + MMI(mmBotLTexelSrcOffset, 3)));

			__m128i mmBotRTexel = _mm_setr_epi32(
				*(u32*)(Bitmap->Pixels + MMI(mmBotRTexelSrcOffset, 0)),
				*(u32*)(Bitmap->Pixels + MMI(mmBotRTexelSrcOffset, 1)),
				*(u32*)(Bitmap->Pixels + MMI(mmBotRTexelSrcOffset, 2)),
				*(u32*)(Bitmap->Pixels + MMI(mmBotRTexelSrcOffset, 3)));

			__m128 mmTopLeft_r = MM_UNPACK_COLOR_CHANNEL(mmTopLTexel, 24);
			__m128 mmTopLeft_g = MM_UNPACK_COLOR_CHANNEL(mmTopLTexel, 16);
			__m128 mmTopLeft_b = MM_UNPACK_COLOR_CHANNEL(mmTopLTexel, 8);
			__m128 mmTopLeft_a = MM_UNPACK_COLOR_CHANNEL0(mmTopLTexel);

			__m128 mmTopRight_r = MM_UNPACK_COLOR_CHANNEL(mmTopRTexel, 24);
			__m128 mmTopRight_g = MM_UNPACK_COLOR_CHANNEL(mmTopRTexel, 16);
			__m128 mmTopRight_b = MM_UNPACK_COLOR_CHANNEL(mmTopRTexel, 8);
			__m128 mmTopRight_a = MM_UNPACK_COLOR_CHANNEL0(mmTopRTexel);

			__m128 mmBotLeft_r = MM_UNPACK_COLOR_CHANNEL(mmBotLTexel, 24);
			__m128 mmBotLeft_g = MM_UNPACK_COLOR_CHANNEL(mmBotLTexel, 16);
			__m128 mmBotLeft_b = MM_UNPACK_COLOR_CHANNEL(mmBotLTexel, 8);
			__m128 mmBotLeft_a = MM_UNPACK_COLOR_CHANNEL0(mmBotLTexel);

			__m128 mmBotRight_r = MM_UNPACK_COLOR_CHANNEL(mmBotRTexel, 24);
			__m128 mmBotRight_g = MM_UNPACK_COLOR_CHANNEL(mmBotRTexel, 16);
			__m128 mmBotRight_b = MM_UNPACK_COLOR_CHANNEL(mmBotRTexel, 8);
			__m128 mmBotRight_a = MM_UNPACK_COLOR_CHANNEL0(mmBotRTexel);

			/*First horizontal row blend*/
			__m128 mmUpperBlend_r = MM_LERP(mmTopLeft_r, mmTopRight_r, mmDeltaX);
			__m128 mmUpperBlend_g = MM_LERP(mmTopLeft_g, mmTopRight_g, mmDeltaX);
			__m128 mmUpperBlend_b = MM_LERP(mmTopLeft_b, mmTopRight_b, mmDeltaX);
			__m128 mmUpperBlend_a = MM_LERP(mmTopLeft_a, mmTopRight_a, mmDeltaX);

			/*Second horizontal row blend*/
			__m128 mmLowerBlend_r = MM_LERP(mmBotLeft_r, mmBotRight_r, mmDeltaX);
			__m128 mmLowerBlend_g = MM_LERP(mmBotLeft_g, mmBotRight_g, mmDeltaX);
			__m128 mmLowerBlend_b = MM_LERP(mmBotLeft_b, mmBotRight_b, mmDeltaX);
			__m128 mmLowerBlend_a = MM_LERP(mmBotLeft_a, mmBotRight_a, mmDeltaX);

			/*Vertical blend*/
			__m128 mmBlended_r = MM_LERP(mmUpperBlend_r, mmLowerBlend_r, mmDeltaY);
			__m128 mmBlended_g = MM_LERP(mmUpperBlend_g, mmLowerBlend_g, mmDeltaY);
			__m128 mmBlended_b = MM_LERP(mmUpperBlend_b, mmLowerBlend_b, mmDeltaY);
			__m128 mmBlended_a = MM_LERP(mmUpperBlend_a, mmLowerBlend_a, mmDeltaY);

			/*Multiplication by input color*/
			mmBlended_r = _mm_mul_ps(mmBlended_r, mmModulationColor_r);
			mmBlended_g = _mm_mul_ps(mmBlended_g, mmModulationColor_g);
			mmBlended_b = _mm_mul_ps(mmBlended_b, mmModulationColor_b);
			mmBlended_a = _mm_mul_ps(mmBlended_a, mmModulationColor_a);

			u32* OutDest = (u32*)(Buffer->Pixels + DestY * Buffer->Pitch + DestX * 4);

			__m128i mmPreDestColor = _mm_loadu_si128((__m128i*)OutDest);

			__m128 mmPreDestColor_r = MM_UNPACK_COLOR_CHANNEL(mmPreDestColor, 24);
			__m128 mmPreDestColor_g = MM_UNPACK_COLOR_CHANNEL(mmPreDestColor, 16);
			__m128 mmPreDestColor_b = MM_UNPACK_COLOR_CHANNEL(mmPreDestColor, 8);
			__m128 mmPreDestColor_a = MM_UNPACK_COLOR_CHANNEL0(mmPreDestColor);

			//float BlendAlpha = PreDestColor.a + BlendedColor.a - PreDestColor.a * BlendedColor.a;
			//__m128 mmBlendAlpha = _mm_sub_ps(_mm_add_ps(), _mm_mul_ps());
			__m128 mmBlendAlpha = mmBlended_a;

			/*Final alpha blend*/
			__m128 mmOneMinusBlendAlpha = _mm_sub_ps(mmOne, mmBlendAlpha);
			__m128 mmColor_r = _mm_add_ps(_mm_mul_ps(mmPreDestColor_r, mmOneMinusBlendAlpha), mmBlended_r);
			__m128 mmColor_g = _mm_add_ps(_mm_mul_ps(mmPreDestColor_g, mmOneMinusBlendAlpha), mmBlended_g);
			__m128 mmColor_b = _mm_add_ps(_mm_mul_ps(mmPreDestColor_b, mmOneMinusBlendAlpha), mmBlended_b);
			__m128 mmColor_a = mmOne;

			__m128i mmColorShifted_r = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(mmColor_r, mm255)), 24);
			__m128i mmColorShifted_g = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(mmColor_g, mm255)), 16);
			__m128i mmColorShifted_b = _mm_slli_epi32(_mm_cvtps_epi32(_mm_mul_ps(mmColor_b, mm255)), 8);
			__m128i mmColorShifted_a = _mm_cvtps_epi32(_mm_mul_ps(mmColor_a, mm255));

			__m128i mmResult = _mm_or_si128(
				_mm_or_si128(mmColorShifted_r, mmColorShifted_g),
				_mm_or_si128(mmColorShifted_b, mmColorShifted_a));

			//TODO(DIma): Mask with end of screen mask;

			//TODO(DIma): make this aligned
			_mm_storeu_si128((__m128i*)OutDest, mmResult);
		}
	}
}

static void RenderBitmap(
	rgba_buffer* Buffer,
	rgba_buffer* Bitmap,
	v2 P,
	float TargetBitmapPixelHeight,
	v4 ModulationColor01 = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
	float TargetScaling = (float)TargetBitmapPixelHeight / (float)Bitmap->Height;

	ModulationColor01.r *= ModulationColor01.a;
	ModulationColor01.g *= ModulationColor01.a;
	ModulationColor01.b *= ModulationColor01.a;

	/*Clamping incoming color*/
	ModulationColor01.r = Clamp01(ModulationColor01.r);
	ModulationColor01.g = Clamp01(ModulationColor01.g);
	ModulationColor01.b = Clamp01(ModulationColor01.b);
	ModulationColor01.a = Clamp01(ModulationColor01.a);

	u32 TargetWidth = (float)Bitmap->Width * TargetScaling;
	u32 TargetHeight = TargetBitmapPixelHeight;

	int InitX = P.x;
	int InitY = P.y;

	int MinX = InitX;
	int MaxX = MinX + TargetWidth;

	int MinY = InitY;
	int MaxY = MinY + TargetHeight;

	if (MinX < 0) {
		MinX = 0;
	}

	if (MaxX > Buffer->Width) {
		MaxX = Buffer->Width;
	}

	if (MinY < 0) {
		MinY = 0;
	}

	if (MaxY > Buffer->Height) {
		MaxY = Buffer->Height;
	}

	float SourceWidth = Bitmap->Width;
	float SourceHeight = Bitmap->Height;

	float OneOverSrcWidth = 1.0f / SourceWidth;
	float OneOverSrcHeight = 1.0f / SourceHeight;

	float OneOverWidth = 1.0f / (float)TargetWidth;
	float OneOverHeight = 1.0f / (float)TargetHeight;

	for (int DestY = MinY; DestY < MaxY; DestY++) {

		float PixelV = ((float)DestY - (float)InitY) * OneOverHeight;
		PixelV = Clamp01(PixelV);
		float SourceY = PixelV * SourceHeight;

		for (int DestX = MinX; DestX < MaxX; DestX++) {

			float PixelU = ((float)DestX - (float)InitX) * OneOverWidth;
			PixelU = Clamp01(PixelU);
			float SourceX = PixelU * SourceWidth;

			u32 MinSourceX_ = (u32)SourceX;
			u32 MinSourceY_ = (u32)SourceY;

			float DeltaX = SourceX - (float)MinSourceX_;
			float DeltaY = SourceY - (float)MinSourceY_;

			u32 MaxSourceX_ = Min(MinSourceX_ + 1, Bitmap->Width - 1);
			u32 MaxSourceY_ = Min(MinSourceY_ + 1, Bitmap->Height - 1);

			u32* TopLeft = (u32*)(Bitmap->Pixels + Bitmap->Pitch * MinSourceY_ + MinSourceX_ * 4);
			u32* TopRight = (u32*)(Bitmap->Pixels + Bitmap->Pitch * MinSourceY_ + MaxSourceX_ * 4);
			u32* BotLeft = (u32*)(Bitmap->Pixels + Bitmap->Pitch * MaxSourceY_ + MinSourceX_ * 4);
			u32* BotRight = (u32*)(Bitmap->Pixels + Bitmap->Pitch * MaxSourceY_ + MaxSourceX_ * 4);

			v4 TopLeftColor = UnpackRGBA(*TopLeft);
			v4 TopRightColor = UnpackRGBA(*TopRight);
			v4 BotLeftColor = UnpackRGBA(*BotLeft);
			v4 BotRightColor = UnpackRGBA(*BotRight);

			/*First row blend*/
			v4 UpperBlend;
			UpperBlend.r = TopLeftColor.r + (TopRightColor.r - TopLeftColor.r) * DeltaX;
			UpperBlend.g = TopLeftColor.g + (TopRightColor.g - TopLeftColor.g) * DeltaX;
			UpperBlend.b = TopLeftColor.b + (TopRightColor.b - TopLeftColor.b) * DeltaX;
			UpperBlend.a = TopLeftColor.a + (TopRightColor.a - TopLeftColor.a) * DeltaX;

			/*Second row blend*/
			v4 LowerBlend;
			LowerBlend.r = BotLeftColor.r + (BotRightColor.r - BotLeftColor.r) * DeltaX;
			LowerBlend.g = BotLeftColor.g + (BotRightColor.g - BotLeftColor.g) * DeltaX;
			LowerBlend.b = BotLeftColor.b + (BotRightColor.b - BotLeftColor.b) * DeltaX;
			LowerBlend.a = BotLeftColor.a + (BotRightColor.a - BotLeftColor.a) * DeltaX;

			/*Vertical blend*/
			v4 BlendedColor;
			BlendedColor.r = UpperBlend.r + (LowerBlend.r - UpperBlend.r) * DeltaY;
			BlendedColor.g = UpperBlend.g + (LowerBlend.g - UpperBlend.g) * DeltaY;
			BlendedColor.b = UpperBlend.b + (LowerBlend.b - UpperBlend.b) * DeltaY;
			BlendedColor.a = UpperBlend.a + (LowerBlend.a - UpperBlend.a) * DeltaY;
			
			BlendedColor.r = BlendedColor.r * ModulationColor01.r;
			BlendedColor.g = BlendedColor.g * ModulationColor01.g;
			BlendedColor.b = BlendedColor.b * ModulationColor01.b;
			BlendedColor.a = BlendedColor.a * ModulationColor01.a;

			u32* OutDest = (u32*)(Buffer->Pixels + DestY * Buffer->Pitch + DestX * 4);
			v4 PreDestColor = UnpackRGBA(*OutDest);

			//float BlendAlpha = PreDestColor.a + BlendedColor.a - PreDestColor.a * BlendedColor.a;
			float BlendAlpha = BlendedColor.a;
			//Assert((BlendAlpha >= 0.0f) && (BlendAlpha <= 1.0f));

			/*Premultiplied alpha in action*/
			v4 AlphaBlendColor;
			AlphaBlendColor.r = (1.0f - BlendAlpha) * PreDestColor.r + BlendedColor.r;
			AlphaBlendColor.g = (1.0f - BlendAlpha) * PreDestColor.g + BlendedColor.g;
			AlphaBlendColor.b = (1.0f - BlendAlpha) * PreDestColor.b + BlendedColor.b;
			AlphaBlendColor.a = 1.0f;

			u32 DestPackedColor = PackRGBA(AlphaBlendColor);
			*OutDest = DestPackedColor;
		}
	}
}

void SoftwareRenderStackToOutput(render_stack* Stack, rgba_buffer* Buffer) {
	u8* At = Stack->Base;
	u8* StackEnd = Stack->Base + Stack->Used;

	while (At < StackEnd) {
		render_stack_entry_header* Header = (render_stack_entry_header*)At;

		u32 SizeOfEntryType = Header->SizeOfEntryType;

		At += sizeof(render_stack_entry_header);
		switch (Header->Type) {
			case(RenderStackEntry_Bitmap): {
				render_stack_entry_bitmap* EntryBitmap = (render_stack_entry_bitmap*)At;

#if 0
				RenderBitmap(
					Buffer, 
					EntryBitmap->Bitmap, 
					EntryBitmap->P,
					EntryBitmap->Height, 
					EntryBitmap->ModulationColor);
#else
				RenderBitmapFast(
					Buffer,
					EntryBitmap->Bitmap,
					EntryBitmap->P,
					EntryBitmap->Height,
					EntryBitmap->ModulationColor);
#endif
			}break;

			case(RenderStackEntry_Clear): {
				render_stack_entry_clear* EntryClear = (render_stack_entry_clear*)At;

#if 0
				RenderClear(Buffer, EntryClear->Color);
#else
				RenderClearFast(Buffer, EntryClear->Color);
#endif
			}break;

			case(RenderStackEntry_Gradient): {
				render_stack_entry_gradient* EntryGrad = (render_stack_entry_gradient*)At;

#if 0
				RenderGradient(Buffer, EntryGrad->Color);
#else
				RenderGradientFast(Buffer, EntryGrad->Color);
#endif
			}break;

			default: {
				Assert(!"Invalid entry type");
			}break;
		}

		At += Header->SizeOfEntryType;
	}
}