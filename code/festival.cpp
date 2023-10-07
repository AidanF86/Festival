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


#include <time.h>
#define NanoToMilli (1.0/1000000.0)

u64
GetNanoseconds() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (u64)ts.tv_sec * 1000000000L + ts.tv_nsec;
}


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
    return Buffer->Lines.Data[l].Length;
}

rect
RectAt(buffer *Buffer, int l, int c)
{
#if 0
    // TODO: narrow this down, set upper bounds on rect generation & text rendering
    if(l < Buffer->LineRectDataStart || l >= Buffer->LineRectDataStart + Buffer->LineRectDataList.Count ||
       c < 0 || c >= Buffer->Lines[l].Length)
        return {0};
    rect Result = Buffer->LineRectDataList.Data[l - Buffer->LineRectDataStart].CharRects.Data[c];
    if(Result.x == 0)
    {
        printf("%d, %d\n", l, c);
        printf("Effective: %d\n", l - Buffer->LineRectDataStart);
    }
    return Result;
#endif
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
       c < 0 || c >= Buffer->Lines.Data[l].Length)
        return 0;
    return Buffer->Lines[l].Data[c];
}

line_rect_data
GetLineRectData(buffer *Buffer, int l)
{
    if(l < 0 || l >= Buffer->Lines.Count)
    {
        printf("GetLineRectData: Out of bounds!\n");
        return {0};
    }
    return Buffer->LineRectDataList.Data[l - Buffer->LineRectDataStart];
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
                            Buffer->Rect.width - x,
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
    
    
    //int StartLine = Buffer->ViewPos;
    //Buffer->LineRectDataStart = StartLine;
    int y = (-1 * Buffer->ViewSubPos);//*CharHeight;
    
    for(int i = 0; i < Buffer->Lines.Count; i++)
    {
        //if(y >= Buffer->TextRect.y + Buffer->TextRect.height){ break; }
        
        ListAdd(DataList, LineRectData());
        
        line_rect_data *RectData = &(DataList->Data[i]);
        string *Line = &Buffer->Lines.Data[i];
        int x = Buffer->TextRect.x;
        
        RectData->LineRect.x = x;
        RectData->LineRect.y = y;
        RectData->LineRect.width = Buffer->TextRect.width;
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
        y += CharHeight;
        
        RectData->LineRect.height = RectData->DisplayLines * CharHeight;
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
    
    
    DrawRectangle(0, 0, 4*CharWidth, ProgramState->ScreenHeight, LIGHTGRAY);
    
    BeginShaderMode(ProgramState->ShaderSDF);
    
    
    int StartY = LineRect(Buffer, Buffer->ViewPos).y + Buffer->ViewSubPos;
    //printf("%d\n", Buffer->LineRectDataList.Count);
    
    for(int l = Buffer->ViewPos; l < Buffer->LineRectDataList.Count; l++)
    {
        char NumberBuffer[6];
        sprintf(NumberBuffer, "%d", l);
        
        DrawTextEx(ProgramState->FontSDF, NumberBuffer, V2(0, LineRect(Buffer, l).y-StartY), FontSize, 0, GRAY);
        //DrawRectangleLinesEx(GetLineRectData(Buffer, l).LineRect, 1, GREEN);
        
        int LineY = GetLineRectData(Buffer, l).LineRect.y;
        if(LineY - StartY > Buffer->TextRect.y + Buffer->TextRect.height) break;
        
        for(int c = 0; c < GetLineRectData(Buffer, l).CharRects.Count; c++)
        {
            char CharBuffer[2] = {CharAt(Buffer, l, c), 0};
            if(CharBuffer[0] == 0 && l == 9) CharBuffer[0] = '|';
            
            rect Rect = RectAt(Buffer, l, c);
            Rect.y -= StartY;
            
            //DrawRectangleLinesEx(Rect, 1, ORANGE);
            DrawRectangleRec(Rect, GRAY);
            DrawTextEx(ProgramState->FontSDF, CharBuffer, V2(Rect.x, Rect.y), FontSize, 0, BLACK);
        }
    }
    EndShaderMode();
    
    
    DrawRectangleLinesEx(Buffer->TextRect, 3, PURPLE);
    
    char buffer[60];
    sprintf(buffer, "%d", Buffer->ViewPos);
    DrawText(buffer, Buffer->TextRect.x + Buffer->TextRect.width, 0, 20, PURPLE);
    sprintf(buffer, "%d", Buffer->ViewSubPos);
    DrawText(buffer, Buffer->TextRect.x + Buffer->TextRect.width, 30, 20, PURPLE);
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
            Buffer->ViewSubPos = 0;
            Buffer->LineRectDataList = {0};
            
            
            ProgramState->ScreenHeight = Memory->WindowHeight;
            ProgramState->ScreenWidth = Memory->WindowWidth;
            
            
            ComputeBufferRects(ProgramState, Buffer);
            FillLineRectData(Buffer, ProgramState);
        }
        
        
        
        ProgramState->ScreenHeight = Memory->WindowHeight;
        ProgramState->ScreenWidth = Memory->WindowWidth;
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
        
        if(IsKeyDown(KEY_LEFT))
            Buffer->TextRect.width -= 6;
        if(IsKeyDown(KEY_RIGHT))
            Buffer->TextRect.width += 6;
        // TODO: this probably doesn't account for a rect with x>0
        Clamp(Buffer->TextRect.width, CharWidth, Buffer->Rect.width - Buffer->TextRect.x);
        
        if(IsAnyControlKeyDown)
        {
            ProgramState->FontSize += GetMouseWheelMove();
            if(ProgramState->FontSize < 6) ProgramState->FontSize = 6;
            if(ProgramState->FontSize > 100) ProgramState->FontSize = 100;
        }
        
        if(!(IsAnyControlKeyDown))
        {
            int NewViewSubPos = Buffer->ViewSubPos;
            // TODO: mouse scrolling is kind of stuttery, especially up
            NewViewSubPos -= GetMouseWheelMove()*20;
            NewViewSubPos += IsKeyDown(KEY_DOWN);
            NewViewSubPos -= IsKeyDown(KEY_UP);
            
            int NewViewPos = Buffer->ViewPos;
            
            if(NewViewPos == Buffer->Lines.Count - 1 && NewViewSubPos > 0)
            {
                NewViewSubPos = 0;
            }
            
            if(NewViewSubPos < 0)
            {
                // TODO: issue with Print and not zeroing end of string
                if(NewViewPos == 0)
                {
                    NewViewSubPos = 0;
                }
                else
                {
                    NewViewPos--;
                    // ISSUE: The prev line rect data doesnt exist... not sure how to solve this
                    NewViewSubPos = GetLineRectData(Buffer, NewViewPos).DisplayLines*CharHeight - 1;
                    //NewViewSubPos = GetLineRectData(Buffer, NewViewPos).DisplayLines*CharHeight;// + NewViewSubPos;
                }
            }
            if(NewViewSubPos >= GetLineRectData(Buffer, NewViewPos).DisplayLines*CharHeight)
            {
                NewViewSubPos -= GetLineRectData(Buffer, NewViewPos).DisplayLines*CharHeight;
                NewViewPos++;
            }
            
            Buffers->ViewPos = NewViewPos;
            Buffers->ViewSubPos = NewViewSubPos;
        }
        
        if(Buffers[0].ViewPos < 0) Buffers[0].ViewPos = 0;
        if(Buffers[0].ViewPos > Buffers[0].Lines.Count-1) Buffers[0].ViewPos = Buffers[0].Lines.Count-1;
        
        
        ComputeBufferRects(ProgramState, Buffer);
        u64 BeforeTime = GetNanoseconds();
        FillLineRectData(Buffer, ProgramState);
        
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawBuffer(ProgramState, Buffer);
        printf("Fill time: %lu\n", (GetNanoseconds() - BeforeTime) /1000000);
        
        
#if 1
#endif
        
        EndDrawing();
    }
}