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
    
    rect CursorRect;
    buffer_pos CursorPos;
    
    string_list Lines;
    
    //int LineDataStart; // line start index
    // Make a list of this!
    line_data_list LineDataList;
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
    
    b32 UserMovedCursor;
    b32 UserMovedView;
};

#endif //FESTIVAL_H