/* date = July 11th 2024 11:16 pm */

#ifndef FESTIVAL_VIEW_H
#define FESTIVAL_VIEW_H


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



#endif //FESTIVAL_VIEW_H
