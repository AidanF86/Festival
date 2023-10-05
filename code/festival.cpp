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
        string *Buffer = &ProgramState->Buffer;
        Font *FontMain = &ProgramState->FontMain;
        Font *FontSDF = &ProgramState->FontSDF;
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            
            // TEXT FILE
            const char *Filename = "test.cpp";
            
            u32 FileSize = (u32)GetFileLength(Filename);
            char *FileData = LoadFileText(Filename);
            
            ProgramState->Buffer = AllocString(FileSize);
            strcpy(ProgramState->Buffer.Data, FileData);
            
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
        
        
        BeginDrawing();
        
        ClearBackground(RAYWHITE);
        
        int y = 0;
        int x = 0;
        int FontDrawSize = 64;
        
        BeginShaderMode(ProgramState->ShaderSDF);
        for(int i = 0; i < Buffer->Length; i++)
        {
            char NextChar[2] = {Buffer->Data[i], 0};
            
            DrawTextEx(ProgramState->FontSDF, NextChar, V2(x, y), FontDrawSize, 0, BLACK);
            //DrawTextEx(ProgramState->FontMain, NextChar, V2(x, y), FontDrawSize, 0, BLACK);
            
            x += FontDrawSize/2;
            if(NextChar[0] == '\n')
            {
                y += FontDrawSize;
                x = 0;
            }
        }
        EndShaderMode();
        
        EndDrawing();
    }
}