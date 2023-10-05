/* date = August 27th 2023 11:39 am */

#ifndef FESTIVAL_H
#define FESTIVAL_H

#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 
#define IsAnyControlKeyDown (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) 

struct buffer
{
    b32 IsOpen;
    
    int ViewPos;
    f32 ViewSubPos;
    
    string_list Lines;
    
#if 0
    union {
        struct {
            int Length;
            char *Data;
        };
        string Text;
    };
    int LineCount;
#endif
};

#define MAX_BUFFERS 30
struct program_state
{
    f32 dTime;
    i32 ScreenWidth, ScreenHeight;
    
    Font FontMain;
    Font FontSDF;
    Shader ShaderSDF;
    
    int FontSize;
    int CharsPerVirtualLine;
    int SubLineOffset;
    int MarginLeft;
    int NumbersWidth;
    
    buffer Buffers[MAX_BUFFERS];
    int OpenBufferCount;
};

#endif //FESTIVAL_H