/* date = August 27th 2023 11:39 am */

#ifndef FESTIVAL_H
#define FESTIVAL_H

enum input_mode
{
    InputMode_Nav,
    InputMode_Select,
    InputMode_Insert,
    InputMode_EntryBar,
};

#define MAX_BUFFERS 50
struct program_state
{
    b32 ShouldExit;
    f32 dTime;
    i32 ScreenWidth, ScreenHeight;
    
    f32 KeyFirstRepeatTime;
    f32 KeyRepeatSpeed = 1;
    
    buffer_list Buffers;
    
    view_list Views;
    int SelectedViewIndex;
    
    b32 HasTargetGlobalCursorPositionBeenSet;
    rect GlobalCursor;
    rect TargetGlobalCursor;
    
    b32 UserMovedCursor;
    b32 UserMovedView;
    
    b32 ShouldChangeIdealCursorCol;
    
    b32 ShowViewInfo;
    b32 ShowViewRects;
    
    b32 ShowSuperDebugMenu;
    int SuperDebugMenuY;
    int SuperDebugMenuH;
    
    input_mode InputMode;
    
    settings Settings;
    
    RenderTexture2D CharTextures[256];
    b32 CharTexturesExist[256];
    
    input Input;
};


#endif //FESTIVAL_H