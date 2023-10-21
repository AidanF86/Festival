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
    int TargetViewPos;
    
    buffer_pos CursorPos;
    int IdealCursorCol;
    
    rect CursorRect;
    rect CursorTargetRect;
    
    string_list Lines;
    
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
        union
        {
            struct
            {
                key_data LeftKey;
                key_data RightKey;
                key_data UpKey;
                key_data DownKey;
                
                key_data AKey;
                key_data BKey;
                key_data CKey;
                key_data DKey;
                key_data EKey;
                key_data FKey;
                key_data GKey;
                key_data HKey;
                key_data IKey;
                key_data JKey;
                key_data KKey;
                key_data LKey;
                key_data MKey;
                key_data NKey;
                key_data OKey;
                key_data PKey;
                key_data QKey;
                key_data RKey;
                key_data SKey;
                key_data TKey;
                key_data UKey;
                key_data VKey;
                key_data WKey;
                key_data XKey;
                key_data YKey;
                key_data ZKey;
                
                key_data Number1Key;
                key_data Number2Key;
                key_data Number3Key;
                key_data Number4Key;
                key_data Number5Key;
                key_data Number6Key;
                key_data Number7Key;
                key_data Number8Key;
                key_data Number9Key;
                key_data Number0Key;
                //key_data _Key;
            };
            struct
            {
                key_data ArrowKeys[4];
                key_data LetterKeys[26];
                key_data NumberKeys[10];
            };
        };
        key_data KeyData[4+26+10];
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