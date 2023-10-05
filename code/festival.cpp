#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

typedef Vector2 v2;
typedef Vector3 v3;
typedef Rectangle rect;

#include "festival_platform.h"
#include "festival_math.h"
#include "festival_string.h"
#include "festival.h"

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
                // need to add AllocString to easily do this
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
            //ProgramState->FontMain = LoadFontEx("LiberationMono-Regular.ttf", 128, NULL, 0);
            
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
            
            
            ProgramState->FontSize = 22;
            ProgramState->CharsPerVirtualLine = 10;
            ProgramState->SubLineOffset = 4;
            ProgramState->MarginLeft = 10;
            ProgramState->NumbersWidth = 4;
            Buffer->ViewPos = 0;
            Buffer->ViewSubPos = 0;
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
            ProgramState->CharsPerVirtualLine--;
        if(IsKeyDown(KEY_RIGHT))
            ProgramState->CharsPerVirtualLine++;
        Clamp(ProgramState->CharsPerVirtualLine, 10, 10000000);
        
        if(IsAnyControlKeyDown)
        {
            ProgramState->FontSize += GetMouseWheelMove();
            if(ProgramState->FontSize < 6) ProgramState->FontSize = 6;
            if(ProgramState->FontSize > 100) ProgramState->FontSize = 100;
        }
        else
        {
            f32 NewViewPos = Buffers[0].ViewPos - 0.3*GetMouseWheelMove();
            Buffers[0].ViewPos = NewViewPos;
        }
        int FontSize = ProgramState->FontSize;
        // which char doesn't matter since it's a monospace font
        int CharWidth = MeasureTextEx(*FontMain, "_", FontSize, 0).x;
        
        if(Buffers[0].ViewPos < 0) Buffers[0].ViewPos = 0;
        if(Buffers[0].ViewPos > Buffers[0].Lines.Count-1) Buffers[0].ViewPos = Buffers[0].Lines.Count-1;
        
        
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        int StartLine = Buffer->ViewPos + Buffer->ViewSubPos;
        int y = ((f32)StartLine - Buffer->ViewPos)*FontSize;
        int x = 0;
        
        DrawRectangle(0, 0, 4*CharWidth, ProgramState->ScreenHeight, LIGHTGRAY);
        
        int WrapPoint = CharsPerVirtualLine*CharWidth + NumbersWidth*CharWidth + MarginLeft;
        
        BeginShaderMode(ProgramState->ShaderSDF);
        for(int i = Buffer->ViewPos; i < Buffer->Lines.Count; i++)
        {
            string *Line = &Buffer->Lines[i];
            
            char NumberBuffer[6];
            sprintf(NumberBuffer, "%d", i);
            DrawTextEx(ProgramState->FontSDF, NumberBuffer, V2(x, y), FontSize, 0, GRAY);
            
            x += NumbersWidth*CharWidth + MarginLeft;
            
            for(int a = 0; a < Line->Length; a++)
            {
                if(x+CharWidth >= WrapPoint)
                {
                    y += FontSize;
                    x = NumbersWidth*CharWidth + MarginLeft + SubLineOffset*CharWidth;
                }
                
                Color DrawColor = BLACK;
                
                char NextChar[2] = {Line->Data[a], 0};
                if(NextChar[0] == ' '){ NextChar[0] = '_'; DrawColor = LIGHTGRAY; };
                if(NextChar[0] == '\n') NextChar[0] = '|';
                
                DrawTextEx(ProgramState->FontSDF, NextChar, V2(x, y), FontSize, 0, DrawColor);
                
                x += CharWidth;
            }
            
            y += FontSize;
            x = 0;
        }
        EndShaderMode();
        
        DrawLine(WrapPoint, 0, WrapPoint, ProgramState->ScreenHeight, RED);
        char buffer[6];
        sprintf(buffer, "%d", CharsPerVirtualLine);
        DrawText(buffer, CharsPerVirtualLine*CharWidth, 0, 20, RED);
        
        EndDrawing();
    }
}