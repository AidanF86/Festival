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
        }
        
        if(WindowShouldClose())
        {
            Memory->IsRunning = false;
        }
        
        
        f32 NewViewPos = Buffers[0].ViewPos - GetMouseWheelMove();
        Buffers[0].ViewPos = NewViewPos;
        
        if(Buffers[0].ViewPos < 0) Buffers[0].ViewPos = 0;
        if(Buffers[0].ViewPos > Buffers[0].Lines.Count-1) Buffers[0].ViewPos = Buffers[0].Lines.Count-1;
        
        
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        int y = 0;
        int x = 0;
        int FontDrawSize = 22;
        
        BeginShaderMode(ProgramState->ShaderSDF);
        for(int i = Buffer->ViewPos; i < Buffer->Lines.Count; i++)
        {
            string *Line = &Buffer->Lines[i];
            
            for(int a = 0; a < Line->Length; a++)
            {
                char NextChar[2] = {Line->Data[a], 0};
                
                DrawTextEx(ProgramState->FontSDF, NextChar, V2(x, y), FontDrawSize, 0, BLACK);
                
                x += FontDrawSize/2;
                
            }
            
            y += FontDrawSize;
            x = 0;
        }
        EndShaderMode();
        
        EndDrawing();
    }
}