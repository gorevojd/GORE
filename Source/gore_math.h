#ifndef GORE_MATH_H
#define GORE_MATH_H

#include "gore_types.h"
#include <math.h>

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.0174532925f
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.2958f
#endif

#define GORE_PI 3.14159265359f
#define GORE_TWO_PI 6.28318530718f

typedef union v2 {
	struct {
		float x;
		float y;
	};

	float E[2];
} v2;

typedef struct iv3 {
	int x;
	int y;
	int z;
} iv3;

typedef union v3 {
	struct {
		float x;
		float y;
		float z;
	};

	struct {
		float r;
		float g;
		float b;
	};

	float E[3];
} v3;

typedef union v4 {
	struct {
		union {
			struct {
				float x;
				float y;
				float z;
			};

			v3 xyz;
		};

		float w;
	};

	struct {
		union {
			struct {
				float r;
				float g;
				float b;
			};

			v3 rgb;
		};

		float a;
	};

	struct {
		union {
			struct {
				float A;
				float B;
				float C;
			};
			v3 ABC;
			v3 N;
		};

		float D;
	};

	float E[4];
} v4;

struct rect2 {
	v2 Min;
	v2 Max;
};

typedef union mat4 {
	float E[16];
	v4 Rows[4];
} mat4;

typedef union quat {
	struct {
		float x, y, z, w;
	};
	v3 xyz;
	v4 xyzw;
} quat;

//NOTE(dima): Helper functions
inline float Sqrt(float Value) {
	float Result;
	Result = sqrtf(Value);
	return(Result);
}

inline float RSqrt(float Value) {
	float Result;
	Result = 1.0f / sqrtf(Value);
	return(Result);
}

inline float Sin(float Rad) {
	float Result = sinf(Rad);
	return(Result);
}

inline float Cos(float Rad) {
	float Result = cosf(Rad);
	return(Result);
}

inline float Tan(float Rad) {
	float Result = tanf(Rad);
	return(Result);
}

inline float ASin(float Value) {
	float Result = asinf(Value);
	return(Result);
}

inline float ACos(float Value) {
	float Result = acosf(Value);
	return(Result);
}

inline float ATan(float Value) {
	float Result = atan(Value);
	return(Result);
}

inline float ATan2(float Y, float X) {
	float Result = atan2f(Y, X);
	return(Result);
}

inline float Exp(float Value) {
	float Result = expf(Value);
	return(Result);
}

inline float Log(float Value) {
	float Result = logf(Value);
	return(Result);
}

inline float Pow(float a, float b) {
	float Result = powf(a, b);
	return(Result);
}

//NOTE(dima): Constructors
inline v2 V2(float x, float y) {
	v2 Result;

	Result.x = x;
	Result.y = y;

	return(Result);
}

inline v3 V3(float x, float y, float z) {
	v3 Result;

	Result.x = x;
	Result.y = y;
	Result.z = z;

	return(Result);
}

inline v4 V4(float Value) {
	v4 Result;
	Result.x = Value;
	Result.y = Value;
	Result.z = Value;
	Result.w = Value;
	return(Result);
}

inline v4 V4(float x, float y, float z, float w) {
	v4 Result;

	Result.x = x;
	Result.y = y;
	Result.z = z;
	Result.w = w;

	return(Result);
}

inline v4 V4(v3 InitVector, float w) {
	v4 Result;
	Result.x = InitVector.x;
	Result.y = InitVector.y;
	Result.z = InitVector.z;
	Result.w = w;
	return(Result);
}

/*Dot operations*/
inline float Dot(v2 A, v2 B) {
	return A.x * B.x + A.y * B.y;
}

inline float Dot(v3 A, v3 B) {
	return A.x * B.x + A.y * B.y + A.z * B.z;
}

inline float Dot(v4 A, v4 B) {
	return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

/*Cross product*/
inline float Cross(v2 A, v2 B) { return A.x * B.y - B.x * A.y; }
inline v3 Cross(v3 A, v3 B) {
	v3 R;
	R.x = A.y * B.z - B.y * A.z;
	R.y = A.z * B.x - B.z * A.x;
	R.z = A.x * B.y - B.x * A.y;
	return(R);
}

/*Add operation*/
inline v2 Add(v2 a, v2 b) {
	a.x += b.x;
	a.y += b.y;
	return(a);
}

inline v3 Add(v3 a, v3 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return(a);
}

inline v4 Add(v4 a, v4 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return(a);
}

/*Subtract operation*/
inline v2 Sub(v2 a, v2 b) {
	a.x -= b.x;
	a.y -= b.y;
	return(a);
}

inline v3 Sub(v3 a, v3 b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return(a);
}

inline v4 Sub(v4 a, v4 b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return(a);
}

/*Multiply operation*/
inline v2 Mul(v2 a, float s) {
	a.x *= s;
	a.y *= s;
	return(a);
}

inline v3 Mul(v3 a, float s) {
	a.x *= s;
	a.y *= s;
	a.z *= s;
	return(a);
}

inline v4 Mul(v4 a, float s) {
	a.x *= s;
	a.y *= s;
	a.z *= s;
	a.w *= s;
	return(a);
}

/*Divide operation*/
inline v2 Div(v2 a, float s) {
	float OneOverS = 1.0f / s;
	a.x *= OneOverS;
	a.y *= OneOverS;
	return(a);
}

inline v3 Div(v3 a, float s) {
	float OneOverS = 1.0f / s;
	a.x *= OneOverS;
	a.y *= OneOverS;
	a.z *= OneOverS;
	return(a);
}

inline v4 Div(v4 a, float s) {

	float OneOverS = 1.0f / s;
	a.x *= OneOverS;
	a.y *= OneOverS;
	a.z *= OneOverS;
	a.w *= OneOverS;
	return(a);
}

/*Hadamard product*/
inline v2 Hadamard(v2 A, v2 B) { return (V2(A.x * B.x, A.y * B.y)); }
inline v3 Hadamard(v3 A, v3 B) { return (V3(A.x * B.x, A.y * B.y, A.z * B.z)); }
inline v4 Hadamard(v4 A, v4 B) { return (V4(A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w)); }

/*Magnitude of the vector*/
inline float Magnitude(v2 A) { return(Sqrt(Dot(A, A))); }
inline float Magnitude(v3 A) { return(Sqrt(Dot(A, A))); }
inline float Magnitude(v4 A) { return(Sqrt(Dot(A, A))); }

/*Squared magnitude*/
inline float SqMagnitude(v2 A) { return(Dot(A, A)); }
inline float SqMagnitude(v3 A) { return(Dot(A, A)); }
inline float SqMagnitude(v4 A) { return(Dot(A, A)); }

/*v2 operator overloading*/
inline bool operator==(v2 a, v2 b) { return((a.x == b.x) && (a.y == b.y)); }
inline bool operator!=(v2 a, v2 b) { return((a.x != b.x) || (a.y != b.y)); }

inline v2 operator+(v2 a) { return(a); }
inline v2 operator-(v2 a) { v2 r = { -a.x, -a.y }; return(r); }

inline v2 operator+(v2 a, v2 b) { return Add(a, b); }
inline v2 operator-(v2 a, v2 b) { return Sub(a, b); }

inline v2 operator*(v2 a, float s) { return Mul(a, s); }
inline v2 operator*(float s, v2 a) { return Mul(a, s); }
inline v2 operator/(v2 a, float s) { return Div(a, s); }

inline v2 operator*(v2 a, v2 b) { v2 r = { a.x * b.x, a.y * b.y }; return(r); }
inline v2 operator/(v2 a, v2 b) { v2 r = { a.x / b.x, a.y / b.y }; return(r); }

inline v2 &operator+=(v2& a, v2 b) { return(a = a + b); }
inline v2 &operator-=(v2& a, v2 b) { return(a = a - b); }
inline v2 &operator*=(v2& a, float s) { return(a = a * s); }
inline v2 &operator/=(v2& a, float s) { return(a = a / s); }

/*v3 operator overloading*/
inline bool operator==(v3 a, v3 b) { return((a.x == b.x) && (a.y == b.y) && (a.z == b.z)); }
inline bool operator!=(v3 a, v3 b) { return((a.x != b.x) || (a.y != b.y) || (a.z != b.z)); }

inline v3 operator+(v3 a) { return(a); }
inline v3 operator-(v3 a) { v3 r = { -a.x, -a.y, -a.z }; return(r); }

inline v3 operator+(v3 a, v3 b) { return Add(a, b); }
inline v3 operator-(v3 a, v3 b) { return Sub(a, b); }

inline v3 operator*(v3 a, float s) { return Mul(a, s); }
inline v3 operator*(float s, v3 a) { return Mul(a, s); }
inline v3 operator/(v3 a, float s) { return Div(a, s); }

inline v3 operator*(v3 a, v3 b) { v3 r = { a.x * b.x, a.y * b.y, a.z * b.z }; return(r); }
inline v3 operator/(v3 a, v3 b) { v3 r = { a.x / b.x, a.y / b.y, a.z / b.z }; return(r); }

inline v3 &operator+=(v3& a, v3 b) { return(a = a + b); }
inline v3 &operator-=(v3& a, v3 b) { return(a = a - b); }
inline v3 &operator*=(v3& a, float s) { return(a = a * s); }
inline v3 &operator/=(v3& a, float s) { return(a = a / s); }

/*v4 operator overloading*/
inline bool operator==(v4 a, v4 b) { return((a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w)); }
inline bool operator!=(v4 a, v4 b) { return((a.x != b.x) || (a.y != b.y) || (a.z != b.z) || (a.w != b.w)); }

inline v4 operator+(v4 a) { return(a); }
inline v4 operator-(v4 a) { v4 r = { -a.x, -a.y }; return(r); }

inline v4 operator+(v4 a, v4 b) { return Add(a, b); }
inline v4 operator-(v4 a, v4 b) { return Sub(a, b); }

inline v4 operator*(v4 a, float s) { return Mul(a, s); }
inline v4 operator*(float s, v4 a) { return Mul(a, s); }
inline v4 operator/(v4 a, float s) { return Div(a, s); }

inline v4 operator*(v4 a, v4 b) { v4 r = { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; return(r); }
inline v4 operator/(v4 a, v4 b) { v4 r = { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; return(r); }

inline v4 &operator+=(v4& a, v4 b) { return(a = a + b); }
inline v4 &operator-=(v4& a, v4 b) { return(a = a - b); }
inline v4 &operator*=(v4& a, float s) { return(a = a * s); }
inline v4 &operator/=(v4& a, float s) { return(a = a / s); }

/*Normalization of the vector*/
inline v2 Normalize(v2 v) { return(Mul(v, RSqrt(Dot(v, v)))); }
inline v3 Normalize(v3 v) { return(Mul(v, RSqrt(Dot(v, v)))); }
inline v4 Normalize(v4 v) { return(Mul(v, RSqrt(Dot(v, v)))); }

inline v2 NOZ(v2 v) { float sqmag = Dot(v, v); return((sqmag) < 0.0000001f ? V2(0.0f, 0.0f) : v * RSqrt(sqmag)); }
inline v3 NOZ(v3 v) { float sqmag = Dot(v, v); return((sqmag) < 0.0000001f ? V3(0.0f, 0.0f, 0.0f) : v * RSqrt(sqmag)); }
inline v4 NOZ(v4 v) { float sqmag = Dot(v, v); return((sqmag) < 0.0000001f ? V4(0.0f, 0.0f, 0.0f, 0.0f) : v * RSqrt(sqmag)); }

/*Vector LERPS*/
#if 0
inline v2 Lerp(v2 A, v2 B, float t) { return(A + (B - A) * t); }
inline v3 Lerp(v3 A, v3 B, float t) { return(A + (B - A) * t); }
inline v4 Lerp(v4 A, v4 B, float t) { return(A + (B - A) * t); }
#else
inline v2 Lerp(v2 A, v2 B, float t) { return((1.0f - t) * A + B * t); }
inline v3 Lerp(v3 A, v3 B, float t) { return((1.0f - t) * A + B * t); }
inline v4 Lerp(v4 A, v4 B, float t) { return((1.0f - t) * A + B * t); }
#endif
inline v2 GetRectDim(rect2 Rect) {
	v2 Result = Rect.Max - Rect.Min;

	return(Result);
}

inline float GetRectWidth(rect2 Rect) {
	float Result = Rect.Max.x - Rect.Min.x;

	return(Result);
}

inline float GetRectHeight(rect2 Rect) {
	float Result = Rect.Max.y - Rect.Min.y;

	return(Result);
}

inline rect2 Rect2MinMax(v2 Min, v2 Max) {
	rect2 Res;
	Res.Min = Min;
	Res.Max = Max;
	return(Res);
}

inline rect2 Rect2MinDim(v2 Min, v2 Dim) {
	rect2 Res;
	Res.Min = Min;
	Res.Max = V2(Min.x + Dim.x, Min.y + Dim.y);
	return(Res);
}

inline float Clamp01(float Val) {
	if (Val < 0.0f) {
		Val = 0.0f;
	}

	if (Val > 1.0f) {
		Val = 1.0f;
	}

	return(Val);
}

inline float Clamp(float Val, float Min, float Max) {
	if (Val < Min) {
		Val = Min;
	}

	if (Val > Max) {
		Val = Max;
	}

	return(Val);
}

inline int Clamp(int Val, int Min, int Max) {
	if (Val < Min) {
		Val = Min;
	}

	if (Val > Max) {
		Val = Max;
	}

	return(Val);
}

inline mat4 Multiply(mat4 M1, mat4 M2) {
	mat4 Result = {};

	Result.E[0] = M1.E[0] * M2.E[0] + M1.E[1] * M2.E[4] + M1.E[2] * M2.E[8] + M1.E[3] * M2.E[12];
	Result.E[1] = M1.E[0] * M2.E[1] + M1.E[1] * M2.E[5] + M1.E[2] * M2.E[9] + M1.E[3] * M2.E[13];
	Result.E[2] = M1.E[0] * M2.E[2] + M1.E[1] * M2.E[6] + M1.E[2] * M2.E[10] + M1.E[3] * M2.E[14];
	Result.E[3] = M1.E[0] * M2.E[3] + M1.E[1] * M2.E[7] + M1.E[2] * M2.E[11] + M1.E[3] * M2.E[15];

	Result.E[4] = M1.E[4] * M2.E[0] + M1.E[5] * M2.E[4] + M1.E[6] * M2.E[8] + M1.E[7] * M2.E[12];
	Result.E[5] = M1.E[4] * M2.E[1] + M1.E[5] * M2.E[5] + M1.E[6] * M2.E[9] + M1.E[7] * M2.E[13];
	Result.E[6] = M1.E[4] * M2.E[2] + M1.E[5] * M2.E[6] + M1.E[6] * M2.E[10] + M1.E[7] * M2.E[14];
	Result.E[7] = M1.E[4] * M2.E[3] + M1.E[5] * M2.E[7] + M1.E[6] * M2.E[11] + M1.E[7] * M2.E[15];

	Result.E[8] = M1.E[8] * M2.E[0] + M1.E[9] * M2.E[4] + M1.E[10] * M2.E[8] + M1.E[11] * M2.E[12];
	Result.E[9] = M1.E[8] * M2.E[1] + M1.E[9] * M2.E[5] + M1.E[10] * M2.E[9] + M1.E[11] * M2.E[13];
	Result.E[10] = M1.E[8] * M2.E[2] + M1.E[9] * M2.E[6] + M1.E[10] * M2.E[10] + M1.E[11] * M2.E[14];
	Result.E[11] = M1.E[8] * M2.E[3] + M1.E[9] * M2.E[7] + M1.E[10] * M2.E[11] + M1.E[11] * M2.E[15];

	Result.E[12] = M1.E[12] * M2.E[0] + M1.E[13] * M2.E[4] + M1.E[14] * M2.E[8] + M1.E[15] * M2.E[12];
	Result.E[13] = M1.E[12] * M2.E[1] + M1.E[13] * M2.E[5] + M1.E[14] * M2.E[9] + M1.E[15] * M2.E[13];
	Result.E[14] = M1.E[12] * M2.E[2] + M1.E[13] * M2.E[6] + M1.E[14] * M2.E[10] + M1.E[15] * M2.E[14];
	Result.E[15] = M1.E[12] * M2.E[3] + M1.E[13] * M2.E[7] + M1.E[14] * M2.E[11] + M1.E[15] * M2.E[15];

	return(Result);
}

inline v4 Multiply(mat4 M, v4 V) {
	v4 Result;

	Result.E[0] = V.E[0] * M.E[0] + V.E[0] * M.E[1] + V.E[0] * M.E[2] + V.E[0] * M.E[3];
	Result.E[1] = V.E[1] * M.E[4] + V.E[1] * M.E[5] + V.E[1] * M.E[6] + V.E[1] * M.E[7];
	Result.E[2] = V.E[2] * M.E[8] + V.E[2] * M.E[9] + V.E[2] * M.E[10] + V.E[2] * M.E[11];
	Result.E[3] = V.E[3] * M.E[12] + V.E[3] * M.E[13] + V.E[3] * M.E[14] + V.E[3] * M.E[15];

	return(Result);
}

inline mat4 Identity() {
	mat4 Result;

	Result.Rows[0] = { 1.0f, 0.0f, 0.0f, 0.0f };
	Result.Rows[1] = { 0.0f, 1.0f, 0.0f, 0.0f };
	Result.Rows[2] = { 0.0f, 0.0f, 1.0f, 0.0f };
	Result.Rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };

	return(Result);
}

inline mat4 Transpose(mat4 M) {
	mat4 Result;

	for (int RowIndex = 0; RowIndex < 4; RowIndex++) {
		for (int ColumtIndex = 0; ColumtIndex < 4; ColumtIndex++) {
			Result.E[ColumtIndex * 4 + RowIndex] = M.E[RowIndex * 4 + ColumtIndex];
		}
	}

	return(Result);
}

inline mat4 TranslationMatrix(v3 Translation) {
	mat4 Result = Identity();

	Result.E[3] = Translation.x;
	Result.E[7] = Translation.y;
	Result.E[11] = Translation.z;

	return(Result);
}

inline mat4 RotationX(float Angle) {
	mat4 Result;

	float CosT = Cos(Angle);
	float SinT = Sin(Angle);

	Result.E[0] = 1.0f;
	Result.E[1] = 0.0f;
	Result.E[2] = 0.0f;
	Result.E[3] = 0.0f;

	Result.E[4] = 0.0f;
	Result.E[5] = CosT;
	Result.E[6] = -SinT;
	Result.E[7] = 0.0f;

	Result.E[8] = 0.0f;
	Result.E[9] = SinT;
	Result.E[10] = CosT;
	Result.E[11] = 0.0f;

	Result.E[12] = 0.0f;
	Result.E[13] = 0.0f;
	Result.E[14] = 0.0f;
	Result.E[15] = 1.0f;

	return(Result);
}

inline mat4 RotationY(float Angle) {
	mat4 Result;

	float CosT = Cos(Angle);
	float SinT = Sin(Angle);

	Result.E[0] = CosT;
	Result.E[1] = 0.0f;
	Result.E[2] = -SinT;
	Result.E[3] = 0.0f;

	Result.E[4] = 0.0f;
	Result.E[5] = 1.0f;
	Result.E[6] = 0.0f;
	Result.E[7] = 0.0f;

	Result.E[8] = SinT;
	Result.E[9] = 0.0f;
	Result.E[10] = CosT;
	Result.E[11] = 0.0f;

	Result.E[12] = 0.0f;
	Result.E[13] = 0.0f;
	Result.E[14] = 0.0f;
	Result.E[15] = 1.0f;

	return(Result);
}


inline mat4 RotationZ(float Angle) {
	mat4 Result;

	float CosT = Cos(Angle);
	float SinT = Sin(Angle);

	Result.E[0] = CosT;
	Result.E[1] = -SinT;
	Result.E[2] = 0.0f;
	Result.E[3] = 0.0f;

	Result.E[4] = SinT;
	Result.E[5] = CosT;
	Result.E[6] = 0.0f;
	Result.E[7] = 0.0f;

	Result.E[8] = 0.0f;
	Result.E[9] = 0.0f;
	Result.E[10] = 1.0f;
	Result.E[11] = 0.0f;

	Result.E[12] = 0.0f;
	Result.E[13] = 0.0f;
	Result.E[14] = 0.0f;
	Result.E[15] = 1.0f;

	return(Result);
}

inline mat4 RotationMatrix(v3 R, float Angle) {
	mat4 Result;

	float CosT = Cos(Angle);
	float SinT = Sin(Angle);
	float InvCosT = 1.0f - Cos(Angle);

	float RxRyInvCos = R.x * R.y * InvCosT;
	float RxRzInvCos = R.x * R.z * InvCosT;
	float RyRzInvCos = R.y * R.z * InvCosT;

	Result.E[0] = CosT + R.x * R.x * InvCosT;
	Result.E[1] = RxRyInvCos - R.z * SinT;
	Result.E[2] = RxRzInvCos + R.y * SinT;
	Result.E[3] = 0;

	Result.E[4] = RxRyInvCos + R.z * SinT;
	Result.E[5] = CosT + R.y * R.y * InvCosT;
	Result.E[6] = RyRzInvCos - R.x * SinT;
	Result.E[7] = 0.0f;

	Result.E[8] = RxRzInvCos - R.y * SinT;
	Result.E[9] = RyRzInvCos + R.x * SinT;
	Result.E[10] = CosT + R.z * R.z * InvCosT;
	Result.E[11] = 0.0f;

	Result.E[12] = 0.0f;
	Result.E[13] = 0.0f;
	Result.E[14] = 0.0f;
	Result.E[15] = 1.0f;

	return(Result);
}

inline mat4 ScalingMatrix(v3 Scale) {
	mat4 Result = Identity();
	Result.E[0] = Scale.x;
	Result.E[5] = Scale.y;
	Result.E[10] = Scale.z;
	Result.E[15] = 1.0f;

	return(Result);
}

inline mat4 Translate(mat4 M, v3 P) {
	mat4 Result = M;

	Result.E[3] += P.x;
	Result.E[7] += P.y;
	Result.E[11] += P.z;

	return(Result);
}

inline mat4 operator*(mat4 M1, mat4 M2) {
	return(Multiply(M1, M2));
}

inline v4 operator*(mat4 M1, v4 V) {
	return(Multiply(M1, V));
}

inline mat4 LookAt(v3 Pos, v3 TargetPos, v3 WorldUp) {
	mat4 Result;

	v3 Fwd = TargetPos - Pos;
	Fwd = NOZ(Fwd);

	v3 Left = Normalize(Cross(WorldUp, Fwd));
	v3 Up = Normalize(Cross(Fwd, Left));

	v3 Eye = Pos;

	Result.E[0] = Left.x;
	Result.E[1] = Left.y;
	Result.E[2] = Left.z;
	Result.E[3] = -Dot(Left, Eye);

	Result.E[4] = Up.x;
	Result.E[5] = Up.y;
	Result.E[6] = Up.z;
	Result.E[7] = -Dot(Up, Eye);

	Result.E[8] = -Fwd.x;
	Result.E[9] = -Fwd.y;
	Result.E[10] = -Fwd.z;
	Result.E[11] = Dot(Fwd, Eye);

	Result.E[12] = 0.0f;
	Result.E[13] = 0.0f;
	Result.E[14] = 0.0f;
	Result.E[15] = 1.0f;

	return(Result);
}

inline mat4 PerspectiveProjection(u32 Width, u32 Height, float FOV, float Far, float Near)
{
	mat4 Result = {};

	float AspectRatio = (float)Width / (float)Height;

	float S = 1.0f / (Tan(FOV * 0.5f * DEG_TO_RAD));
	float A = S / AspectRatio;
	float B = S;
	float OneOverFarMinusNear = 1.0f / (Far - Near);
	Result.E[0] = A;
	Result.E[5] = B;
	Result.E[10] = -(Far + Near) * OneOverFarMinusNear;
	Result.E[11] = -(2.0f * Far * Near) * OneOverFarMinusNear;
	Result.E[14] = -1.0f;

	return(Result);
}

inline mat4 OrthographicProjection(
	u32 Right, u32 Left,
	u32 Top, u32 Bottom,
	float Far, float Near)
{
	mat4 Result = {};

	float OneOverRmL = 1.0f / ((float)Right - (float)Left);
	float OneOverTmB = 1.0f / ((float)Top - (float)Bottom);
	float OneOverFmN = 1.0f / (Far - Near);

	Result.E[0] = 2.0f * OneOverRmL;
	Result.E[3] = -(float)(Right + Left) * OneOverRmL;
	Result.E[5] = 2.0f * OneOverTmB;
	Result.E[7] = -(float)(Top + Bottom) * OneOverTmB;
	Result.E[10] = -2.0f * OneOverFmN;
	Result.E[11] = -(Far + Near) * OneOverFmN;
	Result.E[15] = 1.0f;

	return(Result);
}

inline mat4 OrthographicProjection(
	u32 Width, u32 Height,
	float Far, float Near)
{
	mat4 Result = {};

	float OneOverFmN = 1.0f / (Far - Near);
	Result.E[0] = 2.0f / (float)Width;
	Result.E[3] = -1.0f;
	Result.E[5] = 2.0f / (float)Height;
	Result.E[7] = -1.0f;
	Result.E[10] = -2.0f * OneOverFmN;
	Result.E[11] = -(Far + Near) * OneOverFmN;
	Result.E[15] = 1.0f;

	return(Result);
}

inline mat4 OrthographicProjection(u32 Width, u32 Height) {
	mat4 Result = {};

	Result.E[0] = 2.0f / (float)Width;
	Result.E[3] = -1.0f;
	Result.E[5] = 2.0f / (float)Height;
	Result.E[7] = -1.0f;
	Result.E[10] = 1.0f;
	Result.E[15] = 1.0f;

	return(Result);
}

//NOTE(dima): Plane math
inline v4 NormalizePlane(v4 Plane) {
	float NormalLen = Magnitude(Plane.rgb);

	v4 Result;
	Result.A = Plane.A / NormalLen;
	Result.B = Plane.B / NormalLen;
	Result.C = Plane.C / NormalLen;
	Result.D = Plane.D / NormalLen;

	return(Result);
}

inline float PlanePointTest(v4 Plane, v3 Point) {
	float Res = Dot(Plane.ABC, Point) + Plane.D;

	return(Res);
}

//NOTE(dima): Color math
inline u32 PackRGBA(v4 Color) {
	u32 Result;

	Result =
		((u32)(255.0f * Color.r) << 24) |
		((u32)(255.0f * Color.g) << 16) |
		((u32)(255.0f * Color.b) << 8) |
		((u32)(255.0f * Color.a));

	return(Result);
}

inline v4 UnpackRGBA(u32 Color) {
	v4 Result;

	float OneOver255 = 1.0f / 255.0f;

	Result.r = (Color >> 24) & 0xFF;
	Result.g = (Color >> 16) & 0xFF;
	Result.b = (Color >> 8) & 0xFF;
	Result.a = Color & 0xFF;

	Result.r *= OneOver255;
	Result.g *= OneOver255;
	Result.b *= OneOver255;
	Result.a *= OneOver255;

	return(Result);
}

#endif