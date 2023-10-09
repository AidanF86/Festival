/* date = August 27th 2023 11:39 am */

#ifndef FESTIVAL_H
#define FESTIVAL_H

#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 
#define IsAnyControlKeyDown (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) 


typedef struct rect_list
{
    b32 IsAllocated;
    int Count;
    int ArraySize;
    rect *Data;
    inline rect& operator[](size_t Index) { return Data[Index]; }
    inline const rect& operator[](size_t Index) const { return Data[Index]; }
} rect_list;

struct line_data
{
    rect_list CharRects;
    // TODO: merge EndLineRect into CharRects?
    rect EndLineRect;
    rect LineRect;
    
    int DisplayLines;
};

typedef struct line_data_list
{
    b32 IsAllocated;
    int Count;
    int ArraySize;
    line_data *Data;
    inline line_data& operator[](size_t Index) { return Data[Index]; }
    inline const line_data& operator[](size_t Index) const { return Data[Index]; }
} line_data_list;

struct buffer
{
    b32 IsOpen;
    
    rect Rect;
    rect TextRect;
    
    int ViewPos;
    
    buffer_pos CursorPos;
    int IdealCursorChar;
    
    rect CursorRect;
    rect CursorTargetRect;
    
    string_list Lines;
    
    //int LineDataStart; // line start index
    // Make a list of this!
    line_data_list LineDataList;
};

struct key_data
{
    int KeyCode;
    b32 JustPressed;
    f32 PressTime;
    f32 TimeTillNextRepeat;
};

#define MAX_BUFFERS 30
struct program_state
{
    f32 dTime;
    i32 ScreenWidth, ScreenHeight;
    
    f32 KeyFirstRepeatTime;
    f32 KeyRepeatSpeed = 1;
    
    union
    {
        struct
        {
            key_data LeftKey;
            key_data RightKey;
            key_data UpKey;
            key_data DownKey;
        };
        key_data KeyData[4];
    };
    
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
    
    b32 UserMovedCursor;
    b32 UserMovedView;
};

#define KeyShouldExecute(Key) ((Key).JustPressed || ((Key).PressTime >= ProgramState->KeyFirstRepeatTime && Key.TimeTillNextRepeat <= 0))

#endif //FESTIVAL_H