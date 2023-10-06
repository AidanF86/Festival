#include <stdio.h>

#include "raylib.h"

typedef Vector2 v2;
typedef Vector3 v3;
typedef Rectangle rect;

#include "festival_platform.h"
#include "festival_math.h"
#include "festival_string.h"
#include "festival.h"
#include "festival_lists.h"

line_rect_data
LineRectData() {
    line_rect_data Result = {0};
    Result.CharRects = RectList();
    return Result;
};

int 
CharsInLine(buffer *Buffer, int l)
{
    if(l < 0 || l >= Buffer->Lines.Count)
        return 0;
    return Buffer->Lines[l].Length;
}

rect
RectAt(buffer *Buffer, int l, int c)
{
    // TODO: narrow this down, set upper bounds on rect generation & text rendering
    if(l < Buffer->LineRectDataStart || l >= Buffer->Lines.Count ||
       c < 0 || c >= Buffer->Lines[l].Length)
        return {0};
    return Buffer->LineRectDataList.Data[l - Buffer->LineRectDataStart].CharRects.Data[c];
}

rect
LineRect(buffer *Buffer, int l)
{
    // TODO: narrow this down, set upper bounds on rect generation & text rendering
    if(l < Buffer->LineRectDataStart || l >= Buffer->Lines.Count)
        return {0};
    return Buffer->LineRectDataList.Data[l - Buffer->LineRectDataStart].LineRect;
}

char
CharAt(buffer *Buffer, int l, int c)
{
    if(l < 0 || l >= Buffer->Lines.Count ||
       c < 0 || c >= Buffer->Lines[l].Length)
        return 0;
    return Buffer->Lines[l].Data[c];
}

int
GetVirtualLineCount(program_state *ProgramState, buffer *Buffer, int TestViewPos)
{
    int MarginLeft = ProgramState->MarginLeft;
    int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
    int NumbersWidth = ProgramState->NumbersWidth;
    int SubLineOffset = ProgramState->SubLineOffset;
    int CharWidth = MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0).x;
    int WrapPoint = CharsPerVirtualLine*CharWidth + NumbersWidth*CharWidth + MarginLeft;
    
    int LineCount = 1;
    int x = NumbersWidth*CharWidth + MarginLeft;
    for(int i = 0; i < Buffer->Lines[TestViewPos].Length; i++)
    {
        if(x+CharWidth >= WrapPoint)
        {
            x = NumbersWidth*CharWidth + MarginLeft + SubLineOffset*CharWidth;
            LineCount++;
        }
        x += CharWidth;
    }
    
    return LineCount;
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
    
    int x = NumbersWidth*CharWidth + MarginLeft;
    int y = 0;
    Buffer->TextRect = Rect(x,
                            y,
                            Buffer->Rect.width/4 - x,
                            Buffer->Rect.height - y);
}

void
FillLineRectData(buffer *Buffer, program_state *ProgramState)
{
    line_rect_data_list *DataList = &Buffer->LineRectDataList;
    
    int MarginLeft = ProgramState->MarginLeft;
    int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
    int NumbersWidth = ProgramState->NumbersWidth;
    int SubLineOffset = ProgramState->SubLineOffset;
    v2 FontDim = MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0);
    int CharWidth = FontDim.x;
    int CharHeight = FontDim.y;
    MeasureTextEx(ProgramState->FontMain, "_", ProgramState->FontSize, 0).x;
    int WrapPoint = Buffer->TextRect.x + Buffer->TextRect.width;
    //int WrapPoint = CharsPerVirtualLine*CharWidth + NumbersWidth*CharWidth + MarginLeft;
    
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
    *DataList = LineRectDataList();
    
    
    int StartLine = (f32)Buffer->ViewPos + Buffer->ViewSubPos;
    Buffer->LineRectDataStart = StartLine;
    int y = (Buffer->ViewPos - StartLine)*CharHeight;
    
    for(int i = 0; i < Buffer->Lines.Count; i++)
    {
        ListAdd(DataList, LineRectData());
        
        line_rect_data *RectData = &(DataList->Data[i]);
        string *Line = &Buffer->Lines[i];
        int x = Buffer->TextRect.x;
        
        RectData->LineRect = Rect(Buffer->TextRect.x, y,
                                  Buffer->TextRect.width, CharHeight*GetVirtualLineCount(ProgramState, Buffer, i+StartLine));
        
        for(int a = 0; a < Line->Length; a++)
        {
            if(x+CharWidth >= WrapPoint)
            {
                x = Buffer->TextRect.x + SubLineOffset;
                y += CharHeight;
            }
            
            ListAdd(&(RectData->CharRects), Rect(x, y, CharWidth, CharHeight));
            
            x += CharWidth;
        }
        y += CharHeight;
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
    
    
    f32 StartLine = (f32)Buffer->ViewPos + Buffer->ViewSubPos;
    int y = (Buffer->ViewPos - StartLine)*ProgramState->FontSize;
    
    for(int l = Buffer->LineRectDataStart; l < Buffer->LineRectDataStart + Buffer->LineRectDataList.Count; l++)
    {
        DrawRectangleRec(LineRect(Buffer, l), GREEN);
        for(int c = 0; c < CharsInLine(Buffer, l); c++)
        {
            DrawRectangleLinesEx(RectAt(Buffer, l, c), 1, ORANGE);
        }
    }
    
    DrawRectangle(0, 0, 4*CharWidth, ProgramState->ScreenHeight, LIGHTGRAY);
    
    BeginShaderMode(ProgramState->ShaderSDF);
    for(int l = Buffer->ViewPos; l < Buffer->LineRectDataStart + Buffer->LineRectDataList.Count; l++)
    {
        char NumberBuffer[6];
        sprintf(NumberBuffer, "%d", l);
        DrawTextEx(ProgramState->FontSDF, NumberBuffer, V2(0, RectAt(Buffer, l, 0).y), FontSize, 0, GRAY);
        
        for(int c = 0; c < CharsInLine(Buffer, l); c++)
        {
            char CharBuffer[2] = {CharAt(Buffer, l, c), 0};
            //DrawRectangleLinesEx(RectAt(Buffer, l, c), 1, ORANGE);
            rect Rect = RectAt(Buffer, l, c);
            DrawTextEx(ProgramState->FontSDF, CharBuffer, V2(Rect.x, Rect.y), FontSize, 0, BLACK);
        }
    }
    EndShaderMode();
    
    DrawRectangleLinesEx(Buffer->TextRect, 3, PURPLE);
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
            //strcpy(Buffers[0].Data, FileData);
            
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
            ProgramState->SubLineOffset = 4;
            ProgramState->MarginLeft = 10;
            ProgramState->NumbersWidth = 4;
            Buffer->ViewPos = 0;
            Buffer->ViewSubPos = 0;
            Buffer->LineRectDataList = {0};
            
            
            ProgramState->ScreenHeight = Memory->WindowHeight;
            ProgramState->ScreenWidth = Memory->WindowWidth;
            ComputeBufferRects(ProgramState, Buffer);
            
        }
        ProgramState->ScreenHeight = Memory->WindowHeight;
        ProgramState->ScreenWidth = Memory->WindowWidth;
        int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
        int SubLineOffset = ProgramState->SubLineOffset;
        int MarginLeft = ProgramState->MarginLeft;
        int NumbersWidth = ProgramState->NumbersWidth;
        
        
        
        if(WindowShouldClose())
        {
            Memory->IsRunning = false;
        }
        
        
        if(IsKeyDown(KEY_LEFT))
            Buffer->TextRect.width -= 3;
        //ProgramState->CharsPerVirtualLine--;
        if(IsKeyDown(KEY_RIGHT))
            Buffer->TextRect.width += 3;
        //ProgramState->CharsPerVirtualLine++;
        Clamp(ProgramState->CharsPerVirtualLine, 10, 10000000);
        
        if(IsAnyControlKeyDown)
        {
            ProgramState->FontSize += GetMouseWheelMove();
            if(ProgramState->FontSize < 6) ProgramState->FontSize = 6;
            if(ProgramState->FontSize > 100) ProgramState->FontSize = 100;
        }
        
        int FontSize = ProgramState->FontSize;
        // which char doesn't matter since it's a monospace font
        int CharWidth = MeasureTextEx(*FontMain, "_", FontSize, 0).x;
        
        if(!(IsAnyControlKeyDown))
        {
            f32 NewViewSubPos = Buffer->ViewSubPos - GetMouseWheelMove();
            int NewViewPos = Buffer->ViewPos;
            printf("%d: %d lines\n", NewViewPos, GetVirtualLineCount(ProgramState, Buffer, NewViewPos));
            
            if(NewViewPos == Buffer->Lines.Count - 1 && NewViewSubPos > 0)
            {
                NewViewSubPos = 0;
            }
            
            if(NewViewSubPos < 0)
            {
                if(NewViewPos == 0)
                {
                    NewViewSubPos = 0;
                }
                else
                {
                    NewViewPos--;
                    NewViewSubPos = GetVirtualLineCount(ProgramState, Buffer, NewViewPos)+NewViewSubPos;
                }
            }
            if(NewViewSubPos >= GetVirtualLineCount(ProgramState, Buffer, NewViewPos))
            {
                NewViewPos++;
                NewViewSubPos -= GetVirtualLineCount(ProgramState, Buffers, NewViewPos-1);
            }
            
            Buffers->ViewPos = NewViewPos;
            Buffers->ViewSubPos = NewViewSubPos;
        }
        
        if(Buffers[0].ViewPos < 0) Buffers[0].ViewPos = 0;
        if(Buffers[0].ViewPos > Buffers[0].Lines.Count-1) Buffers[0].ViewPos = Buffers[0].Lines.Count-1;
        
        
        FillLineRectData(Buffer, ProgramState);
        
        
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        
        DrawBuffer(ProgramState, Buffer);
        
        
        
        
#if 0
        char buffer[60];
        sprintf(buffer, "%d", WrapPoint);
        DrawText(buffer, WrapPoint, 0, 20, RED);
#endif
        
        EndDrawing();
    }
}