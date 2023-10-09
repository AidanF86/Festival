#include <stdio.h>

#include "raylib.h"

#include "festival_platform.h"

typedef Vector2 v2;
typedef Vector3 v3;
struct rect
{
    int x, y, w, h;
};
Rectangle
R(rect a)
{
    Rectangle Result = {(f32)a.x, (f32)a.y, (f32)a.w, (f32)a.h};
    return Result;
}

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
#if 0
    // TODO: narrow this down, set upper bounds on rect generation & text rendering
    if(l < Buffer->LineDataStart || l >= Buffer->LineDataStart + Buffer->LineDataList.Count ||
       c < 0 || c >= LineLength(Buffer, l))
        return {0};
    rect Result = Buffer->LineDataList.Data[l - Buffer->LineDataStart].CharRects.Data[c];
    if(Result.x == 0)
    {
        printf("%d, %d\n", l, c);
        printf("Effective: %d\n", l - Buffer->LineDataStart);
    }
    return Result;
#endif
    if(c == LineLength(Buffer, l))
        return Buffer->LineDataList.Data[l].EndLineRect;
    return Buffer->LineDataList.Data[l].CharRects.Data[c];
    
}

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
int
GetBufferPosCol(buffer_pos Pos)
{
}
#endif

int
PosToLine(program_state *ProgramState, buffer *Buffer, int Pos)
{
    v2 FontDim = MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0);
    int CharWidth = FontDim.x;
    int CharHeight = FontDim.y;
    
    int l;
    int PrevLinePos = GetLineData(Buffer, 0).LineRect.y;
    for(l = 1; l < Buffer->Lines.Count; l++)
    {
        line_data LineData = GetLineData(Buffer, l);
        int LinePos = LineData.LineRect.y;
        
        if(LinePos > Pos)
        {
            l;
            break;
        }
    }
    
    if(l >= Buffer->Lines.Count)
        return Buffer->Lines.Count-1;
    
    return l;
    
    //return {0};
}

void
AdjustView(program_state *ProgramState, buffer *Buffer)
{
    v2 FontDim = MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0);
    int CharWidth = FontDim.x;
    int CharHeight = FontDim.y;
    buffer_pos CursorPos = Buffer->CursorPos;
    int ViewPos = Buffer->ViewPos;
    
    rect CursorRect = RectAt(Buffer, CursorPos.l, CursorPos.c);
    //printf("cursor rect: %f, %f\n", CursorRect.x, CursorRect.y);
    
    if(ProgramState->UserMovedCursor)
    { // Adjust based on cursor
        if(CursorRect.y < ViewPos)
        {
            printf("Adjusting up from %d to %d\n", ViewPos, CursorRect.y);
            printf("cursor at %d, %d\n", CursorPos.l, CursorPos.c);
            printf("cursor rect: %f, %f\n", CursorRect.x, CursorRect.y);
            printf("%d\n", CursorPos.l);
            ViewPos = CursorRect.y;
        }
        else if(CursorRect.y > ViewPos + Buffer->TextRect.h - CharHeight)
        {
            ViewPos = CursorRect.y - Buffer->TextRect.h + CharHeight;
        }
    }
    else
    { // Adjust based on view
        if(CursorRect.y < ViewPos)
        {
            // adjust cursor pos to new rect?
            Buffer->CursorPos.l = PosToLine(ProgramState, Buffer, ViewPos) + 3;
        }
        else if(CursorRect.y > ViewPos + Buffer->TextRect.h - CharHeight)
        {
            Buffer->CursorPos.l = PosToLine(ProgramState, Buffer, 
                                            ViewPos + Buffer->TextRect.h) - 4;
        }
    }
    
    Buffer->ViewPos = ViewPos;
    
    // TODO: adjust either cursor or view depending on which the user moved
    
    Clamp(Buffer->ViewPos, 0, Buffer->LineDataList[Buffer->Lines.Count-1].EndLineRect.y);
    Clamp(Buffer->CursorPos.l, 0, Buffer->Lines.Count-1);
    Clamp(Buffer->CursorPos.c, 0, LineLength(Buffer, Buffer->CursorPos.l));
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
                x = Buffer->TextRect.x + SubLineOffset;
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
    
    
    //printf("%d\n", Buffer->LineDataList.Count);
    
    { // Draw cursor
        rect CursorRect;
        if(Buffer->CursorPos.c < LineLength(Buffer, Buffer->CursorPos.l))
        { // use a char rect
            CursorRect = RectAt(Buffer, Buffer->CursorPos.l, Buffer->CursorPos.c);
            CursorRect.y -= Buffer->ViewPos;
        }
        else
        { // use end-of-line rect
            CursorRect = GetLineData(Buffer, Buffer->CursorPos.l).EndLineRect;
            CursorRect.y -= Buffer->ViewPos;
        }
        DrawRectangleRec(R(CursorRect), RED);
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
            
            //DrawRectangleRec(Rect, GRAY);
            DrawTextEx(ProgramState->FontSDF, CharBuffer, V2(Rect.x, Rect.y), FontSize, 0, BLACK);
        }
    }
    EndShaderMode();
    
    
    DrawRectangleLinesEx(R(Buffer->TextRect), 3, PURPLE);
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
                
                printf("Adding line %d, size %d\n", Buffer->Lines.Count, i-LineStart);
                ListAdd(&(Buffer->Lines), AllocString(i-LineStart));
                
                int InLine = 0;
                for(int a = LineStart; a < i; a++)
                {
                    printf("char #%d\n", a-LineStart);
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
            
            
            ProgramState->FontSize = 22;
            ProgramState->CharsPerVirtualLine = 10;
            ProgramState->SubLineOffset = 40;
            ProgramState->MarginLeft = 10;
            ProgramState->NumbersWidth = 4;
            Buffer->ViewPos = 0;
            Buffer->LineDataList = {0};
            Buffer->CursorPos = BufferPos(0, 0);
            
            
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
        
        if(!(IsAnyControlKeyDown))
        {
            // TODO: mouse scrolling is kind of stuttery, especially up
            int NewViewPos = Buffer->ViewPos;
            buffer_pos NewCursorPos = Buffer->CursorPos;
            
            NewViewPos -= GetMouseWheelMove()*20;
            
            Clamp(NewViewPos, 0, Buffer->LineDataList[Buffer->Lines.Count-1].EndLineRect.y);
            printf("%d\n", Buffer->LineDataList[Buffer->Lines.Count-1].EndLineRect.y);
            printf("Pos: %d\n", NewViewPos);
            
            //NewCursorPos.l += IsKeyPressed(KEY_DOWN) - IsKeyPressed(KEY_UP);
            //NewCursorPos.c += IsKeyPressed(KEY_RIGHT) - IsKeyPressed(KEY_LEFT);
            NewCursorPos.l += IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
            NewCursorPos.c += IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
            
            Clamp(NewCursorPos.l, 0, Buffer->Lines.Count-1);
            Clamp(NewCursorPos.c, 0, LineLength(Buffer, NewCursorPos.l));
            
            ProgramState->UserMovedView = false;
            ProgramState->UserMovedCursor = false;
            
            if(NewViewPos != Buffer->ViewPos)
            {
                ProgramState->UserMovedView = true;
            }
            if(NewCursorPos != Buffer->CursorPos)
            {
                ProgramState->UserMovedCursor = true;
            }
            
            Buffer->ViewPos = NewViewPos;
            Buffer->CursorPos = NewCursorPos;
        }
        
        ComputeBufferRects(ProgramState, Buffer);
        AdjustView(ProgramState, Buffer);
        FillLineData(Buffer, ProgramState);
        
        
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