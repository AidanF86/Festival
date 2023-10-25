#include <stdio.h>

#include "raylib.h"

#include "festival_platform.h"

#include "festival_math.h"
#include "festival_string.h"
#include "festival.h"
#include "festival_lists.h"


#include <time.h>
#define NanoToMilli (1.0/1000000.0)

u64
GetNanoseconds() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (u64)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

color
RGBA(int r, int g, int b, int a) {
    color Result;
    Result.r = (u8)r;
    Result.g = (u8)g;
    Result.b = (u8)b;
    Result.a = (u8)a;
    return Result;
}

color
RGB(int r, int g, int b) {
    return RGBA(r, g, b, 255);
}

v2 GetCharDim(program_state *ProgramState)
{
    return V2(MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0));
}

line_data
LineData() {
    line_data Result = {0};
    Result.CharRects = RectList();
    return Result;
};

int 
LineLength(buffer *Buffer, int l)
{
    if(l < 0 || l >= Buffer->Lines.Count)
        return 0;
    return Buffer->Lines.Data[l].Length;
}

rect
RectAt(buffer *Buffer, int l, int c)
{
    if(c == LineLength(Buffer, l))
        return Buffer->LineDataList[l].EndLineRect;
    return Buffer->LineDataList[l].CharRects[c];
    
}

rect RectAt(buffer *Buffer, buffer_pos Pos) { return RectAt(Buffer, Pos.l, Pos.c); }

rect
LineRect(buffer *Buffer, int l)
{
    // TODO: narrow this down, set upper bounds on rect generation & text rendering
    if(l < 0 || l >= Buffer->Lines.Count)
        return {0};
    return Buffer->LineDataList.Data[l].LineRect;
}

char
CharAt(buffer *Buffer, int l, int c)
{
    if(l < 0 || l >= Buffer->Lines.Count ||
       c < 0 || c >= LineLength(Buffer, l))
        return 0;
    return Buffer->Lines[l].Data[c];
}

void
InsertLine(buffer *Buffer, int l, string S)
{
    ListInsert(&Buffer->Lines, l, S);
}

line_data
GetLineData(buffer *Buffer, int l)
{
    if(l < 0 || l >= Buffer->Lines.Count)
    {
        printf("GetLineData: Out of bounds!\n");
        return {0};
    }
    return Buffer->LineDataList.Data[l];
}

void
ComputeBufferRects(program_state *ProgramState, buffer *Buffer)
{
    int MarginLeft = ProgramState->MarginLeft;
    int NumbersWidth = ProgramState->NumbersWidth;
    
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    
    Buffer->Rect = Rect(0, 0, ProgramState->ScreenWidth, ProgramState->ScreenHeight);
    f32 x = NumbersWidth*CharWidth + MarginLeft;
    f32 y = 0;
    Buffer->TextRect = Rect(x, y,
                            Buffer->Rect.w - x, Buffer->Rect.h - y);
}

int
PosToLine(buffer *Buffer, int Pos)
{
    int l;
    int PrevLinePos = GetLineData(Buffer, 0).LineRect.y;
    for(l = 0; l < Buffer->Lines.Count; l++)
    {
        line_data LineData = GetLineData(Buffer, l);
        int LinePos = LineData.LineRect.y;
        
        if(LinePos > Pos)
        {
            if(l > 0)
                l--;
            break;
        }
    }
    
    if(l >= Buffer->Lines.Count)
        return Buffer->Lines.Count-1;
    
    return l;
}

int
ColAt(program_state *ProgramState, buffer *Buffer, buffer_pos P)
{
    int Col = 0;
    int PrevY = RectAt(Buffer, BufferPos(P.l, 0)).y;
    
    for(int c = 1; c < Buffer->Lines[P.l].Length && c <= P.c; c++)
    {
        Col++;
        if(RectAt(Buffer, BufferPos(P.l, c)).y > PrevY)
        {
            Col = ProgramState->SubLineOffset;
        }
        PrevY = RectAt(Buffer, BufferPos(P.l, c)).y;
    }
    
    return Col;
}


buffer_pos
ClosestBufferPos(buffer *Buffer, v2 P)
{
    int l = PosToLine(Buffer, P.y);
    
    buffer_pos ClosestBufferPos = BufferPos(l, 0);
    rect ClosestRect = RectAt(Buffer, ClosestBufferPos);
    v2 ClosestP = V2(ClosestRect.x, ClosestRect.y);
    
    for(int c = 0; c < Buffer->Lines[l].Length; c++)
    {
        rect TestRect = RectAt(Buffer, l, c);
        v2 TestP = V2(TestRect.x, TestRect.y);
        
        v2 Diff = TestP - P;
        v2 CompareDiff = ClosestP - P;
        if(abs(Diff.y) < abs(CompareDiff.y) ||
           (
            !(abs(Diff.y) > abs(CompareDiff.y)) && abs(Diff.x) < abs(CompareDiff.x)
            ))
        {
            ClosestP = TestP;
            ClosestBufferPos = BufferPos(l, c);
        }
    }
    return ClosestBufferPos;
}

buffer_pos
ClosestBufferPos(buffer *Buffer, rect Rect)
{
    return ClosestBufferPos(Buffer, V2(Rect.x, Rect.y));
}

void
AdjustView(program_state *ProgramState, buffer *Buffer)
{
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    buffer_pos CursorPos = Buffer->CursorPos;
    int ViewPos = Buffer->ViewPos;
    int TargetViewPos = Buffer->TargetViewPos;
    
    rect CursorRect = RectAt(Buffer, CursorPos.l, CursorPos.c);
    b32 MovedCursorUpOrDown = false;
    
    if(ProgramState->UserMovedCursor)
    { // Adjust based on cursor
        if(CursorRect.y < ViewPos)
        {
            TargetViewPos = CursorRect.y;
        }
        else if(CursorRect.y > ViewPos + Buffer->TextRect.h - CharHeight)
        {
            TargetViewPos = CursorRect.y - Buffer->TextRect.h + CharHeight;
        }
    }
    else
    { // Adjust based on view
        if(CursorRect.y < TargetViewPos)
        {
            // adjust cursor pos to new rect?
            Buffer->CursorPos.l = PosToLine(Buffer, TargetViewPos) + 3;
            MovedCursorUpOrDown = true;
        }
        else if(CursorRect.y > TargetViewPos + Buffer->TextRect.h - CharHeight)
        {
            Buffer->CursorPos.l = PosToLine(Buffer, 
                                            TargetViewPos + Buffer->TextRect.h) - 4;
            MovedCursorUpOrDown = true;
        }
    }
    
    Buffer->TargetViewPos = TargetViewPos;
    
    Clamp(Buffer->TargetViewPos, 0, Buffer->LineDataList[Buffer->Lines.Count-1].EndLineRect.y);
    Clamp(Buffer->CursorPos.l, 0, Buffer->Lines.Count-1);
    Clamp(Buffer->CursorPos.c, 0, LineLength(Buffer, Buffer->CursorPos.l));
    
    
    if(MovedCursorUpOrDown && ColAt(ProgramState, Buffer, Buffer->CursorPos) < Buffer->IdealCursorCol)
    {
        int Diff = Buffer->IdealCursorCol - ColAt(ProgramState, Buffer, Buffer->CursorPos);
        int DistToEnd = Buffer->Lines[Buffer->CursorPos.l].Length - Buffer->CursorPos.c;
        if(Diff > DistToEnd)
            Diff = DistToEnd;
        Buffer->CursorPos.c += Diff;
    }
    
}

void
FillLineData(buffer *Buffer, program_state *ProgramState)
{
    line_data_list *DataList = &Buffer->LineDataList;
    
    int MarginLeft = ProgramState->MarginLeft;
    int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
    int NumbersWidth = ProgramState->NumbersWidth;
    int SubLineOffset = ProgramState->SubLineOffset;
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    // TODO(cheryl): formalize char-exclusion-zone
    int WrapPoint = Buffer->TextRect.x + Buffer->TextRect.w - CharWidth;
    
    // DeAllocation
    if(DataList->IsAllocated);
    {
        // Deallocate all lists
        for(int i = 0; i < DataList->Count; i++)
        {
            if(DataList->Data[i].CharRects.IsAllocated)
            {
                ListFree(&(DataList->Data[i].CharRects));
            }
            else
            {
                Print("Unallocated rect list???\n");
            }
            
        }
        ListFree(DataList);
    }
    // Allocation
    *DataList = LineDataList();
    
    
    int y = 0;
    
    for(int i = 0; i < Buffer->Lines.Count; i++)
    {
        ListAdd(DataList, LineData());
        
        line_data *RectData = &(DataList->Data[i]);
        string *Line = &Buffer->Lines.Data[i];
        int x = Buffer->TextRect.x;
        
        RectData->LineRect.x = x;
        RectData->LineRect.y = y;
        RectData->LineRect.w = Buffer->TextRect.w;
        RectData->DisplayLines = 1;
        
        for(int a = 0; a < Line->Length; a++)
        {
            if(x+CharWidth >= WrapPoint)
            {
                x = Buffer->TextRect.x + SubLineOffset*CharWidth;
                y += CharHeight;
                RectData->DisplayLines++;
            }
            
            ListAdd(&(RectData->CharRects), Rect(x, y, CharWidth, CharHeight));
            
            x += CharWidth;
        }
        RectData->EndLineRect = Rect(x, y, CharWidth, CharHeight);
        
        y += CharHeight;
        
        RectData->LineRect.h = RectData->DisplayLines * CharHeight;
    }
}


void
DrawChar(program_state *ProgramState, int Char, v2 Pos, color FGColor, color BGColor)
{
    v2 CharDim = GetCharDim(ProgramState);
    if(BGColor.a != 0)
    {
        DrawRectangleV(V(Pos), V(CharDim), BGColor);
    }
    char CharBuffer[2] = {(char)Char, 0};
    DrawTextEx(ProgramState->FontSDF, CharBuffer, V(Pos), ProgramState->FontSize, 0, FGColor);
}

void
DrawChar(program_state *ProgramState, int Char, v2 Pos, color FGColor)
{
    DrawChar(ProgramState, Char, Pos, FGColor, RGBA(0, 0, 0, 0));
}

void
DrawString(program_state *ProgramState, string String, v2 Pos, color FGColor, color BGColor)
{
    v2 CharDim = GetCharDim(ProgramState);
    for(int i = 0; i < String.Length; i++)
    {
        DrawChar(ProgramState, String[i], Pos, FGColor, BGColor);
        Pos.x += CharDim.x;
    }
}

void
DrawString(program_state *ProgramState, string String, v2 Pos, color FGColor)
{
    DrawString(ProgramState, String, Pos, FGColor, RGBA(0, 0, 0, 0));
}

void
DrawBuffer(program_state *ProgramState, buffer *Buffer)
{
    Font *FontMain = &ProgramState->FontMain;
    //int FontSize = ProgramState->FontSize;
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    
    
    DrawRectangle(0, 0, 4*CharWidth, ProgramState->ScreenHeight, ProgramState->LineNumberBGColor);
    
    BeginShaderMode(ProgramState->ShaderSDF);
    
    
    { // Draw cursor
        DrawRectangleRec(R(Buffer->CursorRect - V2(0, Buffer->ViewPos)), ProgramState->CursorBGColor);
    }
    
    // Draw text
    for(int l = 0; l < Buffer->LineDataList.Count; l++)
    {
        string NumberString = String("%d", l);
        DrawString(ProgramState, NumberString, V2(0, LineRect(Buffer, l).y-Buffer->ViewPos), ProgramState->LineNumberFGColor);
        
        line_data LineData = GetLineData(Buffer, l);
        
        int LineY = LineData.LineRect.y;
        if(LineY - Buffer->ViewPos > Buffer->TextRect.y + Buffer->TextRect.h) break;
        
        for(int c = 0; c < LineData.CharRects.Count; c++)
        {
            rect Rect = RectAt(Buffer, l, c);
            Rect.y -= Buffer->ViewPos;
            
            color CharColor = ProgramState->FGColor;
            if(BufferPos(l, c) == Buffer->CursorPos)
            {
                CharColor = ProgramState->CursorFGColor;
            }
            
            DrawChar(ProgramState, CharAt(Buffer, l, c), V2(Rect), CharColor);
        }
    }
    EndShaderMode();
    
    int ViewPos = Buffer->ViewPos;
}

extern "C"
{
    PROGRAM_UPDATE_AND_RENDER(ProgramUpdateAndRender)
    {
        program_state *ProgramState = (program_state *)Memory->Data;
        buffer *Buffers = ProgramState->Buffers;
        buffer *Buffer = Buffers;
        Font *FontMain = &ProgramState->FontMain;
        Font *FontSDF = &ProgramState->FontSDF;
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            
            for(int i = 0; i < MAX_BUFFERS; i++) {
                Buffers[i] = {0};
            };
            
            // TEXT FILE
            const char *Filename = "test.cpp";
            
            u32 FileSize = (u32)GetFileLength(Filename);
            char *FileData = LoadFileText(Filename);
            
            Buffers[0].Lines = StringList();
            printf("Gathering text\n");
            for(int i = 0; i < FileSize; i++)
            {
                int LineStart = i;
                // TODO: need to add AppendString to easily do this
                for(i; FileData[i] != '\n' && i < FileSize; i++)
                {
                }
                
                ListAdd(&(Buffer->Lines), AllocString(i-LineStart));
                
                int InLine = 0;
                for(int a = LineStart; a < i; a++)
                {
                    Buffer->Lines[Buffer->Lines.Count-1].Data[InLine] = FileData[a];
                    InLine++;
                }
            }
            printf("Finished with text\n");
            
            UnloadFileText(FileData);
            
            // FONT
            {
                u32 FontFileSize = 0;
                u8 *FontFileData = LoadFileData("LiberationMono-Regular.ttf", &FontFileSize);
                
                *FontMain = {0};
                FontMain->baseSize = 16;
                FontMain->glyphCount = 95;
                FontMain->glyphs = LoadFontData(FontFileData, FontFileSize, 16, 0, 95, FONT_DEFAULT);
                Image Atlas = GenImageFontAtlas(FontMain->glyphs, &FontMain->recs, 95, 16, 4, 0);
                FontMain->texture = LoadTextureFromImage(Atlas);
                UnloadImage(Atlas);
                
                *FontSDF = {0};
                FontSDF->baseSize = 32;
                FontSDF->glyphCount = 95;
                FontSDF->glyphs = LoadFontData(FontFileData, FontFileSize, 32, 0, 0, FONT_SDF);
                Atlas = GenImageFontAtlas(FontSDF->glyphs, &FontSDF->recs, 95, 32, 0, 1);
                FontSDF->texture = LoadTextureFromImage(Atlas);
                UnloadImage(Atlas);
                
                UnloadFileData(FontFileData);
                
                ProgramState->ShaderSDF = LoadShader(0, TextFormat("../data/shaders/glsl%i/sdf.fs", 330));
                SetTextureFilter(FontSDF->texture, TEXTURE_FILTER_BILINEAR);
            }
            
            
            ProgramState->KeyFirstRepeatTime = 0.4f;
            ProgramState->KeyRepeatSpeed = 0.02f;
            
            for(int i = 0; i < sizeof(ProgramState->KeyData) / sizeof(key_data); i++)
            {
                ProgramState->KeyData[i] = {0};
            }
            ProgramState->LeftKey.KeyCode = KEY_LEFT;
            ProgramState->RightKey.KeyCode = KEY_RIGHT;
            ProgramState->UpKey.KeyCode = KEY_UP;
            ProgramState->DownKey.KeyCode = KEY_DOWN;
            ProgramState->PageUp_Key.KeyCode = KEY_PAGE_UP;
            ProgramState->PageDown_Key.KeyCode = KEY_PAGE_DOWN;
            //ProgramState->Key.KeyCode = KEY_;
            ProgramState->AKey.KeyCode = KEY_A;
            ProgramState->BKey.KeyCode = KEY_B;
            ProgramState->CKey.KeyCode = KEY_C;
            ProgramState->DKey.KeyCode = KEY_D;
            ProgramState->EKey.KeyCode = KEY_E;
            ProgramState->FKey.KeyCode = KEY_F;
            ProgramState->GKey.KeyCode = KEY_G;
            ProgramState->HKey.KeyCode = KEY_H;
            ProgramState->IKey.KeyCode = KEY_I;
            ProgramState->JKey.KeyCode = KEY_J;
            ProgramState->KKey.KeyCode = KEY_K;
            ProgramState->LKey.KeyCode = KEY_L;
            ProgramState->MKey.KeyCode = KEY_M;
            ProgramState->NKey.KeyCode = KEY_N;
            ProgramState->OKey.KeyCode = KEY_O;
            ProgramState->PKey.KeyCode = KEY_P;
            ProgramState->QKey.KeyCode = KEY_Q;
            ProgramState->RKey.KeyCode = KEY_R;
            ProgramState->SKey.KeyCode = KEY_S;
            ProgramState->TKey.KeyCode = KEY_T;
            ProgramState->UKey.KeyCode = KEY_U;
            ProgramState->VKey.KeyCode = KEY_V;
            ProgramState->WKey.KeyCode = KEY_W;
            ProgramState->XKey.KeyCode = KEY_X;
            ProgramState->YKey.KeyCode = KEY_Y;
            ProgramState->ZKey.KeyCode = KEY_Z;
            //ProgramState->Key.KeyCode = KEY_;
            ProgramState->Number0Key.KeyCode = KEY_ZERO;
            ProgramState->Number1Key.KeyCode = KEY_ONE;
            ProgramState->Number2Key.KeyCode = KEY_TWO;
            ProgramState->Number3Key.KeyCode = KEY_THREE;
            ProgramState->Number4Key.KeyCode = KEY_FOUR;
            ProgramState->Number5Key.KeyCode = KEY_FIVE;
            ProgramState->Number6Key.KeyCode = KEY_SIX;
            ProgramState->Number7Key.KeyCode = KEY_SEVEN;
            ProgramState->Number8Key.KeyCode = KEY_EIGHT;
            ProgramState->Number9Key.KeyCode = KEY_NINE;
            //ProgramState->Key.KeyCode = KEY_;
            ProgramState->Grave_Key.KeyCode = KEY_GRAVE;
            ProgramState->Minus_Key.KeyCode = KEY_MINUS;
            ProgramState->Equals_Key.KeyCode = KEY_EQUAL;
            ProgramState->LeftBracket_Key.KeyCode = KEY_LEFT_BRACKET;
            ProgramState->RightBracket_Key.KeyCode = KEY_RIGHT_BRACKET;
            ProgramState->Backslash_Key.KeyCode = KEY_BACKSLASH;
            ProgramState->Semicolon_Key.KeyCode = KEY_SEMICOLON;
            ProgramState->Quote_Key.KeyCode = KEY_APOSTROPHE;
            ProgramState->Slash_Key.KeyCode = KEY_SLASH;
            ProgramState->Comma_Key.KeyCode = KEY_COMMA;
            ProgramState->Period_Key.KeyCode = KEY_PERIOD;
            //ProgramState->Key.KeyCode = KEY_;
            ProgramState->Space_Key.KeyCode = KEY_SPACE;
            ProgramState->Backspace_Key.KeyCode = KEY_BACKSPACE;
            ProgramState->Delete_Key.KeyCode = KEY_DELETE;
            ProgramState->Tab_Key.KeyCode = KEY_TAB;
            ProgramState->Return_Key.KeyCode = KEY_ENTER;
            ProgramState->CapsLock_Key.KeyCode = KEY_CAPS_LOCK;
            ProgramState->Escape_Key.KeyCode = KEY_ESCAPE;
            //ProgramState->Key.KeyCode = KEY_;
            
            ProgramState->FontSize = 22;
            ProgramState->CharsPerVirtualLine = 10;
            ProgramState->SubLineOffset = 4;
            ProgramState->MarginLeft = 10;
            ProgramState->NumbersWidth = 4;
            
            {// Colors
                ProgramState->BGColor = RGB(255, 255, 255);
                ProgramState->FGColor = RGB(0, 0, 0);
                ProgramState->LineNumberBGColor = LIGHTGRAY;
                ProgramState->LineNumberFGColor = GRAY;
                ProgramState->CursorBGColor = ProgramState->FGColor;
                ProgramState->CursorFGColor = ProgramState->BGColor;
            }
            
            Buffer->ViewPos = 0;
            Buffer->LineDataList = {0};
            Buffer->CursorPos = BufferPos(0, 0);
            Buffer->IdealCursorCol = 0;
            
            ProgramState->ScreenHeight = Memory->WindowHeight;
            ProgramState->ScreenWidth = Memory->WindowWidth;
            
            
            ComputeBufferRects(ProgramState, Buffer);
            FillLineData(Buffer, ProgramState);
        }
        
        
        ProgramState->ScreenWidth = Memory->WindowWidth;
        ProgramState->ScreenHeight = Memory->WindowHeight;
        
        int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
        int SubLineOffset = ProgramState->SubLineOffset;
        int MarginLeft = ProgramState->MarginLeft;
        int NumbersWidth = ProgramState->NumbersWidth;
        
        v2 CharDim = GetCharDim(ProgramState);
        int CharWidth = CharDim.x;
        int CharHeight = CharDim.y;
        
        if(WindowShouldClose())
        {
            Memory->IsRunning = false;
        }
        
        // TODO: this probably doesn't account for a rect with x>0
        Clamp(Buffer->TextRect.w, CharWidth, Buffer->Rect.w - Buffer->TextRect.x);
        
        if(IsAnyControlKeyDown)
        {
            ProgramState->FontSize += GetMouseWheelMove();
            if(ProgramState->FontSize < 6) ProgramState->FontSize = 6;
            if(ProgramState->FontSize > 100) ProgramState->FontSize = 100;
        }
        
        
        for(int i = 0; i < sizeof(ProgramState->KeyData) / sizeof(key_data); i++)
        {
            key_data *Key = &ProgramState->KeyData[i];
            if(IsKeyDown(Key->KeyCode))
            {
                if(Key->PressTime == 0)
                    Key->JustPressed = true;
                else
                    Key->JustPressed = false;
                
                Key->PressTime += GetFrameTime();
            }
            else
            {
                Key->PressTime = 0;
                Key->TimeTillNextRepeat = 0;
            }
            
            if(Key->JustPressed || Key->PressTime >= ProgramState->KeyFirstRepeatTime)
            {
                if(Key->TimeTillNextRepeat <= 0)
                {
                    Key->TimeTillNextRepeat = ProgramState->KeyRepeatSpeed;
                }
                else
                {
                    Key->TimeTillNextRepeat -= GetFrameTime();
                }
            }
            
        }
        
        
        if(!(IsAnyControlKeyDown))
        {
            // TODO: mouse scrolling is kind of stuttery, especially up
            int NewTargetViewPos = Buffer->TargetViewPos;
            NewTargetViewPos -= GetMouseWheelMove()*20;
            Clamp(NewTargetViewPos, 0, Buffer->LineDataList[Buffer->Lines.Count-1].EndLineRect.y);
            
            
            buffer_pos NewCursorPos = Buffer->CursorPos;
            
            if(KeyShouldExecute(ProgramState->LeftKey))
            {
                NewCursorPos.c--;
                Buffer->IdealCursorCol = ColAt(ProgramState, Buffer, NewCursorPos);
            }
            if(KeyShouldExecute(ProgramState->RightKey))
            {
                NewCursorPos.c++;
                Buffer->IdealCursorCol = ColAt(ProgramState, Buffer, NewCursorPos);
            }
            
            b32 MovedUpOrDown = false;
            if(KeyShouldExecute(ProgramState->UpKey))
            {
                rect UpRect = Buffer->CursorTargetRect - Rect(0, CharHeight, 0, 0);
                NewCursorPos = ClosestBufferPos(Buffer, UpRect);
                MovedUpOrDown = true;
            }
            if(KeyShouldExecute(ProgramState->DownKey))
            {
                rect DownRect = Buffer->CursorTargetRect + Rect(0, CharHeight, 0, 0);
                NewCursorPos = ClosestBufferPos(Buffer, DownRect);
                MovedUpOrDown = true;
            }
            
            Clamp(NewCursorPos.l, 0, Buffer->Lines.Count-1);
            Clamp(NewCursorPos.c, 0, LineLength(Buffer, NewCursorPos.l));
            
            Clamp(NewCursorPos.c, 0, LineLength(Buffer, NewCursorPos.l));
            
            
            ProgramState->UserMovedView = false;
            ProgramState->UserMovedCursor = false;
            
            if(NewTargetViewPos != Buffer->TargetViewPos)
            {
                ProgramState->UserMovedView = true;
            }
            if(NewCursorPos != Buffer->CursorPos)
            {
                ProgramState->UserMovedCursor = true;
            }
            
            
            // NOTE: Col > IdealCursorChar shouldn't be possible
            if(MovedUpOrDown && ColAt(ProgramState, Buffer, NewCursorPos) < Buffer->IdealCursorCol)
            {
                int Diff = Buffer->IdealCursorCol - ColAt(ProgramState, Buffer, NewCursorPos);
                int DistToEnd = Buffer->Lines[NewCursorPos.l].Length - NewCursorPos.c;
                if(Diff > DistToEnd)
                    Diff = DistToEnd;
                NewCursorPos.c += Diff;
            }
            
            Buffer->TargetViewPos = NewTargetViewPos;
            Buffer->CursorPos = NewCursorPos;
        }
        
        
        for(int i = 0; i < 26; i++)
        {
            if(KeyShouldExecute(ProgramState->LetterKeys[i]))
            {
                char CharToAdd;
                if(IsAnyShiftKeyDown)
                    CharToAdd = 'A' + i;
                else
                    CharToAdd = 'a' + i;
                
                Buffer->Lines[Buffer->CursorPos.l].InsertChar(Buffer->CursorPos.c, CharToAdd);
                Buffer->CursorPos.c++;
                Buffer->IdealCursorCol = ColAt(ProgramState, Buffer, Buffer->CursorPos);
            }
        }
        for(int i = 0; i < 10; i++)
        {
            if(KeyShouldExecute(ProgramState->NumberKeys[i]))
            {
                char CharToAdd;
                if(IsAnyShiftKeyDown)
                {
                    if(i == 1)
                        CharToAdd = '!';
                    else if(i == 2)
                        CharToAdd = '@';
                    else if(i == 3)
                        CharToAdd = '#';
                    else if(i == 4)
                        CharToAdd = '$';
                    else if(i == 5)
                        CharToAdd = '%';
                    else if(i == 6)
                        CharToAdd = '^';
                    else if(i == 7)
                        CharToAdd = '&';
                    else if(i == 8)
                        CharToAdd = '*';
                    else if(i == 9)
                        CharToAdd = '(';
                    else
                        CharToAdd = ')';
                    
                }
                else
                {
                    CharToAdd = '0' + i;
                }
                
                Buffer->Lines[Buffer->CursorPos.l].InsertChar(Buffer->CursorPos.c, CharToAdd);
                Buffer->CursorPos.c++;
                Buffer->IdealCursorCol = ColAt(ProgramState, Buffer, Buffer->CursorPos);
            }
        }
        for(int i = 0; i < 11; i++)
        {
            if(KeyShouldExecute(ProgramState->SymbolKeys[i]))
            {
                
                char CharToAdd = ' ';
                if(!IsAnyShiftKeyDown)
                {
                    if(i == 0)
                        CharToAdd = '`';
                    else if(i == 1)
                        CharToAdd = '-';
                    else if(i == 2)
                        CharToAdd = '=';
                    else if(i == 3)
                        CharToAdd = '[';
                    else if(i == 4)
                        CharToAdd = ']';
                    else if(i == 5)
                        CharToAdd = '\\';
                    else if(i == 6)
                        CharToAdd = ';';
                    else if(i == 7)
                        CharToAdd = '\'';
                    else if(i == 8)
                        CharToAdd = '/';
                    else if(i == 9)
                        CharToAdd = ',';
                    else if(i == 10)
                        CharToAdd = '.';
                }
                else
                {
                    if(i == 0)
                        CharToAdd = '~';
                    else if(i == 1)
                        CharToAdd = '_';
                    else if(i == 2)
                        CharToAdd = '+';
                    else if(i == 3)
                        CharToAdd = '{';
                    else if(i == 4)
                        CharToAdd = '}';
                    else if(i == 5)
                        CharToAdd = '|';
                    else if(i == 6)
                        CharToAdd = ':';
                    else if(i == 7)
                        CharToAdd = '"';
                    else if(i == 8)
                        CharToAdd = '?';
                    else if(i == 9)
                        CharToAdd = '<';
                    else if(i == 10)
                        CharToAdd = '>';
                }
                
                Buffer->Lines[Buffer->CursorPos.l].InsertChar(Buffer->CursorPos.c, CharToAdd);
                Buffer->CursorPos.c++;
                Buffer->IdealCursorCol = ColAt(ProgramState, Buffer, Buffer->CursorPos);
            }
        }
        
        for(int i = 0; i < 7; i++)
        {
            if(KeyShouldExecute(ProgramState->SpecialKeys[i]))
            {
                
                char CharToAdd = ' ';
                if(i == 0)
                {// Space
                    Buffer->Lines[Buffer->CursorPos.l].InsertChar(Buffer->CursorPos.c, ' ');
                    Buffer->CursorPos.c++;
                    Buffer->IdealCursorCol = ColAt(ProgramState, Buffer, Buffer->CursorPos);
                }
                else if(i == 1)
                { // Backspace
                    if(Buffer->CursorPos.c > 0)
                    {
                        Buffer->Lines[Buffer->CursorPos.l].RemoveChar(Buffer->CursorPos.c-1);
                        Buffer->CursorPos.c--;
                        Clamp(Buffer->CursorPos.c, 0, LineLength(Buffer, Buffer->CursorPos.l));
                    }
                }
                else if(i == 2)
                { // Delete
                    Buffer->Lines[Buffer->CursorPos.l].RemoveChar(Buffer->CursorPos.c);
                }
                else if(i == 3)
                { // Tab
                    for(int a = 0; a < 4; a++)
                    {
                        Buffer->Lines[Buffer->CursorPos.l].InsertChar(Buffer->CursorPos.c, ' ');
                        Buffer->CursorPos.c++;
                        Buffer->IdealCursorCol = ColAt(ProgramState, Buffer, Buffer->CursorPos);
                    }
                }
                else if(i == 4)
                { // Return
                    InsertLine(Buffer, Buffer->CursorPos.l+1, CopyString(Buffer->Lines[Buffer->CursorPos.l]));
                    Buffer->Lines[Buffer->CursorPos.l].Slice(0, Buffer->CursorPos.c);
                    Buffer->Lines[Buffer->CursorPos.l+1].Slice(Buffer->CursorPos.c, Buffer->Lines[Buffer->CursorPos.l+1].Length);
                    Buffer->CursorPos.l++;
                    Buffer->CursorPos.c = 0;
                    
                    ProgramState->UserMovedCursor = true;
                }
                else if(i == 5)
                { // Caps lock
                }
                else if(i == 6)
                { // Escape
                }
                
            }
        }
        
        
        if(IsMouseButtonDown(0))
        {
            v2 AdjustedMousePos = V2(GetMousePosition());
            AdjustedMousePos.y += Buffer->TextRect.y;
            AdjustedMousePos.y += Buffer->ViewPos;
            
            buffer_pos MouseBufferPos = ClosestBufferPos(Buffer, AdjustedMousePos);
            //printf("%d, %d\n", MouseBufferPos.l, MouseBufferPos.c);
            Buffer->CursorPos = MouseBufferPos;
            Buffer->IdealCursorCol = Buffer->CursorPos.c;
        }
        
        
        Clamp(Buffer->CursorPos.l, 0, Buffer->Lines.Count-1);
        Clamp(Buffer->CursorPos.c, 0, LineLength(Buffer, Buffer->CursorPos.l));
        
        ComputeBufferRects(ProgramState, Buffer);
        AdjustView(ProgramState, Buffer);
        FillLineData(Buffer, ProgramState);
        
        
        Buffer->CursorTargetRect = RectAt(Buffer, Buffer->CursorPos.l, Buffer->CursorPos.c);
        Buffer->CursorRect = Interpolate(Buffer->CursorRect, Buffer->CursorTargetRect, 0.5f);
        Buffer->ViewPos = Interpolate(Buffer->ViewPos, Buffer->TargetViewPos, 0.4f);
        
        BeginDrawing();
        ClearBackground(ProgramState->BGColor);
        //ClearBackground(RAYWHITE);
        
        DrawBuffer(ProgramState, Buffer);
        //u64 BeforeTime = GetNanoseconds();
        //printf("Fill time: %lu\n", (GetNanoseconds() - BeforeTime) /1000000);
        
        
#if 1
#endif
        
        EndDrawing();
    }
}