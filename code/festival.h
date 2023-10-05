/* date = August 27th 2023 11:39 am */

#ifndef FESTIVAL_H
#define FESTIVAL_H

#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 

struct program_state
{
    f32 dTime;
    i32 ScreenWidth, ScreenHeight;
    
    Font FontMain;
    Font FontSDF;
    
    Shader ShaderSDF;
    
    string Buffer;
};

#endif //FESTIVAL_H