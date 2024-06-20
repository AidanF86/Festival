/* date = August 27th 2023 11:39 am */

#ifndef FESTIVAL_H
#define FESTIVAL_H


struct program_state;
struct view;

struct line_data
{
    rect_list CharRects;
    color_list CharColors;
    // TODO: merge EndLineRect into CharRects?
    rect EndLineRect;
    rect LineRect;
    
    int DisplayLines;
};
DefineList(line_data, LineData)

struct buffer
{
    string FileName;
    string DirPath;
    
    string_list Lines;
    
    int ActionIndex;
    action_list ActionStack;
};
DefineList(buffer, Buffer)

typedef int (*command_function)(program_state*, view*);
struct command
{
    command_function Function;
    const char *Name;
};

enum font_type
{
    FontType_Monospace,
    FontType_Sans,
    FontType_Serif,
};

#include "festival_lister.h"

enum view_spawn_location {
    Location_Below,
    Location_Right,
};

struct view
{
    buffer *Buffer;
    
    int Id;
    int ParentId; // -1 means this is root view
    view_spawn_location SpawnLocation;
    int BirthOrdinal; // [this]-th child. Determines placement and parental succession
    f32 Area; // fraction of parent
    b32 ComputedFromParentThisFrame;
    
    rect Rect;
    rect TextRect;
    
    buffer_pos CursorPos;
    int IdealCursorCol;
    
    b32 Selecting;
    buffer_pos SelectionStartPos;
    
    string InsertModeString;
    buffer_pos InsertModeStartPos;
    b32 InsertModeLineBelow;
    
    int Y;
    int TargetY;
    rect CursorRect;
    rect CursorTargetRect;
    
    line_data_list LineDataList;
    
    b32 ListerIsOpen;
    lister Lister;
};
DefineList(view, View)

enum input_mode
{
    InputMode_Nav,
    InputMode_Select,
    InputMode_Insert,
    InputMode_EntryBar,
};


struct font
{
    Font RFont;
    int AsciiGlyphIndexes[256];
};

#define MAX_BUFFERS 50
struct program_state
{
    b32 ShouldExit;
    f32 dTime;
    i32 ScreenWidth, ScreenHeight;
    
    f32 KeyFirstRepeatTime;
    f32 KeyRepeatSpeed = 1;
    
    font FontMonospace;
    font FontSans;
    font FontSerif;
    font_type FontType;
    
    int FontSize;
    int PrevFontSize;
    int CharsPerVirtualLine;
    int SubLineOffset;
    int MarginLeft;
    int NumbersWidth;
    
    buffer_list Buffers;
    
    view_list Views;
    int SelectedViewIndex;
    
    b32 UserMovedCursor;
    b32 UserMovedView;
    
    //b32 UserMovedVertical;
    b32 ShouldChangeIdealCursorCol;
    
    
    b32 ShowViewInfo;
    b32 ShowViewRects;
    b32 ShowSuperDebugMenu;
    int SuperDebugMenuY;
    
    // modal system
    input_mode InputMode;
    
    struct
    {
        color BGColor;
        color FGColor;
        color LineNumberBGColor;
        color LineNumberFGColor;
        color CursorBGColor;
        color CursorFGColor;
    };
    
    RenderTexture2D CharTextures[256];
    b32 CharTexturesExist[256];
    
    input Input;
};


#endif //FESTIVAL_H