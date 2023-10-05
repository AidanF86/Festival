/* date = August 27th 2023 11:48 am */

#ifndef MISERY_BASE_H
#define MISERY_BASE_H

#include <stdint.h>

typedef float f32;
typedef double f64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int32_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef u32 b32;
typedef u64 b64;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define Assert(X) if(!X) { *0 = 0; }

struct program_memory
{
    u32 WindowHeight;
    u32 WindowWidth;
    
    FILE *LogFile;
    u64 StartTime;
    
    b32 IsRunning;
    
    u64 Size;
    void *Data;
    b32 Initialized;
};

#define PROGRAM_UPDATE_AND_RENDER(name) void name(program_memory *Memory)
typedef PROGRAM_UPDATE_AND_RENDER(program_update_and_render);

#endif //MISERY_BASE_H
