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
        return Buffer->LineDataList.Data[l].EndLineRect;
    return Buffer->LineDataList.Data[l].CharRects.Data[c];
    
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
    int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
    int SubLineOffset = ProgramState->SubLineOffset;
    int MarginLeft = ProgramState->MarginLeft;
    int NumbersWidth = ProgramState->NumbersWidth;
    v2 FontDim = MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0);
    int CharWidth = FontDim.x;
    int CharHeight = FontDim.y;
    
    Buffer->Rect = Rect(0, 0, ProgramState->ScreenWidth, ProgramState->ScreenHeight);
    f32 x = NumbersWidth*CharWidth + MarginLeft;
    f32 y = 0;
    Buffer->TextRect = Rect(x, y,
                            Buffer->Rect.w - x, Buffer->Rect.h - y);
}

#if 0
buffer_pos
ClosestBufferPos(buffer *Buffer, rect Rect)
{
    buffer_pos ClosestPos = Buffer->CursorPos;
    rect ClosestRect = RectAt(Buffer, Buffer->CursorPos);
    
    for(int l = Buffer->CursorPos.l; l >= Buffer->CursorPos.l - 1; l--)
    {
        for(int c = 0; c < Buffer->Lines[l].Length; c++)
        {
            rect TestRect = RectAt(Buffer, l, c);
            rect Diff = TestRect - Rect;
            rect CompareDiff = ClosestRect - Rect;
            if(abs(Diff.y) < CompareDiff.y ||
               (
                !(abs(Diff.y) > abs(CompareDiff.y)) && abs(Diff.x) < abs(CompareDiff.x)
                ))
            {
                ClosestRect = TestRect;
                ClosestPos = BufferPos(l, c);
            }
        }
    }
    return ClosestPos;
}
#endif

int
PosToLine(buffer *Buffer, int Pos)
{
#if 0
    v2 FontDim = MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0);
    int CharWidth = FontDim.x;
    int CharHeight = FontDim.y;
#endif
    
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
    
    //return {0};
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
ClosestBufferPos(buffer *Buffer, rect Rect)
{
    int l = PosToLine(Buffer, Rect.y);
    
    buffer_pos ClosestPos = BufferPos(l, 0);
    rect ClosestRect = RectAt(Buffer, Buffer->CursorPos);
    
    for(int c = 0; c < Buffer->Lines[l].Length; c++)
    {
        rect TestRect = RectAt(Buffer, l, c);
        rect Diff = TestRect - Rect;
        rect CompareDiff = ClosestRect - Rect;
        if(abs(Diff.y) < abs(CompareDiff.y) ||
           (
            !(abs(Diff.y) > abs(CompareDiff.y)) && abs(Diff.x) < abs(CompareDiff.x)
            ))
        {
            ClosestRect = TestRect;
            ClosestPos = BufferPos(l, c);
        }
    }
    return ClosestPos;
}

void
AdjustView(program_state *ProgramState, buffer *Buffer)
{
    v2 FontDim = MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0);
    int CharWidth = FontDim.x;
    int CharHeight = FontDim.y;
    buffer_pos CursorPos = Buffer->CursorPos;
    int ViewPos = Buffer->ViewPos;
    int TargetViewPos = Buffer->TargetViewPos;
    
    rect CursorRect = RectAt(Buffer, CursorPos.l, CursorPos.c);
    //printf("cursor rect: %f, %f\n", CursorRect.x, CursorRect.y);
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
    v2 FontDim = MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0);
    int CharWidth = FontDim.x;
    int CharHeight = FontDim.y;
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
            
            //DrawRectangleLinesEx(Rect(x, y, CharWidth, CharHeight), 1, ORANGE);
            
            ListAdd(&(RectData->CharRects), Rect(x, y, CharWidth, CharHeight));
            //ListAdd(&(RectData->CharRects), Rect(0, 0, CharWidth, CharHeight));
            
            x += CharWidth;
        }
        RectData->EndLineRect = Rect(x, y, CharWidth, CharHeight);
        
        y += CharHeight;
        
        RectData->LineRect.h = RectData->DisplayLines * CharHeight;
    }
}



void
DrawBuffer(program_state *ProgramState, buffer *Buffer)
{
    Font *FontMain = &ProgramState->FontMain;
    int FontSize = ProgramState->FontSize;
    v2 CharDim = MeasureTextEx(*FontMain, "_", FontSize, 0);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    
    
    DrawRectangle(0, 0, 4*CharWidth, ProgramState->ScreenHeight, LIGHTGRAY);
    
    BeginShaderMode(ProgramState->ShaderSDF);
    
    
    { // Draw cursor
        DrawRectangleRec(R(Buffer->CursorRect - V2(0, Buffer->ViewPos)), RED);
    }
    
    // Draw text
    for(int l = 0; l < Buffer->LineDataList.Count; l++)
    {
        char NumberBuffer[6];
        sprintf(NumberBuffer, "%d", l);
        
        DrawTextEx(ProgramState->FontSDF, NumberBuffer, V2(0, LineRect(Buffer, l).y-Buffer->ViewPos), FontSize, 0, GRAY);
        
        line_data LineData = GetLineData(Buffer, l);
        
        int LineY = LineData.LineRect.y;
        if(LineY - Buffer->ViewPos > Buffer->TextRect.y + Buffer->TextRect.h) break;
        
        //DrawRectangleLinesEx(LineData.EndLineRect, 1, ORANGE);
        
        for(int c = 0; c < LineData.CharRects.Count; c++)
        {
            char CharBuffer[2] = {CharAt(Buffer, l, c), 0};
            if(CharBuffer[0] == 0 && l == 9) CharBuffer[0] = '|';
            
            rect Rect = RectAt(Buffer, l, c);
            Rect.y -= Buffer->ViewPos;
            
            //DrawRectangleLinesEx(R(Rect), 1, GRAY);
            DrawTextEx(ProgramState->FontSDF, CharBuffer, V2(Rect.x, Rect.y), FontSize, 0, BLACK);
        }
    }
    EndShaderMode();
    
    
    //DrawRectangleLinesEx(R(Buffer->TextRect), 3, PURPLE);
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
            
            ProgramState->FontSize = 22;
            ProgramState->CharsPerVirtualLine = 10;
            ProgramState->SubLineOffset = 4;
            ProgramState->MarginLeft = 10;
            ProgramState->NumbersWidth = 4;
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
        
        v2 CharDim = MeasureTextEx(*FontMain, "_", ProgramState->FontSize, 0);
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
#if 1
            if(MovedUpOrDown && ColAt(ProgramState, Buffer, NewCursorPos) < Buffer->IdealCursorCol)
            {
                int Diff = Buffer->IdealCursorCol - ColAt(ProgramState, Buffer, NewCursorPos);
                int DistToEnd = Buffer->Lines[NewCursorPos.l].Length - NewCursorPos.c;
                if(Diff > DistToEnd)
                    Diff = DistToEnd;
                NewCursorPos.c += Diff;
            }
#endif
            
            
            
            Buffer->TargetViewPos = NewTargetViewPos;
            Buffer->CursorPos = NewCursorPos;
        }
        
        //printf("%d\n", ColAt(ProgramState, Buffer, Buffer->CursorPos));
        
        for(int i = 4; i < 30; i++)
        {
            if(KeyShouldExecute(ProgramState->KeyData[i]))
            {
                char CharToAdd;
                if(IsAnyShiftKeyDown)
                    CharToAdd = (i-4) + 65;
                else
                    CharToAdd = (i-4) + 97;
                
                Buffer->Lines[Buffer->CursorPos.l].InsertChar(Buffer->CursorPos.c, CharToAdd);
                Buffer->CursorPos.c++;
                Buffer->IdealCursorCol = ColAt(ProgramState, Buffer, Buffer->CursorPos);
            }
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
        ClearBackground(RAYWHITE);
        
        DrawBuffer(ProgramState, Buffer);
        //u64 BeforeTime = GetNanoseconds();
        //printf("Fill time: %lu\n", (GetNanoseconds() - BeforeTime) /1000000);
        
        
#if 1
#endif
        
        EndDrawing();
    }
}