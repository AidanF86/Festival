/* date = July 12th 2024 5:32 pm */

#ifndef FESTIVAL_SETTINGS_H
#define FESTIVAL_SETTINGS_H

struct colors
{
    color ViewBG;
    color TextBG;
    color TextFG;
    color LineNumberBG;
    color LineNumberFG;
    color CursorActiveBG;
    color CursorActiveFG;
    color CursorInactiveBG;
    color CursorInactiveFG;
    color SelectionAreaActiveBG;
    color SelectionAreaActiveFG;
    color SelectionAreaInactiveBG;
    color SelectionAreaInactiveFG;
};

struct settings
{
    u32 TextSubLineOffset;
    u32 TextMarginLeft;
    u32 LineNumberWidth;
    u32 ScrollbarWidth;
    
    font Font;
    
    colors Colors;
};

#endif //FESTIVAL_SETTINGS_H
