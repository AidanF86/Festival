/* date = August 27th 2023 11:48 am */

#ifndef MISERY_BASE_H
#define MISERY_BASE_H

#include "festival_base.h"

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

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
