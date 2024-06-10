/* date = March 23rd 2024 2:11 pm */

#ifndef CCC_BASE_H
#include "ccc_base.h"
#endif

#ifdef RAYLIB_H

#ifndef CCC_INPUT_H
#define CCC_INPUT_H


#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 
#define IsAnyAltKeyDown (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) 
#define IsAnyControlKeyDown (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
#define KeyShouldExecute(Key) ((Key).JustPressed || ((Key).PressTime >= Input->KeyFirstRepeatTime && Key.TimeTillNextRepeat <= 0))

#define IsAlphaNumeric(c) (\
((c) >= 'a' && (c) <= 'z') || \
((c) >= 'A' && (c) <= 'Z') || \
((c) >= '0' && (c) <= '9')\
)
#define IsNonSpecial(c) (IsAlphaNumeric(c) || \
c == '_')




struct key_data
{
    int KeyCode;
    b32 JustPressed;
    b32 IsDown;
    f32 PressTime;
    f32 TimeTillNextRepeat;
};

struct input
{
    f32 KeyFirstRepeatTime;
    f32 KeyRepeatSpeed;
    
    v2 MousePos;
    
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


void
FillKeyData(input *Input)
{
    Input->KeyFirstRepeatTime = 0.4f;
    Input->KeyRepeatSpeed = 0.02f;
    for(int i = 0; i < sizeof(Input->KeyData) / sizeof(key_data); i++)
    {
        Input->KeyData[i] = {0};
    }
    Input->LeftKey.KeyCode = KEY_LEFT;
    Input->RightKey.KeyCode = KEY_RIGHT;
    Input->UpKey.KeyCode = KEY_UP;
    Input->DownKey.KeyCode = KEY_DOWN;
    Input->PageUp_Key.KeyCode = KEY_PAGE_UP;
    Input->PageDown_Key.KeyCode = KEY_PAGE_DOWN;
    //Input->Key.KeyCode = KEY_;
    Input->AKey.KeyCode = KEY_A;
    Input->BKey.KeyCode = KEY_B;
    Input->CKey.KeyCode = KEY_C;
    Input->DKey.KeyCode = KEY_D;
    Input->EKey.KeyCode = KEY_E;
    Input->FKey.KeyCode = KEY_F;
    Input->GKey.KeyCode = KEY_G;
    Input->HKey.KeyCode = KEY_H;
    Input->IKey.KeyCode = KEY_I;
    Input->JKey.KeyCode = KEY_J;
    Input->KKey.KeyCode = KEY_K;
    Input->LKey.KeyCode = KEY_L;
    Input->MKey.KeyCode = KEY_M;
    Input->NKey.KeyCode = KEY_N;
    Input->OKey.KeyCode = KEY_O;
    Input->PKey.KeyCode = KEY_P;
    Input->QKey.KeyCode = KEY_Q;
    Input->RKey.KeyCode = KEY_R;
    Input->SKey.KeyCode = KEY_S;
    Input->TKey.KeyCode = KEY_T;
    Input->UKey.KeyCode = KEY_U;
    Input->VKey.KeyCode = KEY_V;
    Input->WKey.KeyCode = KEY_W;
    Input->XKey.KeyCode = KEY_X;
    Input->YKey.KeyCode = KEY_Y;
    Input->ZKey.KeyCode = KEY_Z;
    //Input->Key.KeyCode = KEY_;
    Input->Number0Key.KeyCode = KEY_ZERO;
    Input->Number1Key.KeyCode = KEY_ONE;
    Input->Number2Key.KeyCode = KEY_TWO;
    Input->Number3Key.KeyCode = KEY_THREE;
    Input->Number4Key.KeyCode = KEY_FOUR;
    Input->Number5Key.KeyCode = KEY_FIVE;
    Input->Number6Key.KeyCode = KEY_SIX;
    Input->Number7Key.KeyCode = KEY_SEVEN;
    Input->Number8Key.KeyCode = KEY_EIGHT;
    Input->Number9Key.KeyCode = KEY_NINE;
    //Input->Key.KeyCode = KEY_;
    Input->Grave_Key.KeyCode = KEY_GRAVE;
    Input->Minus_Key.KeyCode = KEY_MINUS;
    Input->Equals_Key.KeyCode = KEY_EQUAL;
    Input->LeftBracket_Key.KeyCode = KEY_LEFT_BRACKET;
    Input->RightBracket_Key.KeyCode = KEY_RIGHT_BRACKET;
    Input->Backslash_Key.KeyCode = KEY_BACKSLASH;
    Input->Semicolon_Key.KeyCode = KEY_SEMICOLON;
    Input->Quote_Key.KeyCode = KEY_APOSTROPHE;
    Input->Slash_Key.KeyCode = KEY_SLASH;
    Input->Comma_Key.KeyCode = KEY_COMMA;
    Input->Period_Key.KeyCode = KEY_PERIOD;
    //Input->Key.KeyCode = KEY_;
    Input->Space_Key.KeyCode = KEY_SPACE;
    Input->Backspace_Key.KeyCode = KEY_BACKSPACE;
    Input->Delete_Key.KeyCode = KEY_DELETE;
    Input->Tab_Key.KeyCode = KEY_TAB;
    Input->Return_Key.KeyCode = KEY_ENTER;
    Input->CapsLock_Key.KeyCode = KEY_CAPS_LOCK;
    Input->Escape_Key.KeyCode = KEY_ESCAPE;
    //Input->Key.KeyCode = KEY_;
}

void
UpdateKeyInput(input *Input)
{
    for(int i = 0; i < sizeof(Input->KeyData) / sizeof(key_data); i++)
    {
        key_data *Key = &Input->KeyData[i];
        
        if(IsKeyDown(Key->KeyCode))
        {
            Key->IsDown = true;
            if(Key->PressTime == 0)
                Key->JustPressed = true;
            else
                Key->JustPressed = false;
            
            Key->PressTime += GetFrameTime();
        }
        else
        {
            Key->JustPressed = false;
            Key->PressTime = 0;
            Key->TimeTillNextRepeat = 0;
        }
        
        if(Key->JustPressed || Key->PressTime >= Input->KeyFirstRepeatTime)
        {
            if(Key->TimeTillNextRepeat <= 0)
            {
                Key->TimeTillNextRepeat = Input->KeyRepeatSpeed;
            }
            else
            {
                Key->TimeTillNextRepeat -= GetFrameTime();
            }
        }
    }
}




#endif //CCC_INPUT_H

#endif // def RAYLIB_H