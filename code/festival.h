/* date = August 27th 2023 11:39 am */

#ifndef FESTIVAL_H
#define FESTIVAL_H

#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 
#define IsAnyAltKeyDown (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) 
#define IsAnyControlKeyDown (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))

#define IsAlphaNumeric(c) (\
((c) >= 'a' && (c) <= 'z') || \
((c) >= 'A' && (c) <= 'Z') || \
((c) >= '0' && (c) <= '9')\
)
#define IsNonSpecial(c) (IsAlphaNumeric(c) || \
c == '_')

typedef Color color;
struct color_list;
struct program_state;
struct view;

DefineList(int, Int)
DefineList(f32, f32)
DefineList(rect, Rect)
DefineList(color, Color)

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
    string Path;
    string_list Lines;
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


enum lister_type
{
    ListerType_String,
    ListerType_BufferPointer,
    ListerType_Command,
    ListerType_FontType,
};
enum lister_purpose {
    ListerPurpose_EditFile,
    ListerPurpose_SwitchBuffer,
    ListerPurpose_RunCommand,
    ListerPurpose_SwitchFontType,
};
struct lister_entry
{
    string Name;
    lister_type Type;
    union {
        struct {
            buffer *Buffer;
        };
        struct {
            string String;
        };
        struct {
            command Command;
        };
        struct {
            font_type FontType;
        };
    };
};
DefineList(lister_entry, ListerEntry)

struct lister
{
    lister_type Type;
    lister_purpose Purpose;
    
    string InputLabel;
    string Input;
    lister_entry_list Entries;
    int_list MatchingEntries;
    
    int HoverIndex;
    int SelectedIndex;
    b32 ShouldExecute;
    
    int Y;
    int TargetY;
    rect_list Rects; // correspond to MatchingEntries
};


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
    
    int Y;
    int TargetY;
    rect CursorRect;
    rect CursorTargetRect;
    
    line_data_list LineDataList;
    
    
    b32 ListerIsOpen;
    lister Lister;
};
DefineList(view, View)

struct key_data
{
    int KeyCode;
    b32 JustPressed;
    f32 PressTime;
    f32 TimeTillNextRepeat;
};

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
    
    b32 ShowViewInfo;
    b32 ShowViewRects;
    
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
                key_data PageUp_Key;
                key_data PageDown_Key;
                
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
                
                key_data Number0Key;
                key_data Number1Key;
                key_data Number2Key;
                key_data Number3Key;
                key_data Number4Key;
                key_data Number5Key;
                key_data Number6Key;
                key_data Number7Key;
                key_data Number8Key;
                key_data Number9Key;
                
                key_data Grave_Key;
                key_data Minus_Key;
                key_data Equals_Key;
                key_data LeftBracket_Key;
                key_data RightBracket_Key;
                key_data Backslash_Key;
                key_data Semicolon_Key;
                key_data Quote_Key;
                key_data Slash_Key;
                key_data Comma_Key;
                key_data Period_Key;
                
                key_data Space_Key;
                key_data Backspace_Key;
                key_data Delete_Key;
                key_data Tab_Key;
                key_data Return_Key;
                key_data CapsLock_Key;
                key_data Escape_Key;
            };
            struct
            {
                key_data NavKeys[6];
                key_data LetterKeys[26];
                key_data NumberKeys[10];
                key_data SymbolKeys[11];
                key_data SpecialKeys[7];
            };
        };
        key_data KeyData[6+26+10+11+7];
    };
};


#define KeyShouldExecute(Key) ((Key).JustPressed || ((Key).PressTime >= ProgramState->KeyFirstRepeatTime && Key.TimeTillNextRepeat <= 0))

#endif //FESTIVAL_H