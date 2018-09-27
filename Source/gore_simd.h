#ifndef GORE_SIMD_H_INCLUDED
#define GORE_SIMD_H_INCLUDED

//NOTE(dima): Get i element from mm SIMD float vector
#define MM_GetI(mm, i) (mm).m128_f32[i]

//NOTE(dima): Get i element from mm SIMD integer vector
#define MMI_GetI(mm, i) (mm).m128i_u32[i]

inline __m128i MM_MulI(__m128i A, __m128i B) {
	__m128i Result = _mm_or_si128(
		_mm_mullo_epi16(A, B),
		_mm_slli_epi32(_mm_mulhi_epi16(A, B), 16));

	return(Result);
}

#endif