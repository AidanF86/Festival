/* date = June 17th 2024 11:25 pm */

#ifndef LISTER_H
#define LISTER_H


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

lister Lister(lister_type Type)
{
    lister Result;
    Result.Type = Type;
    Result.Entries = ListerEntryList();
    Result.MatchingEntries = IntList();
    Result.Y = 0;
    Result.Y = Result.TargetY;
    Result.Rects = RectList();
    Result.SelectedIndex = 0;
    Result.HoverIndex = 0;
    return Result;
}




#endif //LISTER_H
