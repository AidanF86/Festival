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
int 
LineLength(view *View, int l)
{
    return LineLength(View->Buffer, l);
}
int
LineCount(buffer *Buffer)
{
    return Buffer->Lines.Count;
}
int
LineCount(view *View)
{
    if(!View->Buffer)
        return 0;
    return View->Buffer->Lines.Count;
}

rect
CharRectAt(view *View, int l, int c)
{
    if(c == LineLength(View, l))
        return View->LineDataList[l].EndLineRect;
    return View->LineDataList[l].CharRects[c];
}

rect CharRectAt(view *View, buffer_pos Pos) { return CharRectAt(View, Pos.l, Pos.c); }

rect
LineRect(view *View, int l)
{
    if(l < 0 || l >= LineCount(View))
        return {0};
    return View->LineDataList.Data[l].LineRect;
}

v2
CharToScreenSpace(view *View, v2 CharRect)
{
    v2 Result = CharRect;
    Result.x += View->TextRect.x;
    Result.y += View->TextRect.y;
    Result.y -= View->Y;
    return Result;
}
v2
ScreenToCharSpace(view *View, v2 ScreenRect)
{
    v2 Result = ScreenRect;
    Result.y += View->Y;
    Result.y -= View->TextRect.y;
    Result.x -= View->TextRect.x;
    return Result;
}
rect
CharToScreenSpace(view *View, rect CharRect)
{
    rect Result = CharRect;
    v2 V = CharToScreenSpace(View, V2(CharRect));
    Result.x = V.x;
    Result.y = V.y;
    return Result;
}
rect
ScreenToCharSpace(view *View, rect ScreenRect)
{
    rect Result = ScreenRect;
    v2 V = ScreenToCharSpace(View, V2(ScreenRect));
    Result.x = V.x;
    Result.y = V.y;
    return Result;
}

rect
ScreenRectAt(view *View, int l, int c)
{
    return CharToScreenSpace(View, CharRectAt(View, l, c));
}

rect
ScreenRectAt(view *View, buffer_pos Pos)
{
    return ScreenRectAt(View, Pos.l, Pos.c);
}

char
CharAt(buffer *Buffer, int l, int c)
{
    if(l < 0 || l >= Buffer->Lines.Count ||
       c < 0 || c >= LineLength(Buffer, l))
        return 0;
    return Buffer->Lines[l].Data[c];
}
char
CharAt(view *View, int l, int c)
{
    if(!View->Buffer)
        return 0;
    return CharAt(View->Buffer, l, c);
}

void
InsertLine(buffer *Buffer, int l, string S)
{
    ListInsert(&Buffer->Lines, l, S);
}

line_data
LineDataAt(view *View, int l)
{
    if(l < 0 || l >= LineCount(View))
    {
        printf("GetLineData: Out of bounds!\n");
        return {0};
    }
    return View->LineDataList[l];
}

int
YToLine(view *View, int Y)
{
    int l;
    int PrevLineY = LineDataAt(View, 0).LineRect.y;
    for(l = 0; l < LineCount(View); l++)
    {
        line_data LineData = LineDataAt(View, l);
        int LineY = LineData.LineRect.y;
        
        if(LineY > Y)
        {
            if(l > 0)
                l--;
            break;
        }
    }
    
    if(l >= LineCount(View))
        return LineCount(View)-1;
    
    return l;
}

int
ColAt(program_state *ProgramState, view *View, buffer_pos P)
{
    int Col = 0;
    int PrevY = CharRectAt(View, BufferPos(P.l, 0)).y;
    
    for(int c = 1; c < LineLength(View, P.l) && c <= P.c; c++)
    {
        Col++;
        if(CharRectAt(View, BufferPos(P.l, c)).y > PrevY)
        {
            Col = ProgramState->SubLineOffset;
        }
        PrevY = CharRectAt(View, BufferPos(P.l, c)).y;
    }
    
    return Col;
}


buffer_pos
ClosestBufferPos(view *View, v2 P)
{ // P is in char space
    int l = YToLine(View, P.y);
    
    buffer_pos ClosestBufferP = BufferPos(l, 0);
    rect ClosestRect = CharRectAt(View, ClosestBufferP);
    v2 ClosestP = V2(ClosestRect.x+ClosestRect.w/2, ClosestRect.y+ClosestRect.h/2);
    
    for(int c = 0; c <= View->LineDataList[l].CharRects.Count; c++)
    {
        rect TestRect = CharRectAt(View, l, c);
        v2 TestP = V2(TestRect.x+TestRect.w/2, TestRect.y+TestRect.h/2);
        
        v2 Diff = TestP - P;
        v2 CompareDiff = ClosestP - P;
        if(abs(Diff.y) < abs(CompareDiff.y) ||
           ( !(abs(Diff.y) > abs(CompareDiff.y)) && abs(Diff.x) < abs(CompareDiff.x) )
           )
        {
            ClosestP = TestP;
            ClosestBufferP = BufferPos(l, c);
        }
    }
    
    return ClosestBufferP;
}

buffer_pos
ClosestBufferPos(view *View, rect Rect)
{
    return ClosestBufferPos(View, V2(Rect.x+Rect.w/2, Rect.y+Rect.h/2));
}

void
AdjustView(program_state *ProgramState, view *View)
{
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    buffer_pos CursorPos = View->CursorPos;
    int Y = View->Y;
    int TargetY = View->TargetY;
    
    rect CursorRect = CharRectAt(View, CursorPos.l, CursorPos.c);
    b32 MovedCursorUpOrDown = false;
    
    if(ProgramState->UserMovedCursor)
    { // Adjust based on cursor
        if(CursorRect.y < Y)
        {
            TargetY = CursorRect.y;
        }
        else if(CursorRect.y > Y + View->TextRect.h - CharHeight)
        {
            TargetY = CursorRect.y - View->TextRect.h + CharHeight;
        }
    }
    else
    { // Adjust based on view
        if(CursorRect.y < TargetY)
        {
            // adjust cursor pos to new rect?
            View->CursorPos.l = YToLine(View, TargetY) + 3;
            MovedCursorUpOrDown = true;
        }
        else if(CursorRect.y > TargetY + View->TextRect.h - CharHeight)
        {
            View->CursorPos.l = YToLine(View, 
                                        TargetY + View->TextRect.h) - 4;
            MovedCursorUpOrDown = true;
        }
    }
    
    View->TargetY = TargetY;
    
    Clamp(View->TargetY, 0, LineDataAt(View, LineCount(View)-1).EndLineRect.y);
    Clamp(View->CursorPos.l, 0, LineCount(View)-1);
    Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
    
    
    if(MovedCursorUpOrDown && ColAt(ProgramState, View, View->CursorPos) < View->IdealCursorCol)
    {
        int Diff = View->IdealCursorCol - ColAt(ProgramState, View, View->CursorPos);
        int DistToEnd = LineLength(View, View->CursorPos.l) - View->CursorPos.c;
        if(Diff > DistToEnd)
            Diff = DistToEnd;
        View->CursorPos.c += Diff;
    }
}

void
FillLineData(view *View, program_state *ProgramState)
{
    line_data_list *DataList = &View->LineDataList;
    
    int MarginLeft = ProgramState->MarginLeft;
    int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
    int NumbersWidth = ProgramState->NumbersWidth;
    int SubLineOffset = ProgramState->SubLineOffset;
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    // TODO(cheryl): formalize char-exclusion-zone
    int WrapPoint = View->TextRect.w - CharWidth;
    
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
    
    for(int l = 0; l < LineCount(View); l++)
    {
        ListAdd(DataList, LineData());
        
        line_data *RectData = &(DataList->Data[l]);
        int x = 0;
        
        RectData->LineRect.x = x;
        RectData->LineRect.y = y;
        RectData->LineRect.w = View->TextRect.w;
        RectData->DisplayLines = 1;
        
        for(int c = 0; c < LineLength(View, l); c++)
        {
            // Rect is within the space of textrect
            // so when drawing, offset by textrect.x and textrect.y
            // as well as buffer viewpos
            if(x+CharWidth >= WrapPoint)
            {
                x = SubLineOffset*CharWidth;
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
DrawView(program_state *ProgramState, view *View)
{
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    
    
    DrawRectangle(View->Rect.x, View->Rect.y, 4*CharWidth, ProgramState->ScreenHeight, ProgramState->LineNumberBGColor);
    
    BeginShaderMode(ProgramState->ShaderSDF);
    
    
    // Draw cursor
    rect CursorDrawRect = CharToScreenSpace(View, View->CursorRect);
    if(View == ProgramState->SelectedView)
        DrawRectangleRec(R(CursorDrawRect), ProgramState->CursorBGColor);
    else
        DrawRectangleLinesEx(R(CursorDrawRect), 2, ProgramState->CursorBGColor);
    
    // Draw text
    for(int l = 0; l < LineCount(View); l++)
    {
        string NumberString = String("%d", l);
        DrawString(ProgramState, NumberString, V2(View->Rect.x, LineRect(View, l).y - View->Y), ProgramState->LineNumberFGColor);
        
        line_data LineData = LineDataAt(View, l);
        
        int LineY = LineData.LineRect.y;
        if(LineY - View->Y > View->TextRect.y + View->TextRect.h)
            break;
        
        for(int c = 0; c < LineData.CharRects.Count; c++)
        {
            rect Rect = ScreenRectAt(View, l, c);
            
            color CharColor = ProgramState->FGColor;
            if(View == ProgramState->SelectedView && BufferPos(l, c) == View->CursorPos)
            {
                CharColor = ProgramState->CursorFGColor;
            }
            
            DrawChar(ProgramState, CharAt(View, l, c), V2(Rect), CharColor);
        }
    }
    EndShaderMode();
    
    DrawRectangleLinesEx(R(View->Rect), 5, ORANGE);
    DrawRectangleLinesEx(R(View->TextRect), 2, BLUE);
}

extern "C"
{
    PROGRAM_UPDATE_AND_RENDER(ProgramUpdateAndRender)
    {
        program_state *ProgramState = (program_state *)Memory->Data;
        ProgramState->ScreenWidth = Memory->WindowWidth;
        ProgramState->ScreenHeight = Memory->WindowHeight;
        buffer *Buffers = ProgramState->Buffers;
        view *Views = ProgramState->Views;
        buffer *Buffer = &ProgramState->Buffers[0];
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
            
            
            view *View = ProgramState->SelectedView;
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
            
            
            
            ProgramState->ScreenHeight = Memory->WindowHeight;
            ProgramState->ScreenWidth = Memory->WindowWidth;
            
            for(int i = 0; i < 6; i++)
            {
                ProgramState->Views[i].Active = false;
            }
            
            ProgramState->Views[0] = {0};
            ProgramState->Views[0].Active = true;
            ProgramState->Views[0].Row = 0;
            ProgramState->Views[0].Col = 0;
            ProgramState->Views[0].Y = 0;
            ProgramState->Views[0].LineDataList = {0};
            ProgramState->Views[0].CursorPos = BufferPos(0, 0);
            ProgramState->Views[0].IdealCursorCol = 0;
            ProgramState->Views[0].Buffer = &Buffers[0];
            ProgramState->Views[0].Rect.h = ProgramState->ScreenWidth;
            
            ProgramState->Views[1] = {0};
            ProgramState->Views[1].Active = true;
            ProgramState->Views[1].Row = 0;
            ProgramState->Views[1].Col = 1;
            ProgramState->Views[1].Y = 0;
            ProgramState->Views[1].LineDataList = {0};
            ProgramState->Views[1].CursorPos = BufferPos(0, 0);
            ProgramState->Views[1].IdealCursorCol = 0;
            ProgramState->Views[1].Buffer = &Buffers[0];
            ProgramState->Views[1].Rect.h = ProgramState->ScreenHeight;
            
            ProgramState->SelectedView = &ProgramState->Views[0];
            
            for(int i = 0; i < 6; i++)
            {
                view *View = &ProgramState->Views[i];
                if(View->Active)
                    FillLineData(View, ProgramState);
            }
        }
        
        BeginDrawing();
        
        view *View = ProgramState->SelectedView;
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
        Clamp(View->TextRect.w, CharWidth, View->Rect.w - View->TextRect.x);
        
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
            int NewTargetY = View->TargetY;
            NewTargetY -= GetMouseWheelMove()*20;
            Clamp(NewTargetY, 0, LineDataAt(View, LineCount(View)-1).EndLineRect.y);
            
            buffer_pos NewCursorPos = View->CursorPos;
            
            if(KeyShouldExecute(ProgramState->LeftKey))
            {
                NewCursorPos.c--;
                View->IdealCursorCol = ColAt(ProgramState, View, NewCursorPos);
            }
            if(KeyShouldExecute(ProgramState->RightKey))
            {
                NewCursorPos.c++;
                View->IdealCursorCol = ColAt(ProgramState, View, NewCursorPos);
            }
            
            b32 MovedUpOrDown = false;
            if(KeyShouldExecute(ProgramState->UpKey))
            {
                rect UpRect = View->CursorTargetRect - V2(0, CharHeight);
                NewCursorPos = ClosestBufferPos(View, UpRect);
                MovedUpOrDown = true;
            }
            if(KeyShouldExecute(ProgramState->DownKey))
            {
                rect DownRect = View->CursorTargetRect + V2(0, CharHeight);
                NewCursorPos = ClosestBufferPos(View, DownRect);
                MovedUpOrDown = true;
            }
            
            
            Clamp(NewCursorPos.l, 0, LineCount(View)-1);
            Clamp(NewCursorPos.c, 0, LineLength(View, NewCursorPos.l));
            
            ProgramState->UserMovedView = false;
            ProgramState->UserMovedCursor = false;
            
            if(NewTargetY != View->TargetY)
            {
                ProgramState->UserMovedView = true;
            }
            if(NewCursorPos != View->CursorPos)
            {
                ProgramState->UserMovedCursor = true;
            }
            
            // NOTE: Col > IdealCursorChar shouldn't be possible
            if(MovedUpOrDown && ColAt(ProgramState, View, NewCursorPos) < View->IdealCursorCol && NewCursorPos.l != View->CursorPos.l)
            {
                int Diff = View->IdealCursorCol - ColAt(ProgramState, View, NewCursorPos);
                int DistToEnd = LineLength(View, NewCursorPos.l) - NewCursorPos.c;
                if(Diff > DistToEnd)
                    Diff = DistToEnd;
                NewCursorPos.c += Diff;
            }
            
            View->TargetY = NewTargetY;
            View->CursorPos = NewCursorPos;
        }
        
        if(IsMouseButtonDown(0))
        {
            v2 MousePos = V2(GetMousePosition());
            
            buffer_pos MouseBufferPos = ClosestBufferPos(View, ScreenToCharSpace(View, MousePos));
            View->CursorPos = MouseBufferPos;
            View->IdealCursorCol = View->CursorPos.c;
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
                
                Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, CharToAdd);
                View->CursorPos.c++;
                View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
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
                
                Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, CharToAdd);
                View->CursorPos.c++;
                View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
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
                
                Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, CharToAdd);
                View->CursorPos.c++;
                View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
            }
        }
        
        for(int i = 0; i < 7; i++)
        {
            if(KeyShouldExecute(ProgramState->SpecialKeys[i]))
            {
                
                char CharToAdd = ' ';
                if(i == 0)
                {// Space
                    Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, ' ');
                    View->CursorPos.c++;
                    View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
                }
                else if(i == 1)
                { // Backspace
                    if(View->CursorPos.c > 0)
                    {
                        Buffer->Lines[View->CursorPos.l].RemoveChar(View->CursorPos.c-1);
                        View->CursorPos.c--;
                        Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
                    }
                }
                else if(i == 2)
                { // Delete
                    Buffer->Lines[View->CursorPos.l].RemoveChar(View->CursorPos.c);
                }
                else if(i == 3)
                { // Tab
                    // Switch views
                    int row = View->Row;
                    int col = View->Col;
                    view *NextView = View;
                    for(int i = 0; i < 6; i++)
                    {
                        view *TestView = &ProgramState->Views[i];
                        if(TestView != View && TestView->Active)
                        {
                            if((TestView->Col == View->Col && TestView->Row > View->Row && TestView->Row < NextView->Row) ||
                               (TestView->Col != NextView->Col) ||
                               (TestView->Col == NextView->Col && TestView->Row < NextView->Row))
                            {
                                NextView = TestView;
                            }
                        }
                    }
                    ProgramState->SelectedView = NextView;
                    View = ProgramState->SelectedView;
#if 0
                    for(int a = 0; a < 4; a++)
                    {
                        Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, ' ');
                        View->CursorPos.c++;
                        View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
                    }
#endif
                }
                else if(i == 4)
                { // Return
                    InsertLine(Buffer, View->CursorPos.l+1, CopyString(Buffer->Lines[View->CursorPos.l]));
                    Buffer->Lines[View->CursorPos.l].Slice(0, View->CursorPos.c);
                    Buffer->Lines[View->CursorPos.l+1].Slice(View->CursorPos.c, Buffer->Lines[View->CursorPos.l+1].Length);
                    View->CursorPos.l++;
                    View->CursorPos.c = 0;
                    
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
        
        
        
        
        
        Clamp(View->CursorPos.l, 0, LineCount(View)-1);
        Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
        
        //ComputeViewRects(ProgramState, Views);
        //int MarginLeft = ProgramState->MarginLeft;
        //int NumbersWidth = ProgramState->NumbersWidth;
        //v2 CharDim = GetCharDim(ProgramState);
        //int CharWidth = CharDim.x;
        //int CharHeight = CharDim.y;
        
        
        ProgramState->LeftW = ProgramState->ScreenWidth/2;
        int LeftViews = 0;
        int RightViews = 0;
        for(int i = 0; i < 6; i++)
        {
            if(Views[i].Active)
            {
                if(Views[i].Col == 0)
                    LeftViews++;
                else
                    RightViews++;
            }
        }
        
        if(LeftViews + RightViews == 1)
        { // Just one
            view *View;
            for(int i = 0; i < 6; i++)
            {
                if(Views[i].Active)
                    View = &Views[i];
            }
            
            View->Rect = Rect(0, 0, ProgramState->ScreenWidth, ProgramState->ScreenHeight);
            int x = View->Rect.x + NumbersWidth*CharWidth+MarginLeft;
            int y = View->Rect.y;
            View->TextRect = Rect(x, y, View->Rect.w - x, View->Rect.h - y);
        }
        else if(LeftViews > 0 && RightViews > 0)
        { // >=1 on both sides, Complex case
            
            // position left side
            {
                int ViewIndices[3];
                int ViewIndicesIndex = 0;
                for(int a = 0; a < LeftViews; a++)
                {
                    for(int i = 0; i < 6; i++)
                    {
                        if(Views[i].Active && Views[i].Col == 0 && Views[i].Row == a)
                        {
                            ViewIndices[ViewIndicesIndex] = i;
                            ViewIndicesIndex++;
                        }
                    }
                }
                
                int y = 0;
                for(int i = 0; i < LeftViews; i++)
                {
                    view *View = &ProgramState->Views[ViewIndices[i]];
                    View->Rect.y = y;
                    View->Rect.w = ProgramState->LeftW;
                    if(i == LeftViews-1)
                        View->Rect.h = ProgramState->ScreenHeight - y;
                    y += View->Rect.h;
                }
            }
            
            
            // position right side
            {
                int ViewIndices[3];
                int ViewIndicesIndex = 0;
                for(int a = 0; a < RightViews; a++)
                {
                    for(int i = 0; i < 6; i++)
                    {
                        if(Views[i].Active && Views[i].Col == 1 && Views[i].Row == a)
                        {
                            ViewIndices[ViewIndicesIndex] = i;
                            ViewIndicesIndex++;
                        }
                    }
                }
                
                int y = 0;
                for(int i = 0; i < RightViews; i++)
                {
                    view *View = &ProgramState->Views[ViewIndices[i]];
                    View->Rect.y = y;
                    View->Rect.x = ProgramState->LeftW;
                    View->Rect.w = ProgramState->ScreenWidth - ProgramState->LeftW;
                    if(i == RightViews-1)
                        View->Rect.h = ProgramState->ScreenHeight - y;
                    y += View->Rect.h;
                }
            }
        }
        else if(LeftViews+RightViews)
        { // All on one side
        }
        
        //ComputeViewRects(ProgramState);
        for(int i = 0; i < 6; i++)
        {
            view *View = &ProgramState->Views[i];
            View->TextRect.x = View->Rect.x + NumbersWidth*CharWidth + MarginLeft;
            View->TextRect.y = View->Rect.y;
            View->TextRect.w = View->Rect.w - (NumbersWidth*CharWidth + MarginLeft);
            View->TextRect.h = View->Rect.h;
        }
        
        AdjustView(ProgramState, View);
        
        
        View->CursorTargetRect = CharRectAt(View, View->CursorPos.l, View->CursorPos.c);
        View->CursorRect = Interpolate(View->CursorRect, View->CursorTargetRect, 0.5f);
        View->Y = Interpolate(View->Y, View->TargetY, 0.4f);
        
        ClearBackground(ProgramState->BGColor);
        
        for(int i = 0; i < 6; i++)
        {
            view *View = &ProgramState->Views[i];
            if(View->Active)
            {
                FillLineData(View, ProgramState);
                DrawView(ProgramState, View);
            }
        }
        //u64 BeforeTime = GetNanoseconds();
        //printf("Fill time: %lu\n", (GetNanoseconds() - BeforeTime) /1000000);
        
        
        EndDrawing();
    }
}