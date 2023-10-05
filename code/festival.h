/* date = August 27th 2023 11:39 am */

#ifndef FESTIVAL_H
#define FESTIVAL_H

#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 

struct buffer
{
    b32 IsOpen;
    
    f32 ViewPos;
    
    union {
        struct {
            int Length;
            char *Data;
        };
        string Text;
    };
    int LineCount;
};

#define MAX_BUFFERS 30
struct program_state
{
    f32 dTime;
    i32 ScreenWidth, ScreenHeight;
    
    Font FontMain;
    Font FontSDF;
    Shader ShaderSDF;
    
    buffer Buffers[MAX_BUFFERS];
    int OpenBufferCount;
};

#endif //FESTIVAL_H