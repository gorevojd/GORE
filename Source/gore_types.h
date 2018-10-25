#ifndef GORE_TYPES_H_INCLUDED
#define GORE_TYPES_H_INCLUDED

#define GLOBAL_VARIABLE static

#define STRONG_ASSERT(cond) if(!(cond)){ *((int*)0) = 0;}

#define Assert(cond) if(!(cond)){ *((int*)0) = 0;}
#define ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))
#define InvalidCodePath Assert(!"Invalid code path!");
#define INVALID_CODE_PATH Assert(!"Invalid code path!");


#ifndef PRETTY_TYPES_DEFINED
#define PRETTY_TYPES_DEFINED

typedef int b32;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed char i8;
typedef signed short s16;
typedef signed short i16;
typedef signed int s32;
typedef signed int i32;
typedef signed long long s64;
typedef signed long long i64;
#endif


#ifndef Min
#define Min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef Max
#define Max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef Abs
#define Abs(a) ((a) >= 0) ? (a) : -(a)
#endif

#ifndef GET_ALIGN_OFFSET
#define GET_ALIGN_OFFSET(val, align) ((align - ((size_t)val & (align - 1))) & (align - 1))
#endif 

#ifndef MEGABYTES
#define MEGABYTES(count) ((count) * 1024 * 1024)
#endif

#ifndef KILOBYTES
#define KILOBYTES(count) ((count) * 1024)
#endif

#endif