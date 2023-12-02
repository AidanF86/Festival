#include <stdio.h>

#include "raylib.h"

#include "festival_platform.h"

#include "festival_math.h"
#include "festival_string.h"
#include "festival.h"
#include "festival_lists.h"

#include <time.h>
#define NanoToMilli (1.0/1000000.0)

#include "festival_functions.cpp"


void
DrawChar(program_state *ProgramState, int Char, v2 Pos, int Size, color FGColor, color BGColor)
{
    v2 CharDim = GetCharDim(ProgramState, Size);
    if(BGColor.a != 0)
    {
        DrawRectangleV(V(Pos), V(CharDim), BGColor);
    }
    char CharBuffer[2] = {(char)Char, 0};
    DrawTextEx(ProgramState->FontSDF, CharBuffer, V(Pos), Size, 0, FGColor);
}

void
DrawChar(program_state *ProgramState, int Char, v2 Pos, int Size, color FGColor)
{
    DrawChar(ProgramState, Char, Pos, Size, FGColor, RGBA(0, 0, 0, 0));
}

void
DrawString(program_state *ProgramState, string String, v2 Pos, int Size, color FGColor, color BGColor)
{
    v2 CharDim = GetCharDim(ProgramState, Size);
    for(int i = 0; i < String.Length; i++)
    {
        DrawChar(ProgramState, String[i], Pos, Size, FGColor, BGColor);
        Pos.x += CharDim.x;
    }
}

void
DrawString(program_state *ProgramState, string String, v2 Pos, int Size, color FGColor)
{
    DrawString(ProgramState, String, Pos, Size, FGColor, RGBA(0, 0, 0, 0));
}

void
DrawView(program_state *ProgramState, view *View)
{
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    b32 ViewIsSelected = View == &ProgramState->Views[ProgramState->SelectedViewIndex];
    rect ViewRect = View->Rect;
    v2 ViewPos = V2(View->Rect.x, View->Rect.y);
    v2 ViewDim = V2(View->Rect.w, View->Rect.h);
    
    BeginScissorMode(ViewRect.x, ViewRect.y, ViewRect.w, ViewRect.h);
    
    DrawRectangleRec(R(View->Rect), ProgramState->BGColor);
    DrawRectangle(ViewPos.x, ViewPos.y, 4*CharWidth, ViewDim.h, ProgramState->LineNumberBGColor);
    
    rect CursorDrawRect = CharToScreenSpace(View, View->CursorRect);
    
    if(ViewIsSelected)
    {
        if(ProgramState->InputMode == InputMode_Insert)
        {
            DrawRectangleRec(R(Rect(CursorDrawRect.x, CursorDrawRect.y,
                                    CursorDrawRect.w/3, CursorDrawRect.h)),
                             ProgramState->CursorBGColor);
            
#if 0
            DrawRectangleRec(R(Rect(CursorDrawRect.x,
                                    CursorDrawRect.y + CursorDrawRect.h - CursorDrawRect.h/6,
                                    CursorDrawRect.w, CursorDrawRect.h/6)),
                             ProgramState->CursorBGColor);
#endif
        }
        else
            DrawRectangleRec(R(CursorDrawRect), ProgramState->CursorBGColor);
    }
    else
        DrawRectangleLinesEx(R(CursorDrawRect), 2, ProgramState->CursorBGColor);
    
    // Draw text
    BeginShaderMode(ProgramState->ShaderSDF);
    for(int l = 0; l < LineCount(View); l++)
    {
        string NumberString = String("%d", l);
        DrawString(ProgramState, NumberString, V2(View->Rect.x, LineRect(View, l).y - View->Y), ProgramState->FontSize, ProgramState->LineNumberFGColor);
        
        line_data LineData = LineDataAt(View, l);
        
        int LineY = LineData.LineRect.y;
        if(LineY - View->Y > View->TextRect.y + View->TextRect.h)
            break;
        
        for(int c = 0; c < LineData.CharRects.Count; c++)
        {
            rect Rect = ScreenRectAt(View, l, c);
            
            color CharColor = ProgramState->FGColor;
            if(ViewIsSelected && BufferPos(l, c) == View->CursorPos && ProgramState->InputMode != InputMode_Insert)
            {
                CharColor = ProgramState->CursorFGColor;
            }
            
            DrawChar(ProgramState, CharAt(View, l, c), V2(Rect), ProgramState->FontSize, CharColor);
        }
    }
    EndShaderMode();
    
    if(ProgramState->ShowViewRects)
    {
        //DrawRectangleLinesEx(R(View->TextRect), 2, {159, 192, 123, 255});
    }
    
    if(ProgramState->ShowViewInfo)
    {
        int InfoFontSize = 20;
        int KeyValueDistanceChars = 10;
        int InfoCharHeight = GetCharDim(ProgramState).y;
        int InfoCharWidth = GetCharDim(ProgramState).x;
        int KeyValueDistance = KeyValueDistanceChars * InfoCharWidth;
        
        DrawRectangleRec(R(View->Rect), {0, 0, 0, 200});
        BeginShaderMode(ProgramState->ShaderSDF);
        {
            v2 TextPos = V2(View->Rect.x + 10, View->Rect.y + 10);
            DrawString(ProgramState, String("id"), TextPos, InfoFontSize, WHITE);
            DrawString(ProgramState, String("%d", View->Id), TextPos + V2(KeyValueDistance, 0), InfoFontSize, YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(ProgramState, String("parent"), TextPos, InfoFontSize, WHITE);
            DrawString(ProgramState, String("%d", View->ParentId), TextPos + V2(KeyValueDistance, 0), InfoFontSize, YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(ProgramState, String("birth #"), TextPos, InfoFontSize, WHITE);
            DrawString(ProgramState, String("%d", View->BirthOrdinal), TextPos + V2(KeyValueDistance, 0), InfoFontSize, YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(ProgramState, String("area"), TextPos, InfoFontSize, WHITE);
            DrawString(ProgramState, String("%d", View->Area), TextPos + V2(KeyValueDistance, 0), InfoFontSize, YELLOW);
        }
        EndShaderMode();
    }
    
    if(ProgramState->ShowViewRects)
    {
        DrawRectangleLinesEx(R(View->Rect), 2, {216, 50, 10, 255});
    }
    
    EndScissorMode();
}


void
MoveCursorPos(program_state *ProgramState, view *View, buffer_pos dPos)
{
    // TODO: set ideal cursor pos
    ProgramState->UserMovedCursor = true;
    View->CursorPos += dPos;
    Clamp(View->CursorPos.l, 0, LineCount(View));
    Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
}

/*
 hi  yeah   well   hello
*/

void
MoveBackNonWhitespace(program_state *ProgramState, view *View)
{
    b32 StartedAtSpace = false;
    if(CharAt(View, View->CursorPos) == ' ' || CharAt(View, View->CursorPos - BufferPos(0, 1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        do {
            View->CursorPos -= BufferPos(0, 1);
        }while(CharAt(View, View->CursorPos) == ' ' && View->CursorPos.c > 0);
    }
    
    while(CharAt(View, View->CursorPos) != ' ' && View->CursorPos.c > 0)
    {
        View->CursorPos -= BufferPos(0, 1);
    }
}

void
MoveForwardNonWhitespace(program_state *ProgramState, view *View)
{
    b32 StartedAtSpace = false;
    if(CharAt(View, View->CursorPos) == ' ' || CharAt(View, View->CursorPos + BufferPos(0, 1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        do {
            //View->CursorPos += BufferPos(0, 1);
            MoveCursorPos(ProgramState, View, BufferPos(0, 1));
        }while(CharAt(View, View->CursorPos) == ' ' && View->CursorPos.c < LineLength(View, View->CursorPos.l));
    }
    
    while(CharAt(View, View->CursorPos) != ' ' && View->CursorPos.c < LineLength(View, View->CursorPos.l))
    {
        //View->CursorPos += BufferPos(0, 1);
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
}

buffer_pos
SeekBackBorder(view *View, buffer_pos From)
{
    buffer_pos Result = From;
    if(CharAt(View, Result) == 0)
        Result.c--;
    if(Result.c < 0)
        return From;
    
    b32 StartedAtSpace = false;
    if(CharAt(View, Result) == ' ' || CharAt(View, Result + BufferPos(0, -1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        Print("Started at space");
        do {
            Result.c--;
        }while(CharAt(View, Result) == ' ' && Result.c < LineLength(View, Result.l));
        
        return Result;
    }
    
    b32 StartedAtSpecial = false;
    if(!IsNonSpecial(CharAt(View, Result)))
    {
        StartedAtSpecial = true;
    }
    
    if(StartedAtSpecial)
    {
        while(!IsNonSpecial(CharAt(View, Result)) && 
              Result.c < LineLength(View, Result.l))
            Result.c--;
        return Result;
    }
    
    char c = CharAt(View, Result);
    while(( c != ' ' && (IsNonSpecial(c)) )
          && Result.c < LineLength(View, Result.l))
    {
        Result.c--;
        c = CharAt(View, Result);
    }
    
    return Result;
}

buffer_pos
SeekForwardBorder(view *View, buffer_pos From)
{
    buffer_pos Result = From;
    
    b32 StartedAtSpace = false;
    if(CharAt(View, Result) == ' ' || CharAt(View, Result + BufferPos(0, 1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        do {
            Result.c++;
        }while(CharAt(View, Result) == ' ' && Result.c < LineLength(View, Result.l));
        
        return Result;
    }
    
    b32 StartedAtSpecial = false;
    if(!IsNonSpecial(CharAt(View, Result)))
    {
        StartedAtSpecial = true;
    }
    
    if(StartedAtSpecial)
    {
        while(!IsNonSpecial(CharAt(View, Result))
              && Result.c < LineLength(View, Result.l))
            Result.c++;
        return Result;
    }
    
    char c = CharAt(View, Result);
    while(( c != ' ' && (IsNonSpecial(c)) )
          && Result.c < LineLength(View, Result.l))
    {
        Result.c++;
        c = CharAt(View, Result);
    }
    
    return Result;
}

b32
AtLineBeginning(view *View, buffer_pos Pos)
{
    return Pos.c == 0;
}
b32
AtLineEnd(view *View, buffer_pos Pos)
{
    return Pos.c == LineLength(View, Pos.l);
}

buffer_pos
SeekLineBeginning(view *View, buffer_pos From)
{
    return BufferPos(From.l, 0);
}
buffer_pos
SeekLineEnd(view *View, buffer_pos From)
{
    return BufferPos(From.l, LineLength(View, From.l));
}

buffer_pos
SeekPrevEmptyLine(view *View, buffer_pos From)
{
    int ResultLine = From.l;
    while(ResultLine > 0)
    {
        ResultLine--;
        if(LineLength(View, ResultLine) == 0)
            break;
    }
    return BufferPos(ResultLine, 0);
}
buffer_pos
SeekNextEmptyLine(view *View, buffer_pos From)
{
    int ResultLine = From.l;
    while(ResultLine < LineCount(View))
    {
        ResultLine++;
        if(LineLength(View, ResultLine) == 0)
            break;
    }
    return BufferPos(ResultLine, LineLength(View, ResultLine) + 1);
}

void
SetCursorPos(program_state *ProgramState, view *View, buffer_pos Pos)
{
    ProgramState->UserMovedCursor = true;
    View->CursorPos = Pos;
    Clamp(View->CursorPos.l, 0, LineCount(View));
    Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
}

void
HandleInput_Nav(program_state *ProgramState)
{
    if(KeyShouldExecute(ProgramState->FKey))
    {
        ProgramState->InputMode = InputMode_Insert;
    }
    
    if(KeyShouldExecute(ProgramState->NKey))
    {
        if(IsAnyShiftKeyDown)
        {
            ListAdd(&ProgramState->Views, View(ProgramState, &ProgramState->Buffers[0], ProgramState->Views.Data[ProgramState->SelectedViewIndex].Id, Location_Below));
            printf("splitting view vertically\n");
        }
        else
        {
            ListAdd(&ProgramState->Views, View(ProgramState, &ProgramState->Buffers[0], ProgramState->Views.Data[ProgramState->SelectedViewIndex].Id, Location_Right));
            printf("splitting view horizontally\n");
        }
    }
    
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    if(KeyShouldExecute(ProgramState->IKey))
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
    if(KeyShouldExecute(ProgramState->KKey))
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
    if(KeyShouldExecute(ProgramState->JKey))
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
    if(KeyShouldExecute(ProgramState->LKey))
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    
    if(KeyShouldExecute(ProgramState->UKey))
        SetCursorPos(ProgramState, View, SeekBackBorder(View, View->CursorPos));
    if(KeyShouldExecute(ProgramState->OKey))
        SetCursorPos(ProgramState, View, SeekForwardBorder(View, View->CursorPos));
    
    if(KeyShouldExecute(ProgramState->HKey))
    {
        if(!AtLineBeginning(View, View->CursorPos))
            SetCursorPos(ProgramState, View, SeekLineBeginning(View, View->CursorPos));
        else
            SetCursorPos(ProgramState, View, SeekPrevEmptyLine(View, View->CursorPos));
    }
    if(KeyShouldExecute(ProgramState->Semicolon_Key))
    {
        if(!AtLineEnd(View, View->CursorPos))
            SetCursorPos(ProgramState, View, SeekLineEnd(View, View->CursorPos));
        else
            SetCursorPos(ProgramState, View, SeekNextEmptyLine(View, View->CursorPos));
    }
    
    if(KeyShouldExecute(ProgramState->WKey))
    {// TODO: write current buffer to file
    }
    
    if(KeyShouldExecute(ProgramState->QKey))
    {
        if(IsAnyControlKeyDown)
        {
            if(IsAnyShiftKeyDown)
            {// Exit program
                ProgramState->ShouldExit = true;
            }
            else
            {// Close current view
                RemoveView(ProgramState, ProgramState->SelectedViewIndex);
            }
        }
        else
        {// TODO: Close current buffer
        }
    }
    
}


void
HandleInput_Insert(program_state *ProgramState)
{
    buffer *Buffer = ProgramState->Views[ProgramState->SelectedViewIndex].Buffer;
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    if(KeyShouldExecute(ProgramState->Escape_Key))
    {
        ProgramState->InputMode = InputMode_Nav;
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
                // TODO: handling tab char
                for(int a = 0; a < 4; a++)
                {
                    Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, ' ');
                    View->CursorPos.c++;
                    View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
                }
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
}

void
HandleInput(program_state *ProgramState)
{
    if(ProgramState->InputMode == InputMode_Nav)
        HandleInput_Nav(ProgramState);
    else if(ProgramState->InputMode == InputMode_Insert)
        HandleInput_Insert(ProgramState);
}


extern "C"
{
    PROGRAM_UPDATE_AND_RENDER(ProgramUpdateAndRender)
    {
        program_state *ProgramState = (program_state *)Memory->Data;
        ProgramState->ScreenWidth = Memory->WindowWidth;
        ProgramState->ScreenHeight = Memory->WindowHeight;
        buffer *Buffers = ProgramState->Buffers;
        view_list *Views = &ProgramState->Views;
        buffer *Buffer = &ProgramState->Buffers[0];
        Font *FontMain = &ProgramState->FontMain;
        Font *FontSDF = &ProgramState->FontSDF;
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            ProgramState->ShouldExit = false;
            
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
                for(i; FileData[i] != '\n' && i < FileSize; i++) {}
                
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
            
            LoadFonts(ProgramState, FontMain, FontSDF);
            ProgramState->KeyFirstRepeatTime = 0.4f;
            ProgramState->KeyRepeatSpeed = 0.02f;
            
            FillKeyData(ProgramState);
            
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
            
            ProgramState->Views = ViewList();
            ListAdd(Views, View(ProgramState, &Buffers[0], -1, Location_Below));
            
            ProgramState->SelectedViewIndex = 0;
            
            ProgramState->ShowViewInfo = false;
            ProgramState->ShowViewRects = false;
            
            FillLineData(&ProgramState->Views[0], ProgramState);
        }
        
        UpdateKeyInput(ProgramState);
        
        ProgramState->ScreenWidth = Memory->WindowWidth;
        ProgramState->ScreenHeight = Memory->WindowHeight;
        
        BeginDrawing();
        
        int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
        int SubLineOffset = ProgramState->SubLineOffset;
        int MarginLeft = ProgramState->MarginLeft;
        int NumbersWidth = ProgramState->NumbersWidth;
        
        v2 CharDim = GetCharDim(ProgramState);
        int CharWidth = CharDim.x;
        int CharHeight = CharDim.y;
        
        if(WindowShouldClose() || ProgramState->ShouldExit)
        {
            Memory->IsRunning = false;
        }
        
        /*
                if(KeyShouldExecute(ProgramState->IKey))
                    ProgramState->ShowViewInfo = !ProgramState->ShowViewInfo;
        */
        
        
        
        view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
        
        // Compute view rects
        
        // Root view is at top left
        // Compute root view
        b32 FoundRootView;
        view *RootView;
        for(int i = 0; i < Views->Count; i++)
        {
            if(Views->Data[i].Id == 0)
            {
                FoundRootView = true;
                RootView = &(Views->Data[i]);
                break;
            }
        }
        if(!FoundRootView)
            printf("MAJOR ERROR: CAN'T FIND ROOT VIEW\n");
        
        
        for(int i = 0; i < Views->Count; i++)
        {
            Views->Data[i].ComputedFromParentThisFrame = false;
        }
        
#if 0
        rect *OldRects = (rect *)malloc(sizeof(rect) * Views->Count);
        for(int i = 0; i < Views->Count; i++)
        {
            OldRects[i] = Views->Data[i].Rect;
        }
#endif
        
        RootView->Rect.x = 0;
        RootView->Rect.y = 0;
        RootView->Rect.w = ProgramState->ScreenWidth;
        RootView->Rect.h = ProgramState->ScreenHeight;
        ComputeTextRect(ProgramState, RootView);
        RootView->ComputedFromParentThisFrame = true;
        
        for(int ViewIndex = 0; ViewIndex < Views->Count; ViewIndex++)
        {
            int Id = Views->Data[ViewIndex].Id;
            // Compute child count
            int ChildCount = 0;
            for(int i = 0; i < Views->Count; i++)
            {
                if(Views->Data[i].ParentId == Id)
                    ChildCount++;
            }
            if(ChildCount == 0)
                continue;
            
            // Compute children
            
            // Operate on children in birth order
            for(int child = 0; child < ChildCount; child++)
            {
                view *NextChild = NULL;
                int LowestBirthOrdinal = 1000000;
                // get next child to operate on
                for(int i = 0; i < Views->Count; i++)
                {
                    view *TestView = &Views->Data[i];
                    if((NextChild == NULL && TestView->ParentId == Id && !TestView->ComputedFromParentThisFrame) || (TestView->ParentId == Id && !TestView->ComputedFromParentThisFrame && TestView->BirthOrdinal < LowestBirthOrdinal))
                    {
                        NextChild = TestView;
                        LowestBirthOrdinal = NextChild->BirthOrdinal;
                    }
                }
                if(NextChild == NULL)
                {
                    printf("ERROR: Couldn't find next child!\n");
                    break;
                }
                
                // Compute child
                
                view *Parent = &Views->Data[ViewIndex];
                view *Child = NextChild;
                f32 Ratio = Child->Area;
                //Ratio = 0.5f;
                //printf("id: %d\n", Child->Id);
                //printf("%f\n", Ratio);
                
                if(Child->SpawnLocation == Location_Right)
                {
                    // horizontal splitting
                    Child->Rect.x = Parent->Rect.x + Parent->Rect.w * (1.0f-Ratio);
                    Child->Rect.y = Parent->Rect.y;
                    Child->Rect.w = Parent->Rect.w * Ratio;
                    Child->Rect.h = Parent->Rect.h;
                    Parent->Rect.w = Parent->Rect.w * (1.0f-Ratio);
                }
                else
                {
                    // vertical splitting
                    Child->Rect.x = Parent->Rect.x;
                    Child->Rect.y = Parent->Rect.y + Parent->Rect.h * (1.0f-Ratio);
                    Child->Rect.w = Parent->Rect.w;
                    Child->Rect.h = Parent->Rect.h * Ratio;
                    Parent->Rect.h = Parent->Rect.h * (1.0f-Ratio);
                }
                
                ComputeTextRect(ProgramState, Child);
                ComputeTextRect(ProgramState, Parent);
                
                NextChild->ComputedFromParentThisFrame = true;
            }
        }
        
        if(IsMouseButtonDown(0))
        {
            v2 MousePos = V2(GetMousePosition());
            
            int NewViewIndex = 0;
            for(int i = 0; i < Views->Count; i++)
            {
                rect Rect = Views->Data[i].Rect;
                if(MousePos.x >= Rect.x && MousePos.y >= Rect.y &&
                   MousePos.x <= Rect.x + Rect.w && MousePos.y <= Rect.y + Rect.h)
                {
                    NewViewIndex = i;
                }
            }
            ProgramState->SelectedViewIndex = NewViewIndex;
            View = &ProgramState->Views[ProgramState->SelectedViewIndex];
        }
        
        
        
        if(IsAnyControlKeyDown)
        {
            ProgramState->FontSize += GetMouseWheelMove();
            if(ProgramState->FontSize < 6) ProgramState->FontSize = 6;
            if(ProgramState->FontSize > 100) ProgramState->FontSize = 100;
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
        
        
        HandleInput(ProgramState);
        
        
        Clamp(View->CursorPos.l, 0, LineCount(View)-1);
        Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
        
        
#if 0
        printf("\n\n\n\n");
        for(int i = 0; i < Views->Count; i++)
        {
            view *View = &Views->Data[i];
            printf("View %d: parent=%d, birthordinal=%d, area=%f\n", View->Id, View->ParentId, View->BirthOrdinal, View->Area);
            printf("\trect=(%d, %d, %d, %d)\n", View->Rect.x, View->Rect.y, View->Rect.w, View->Rect.h);
            printf("\ttext=(%d, %d, %d, %d)\n", View->TextRect.x, View->TextRect.y, View->TextRect.w, View->TextRect.h);
        }
#endif
        
        
        AdjustView(ProgramState, View);
        
        
        for(int i = 0; i < Views->Count; i++)
        {
            view *View = &Views->Data[i];
            FillLineData(View, ProgramState);
            View->CursorTargetRect = CharRectAt(View, View->CursorPos.l, View->CursorPos.c);
            View->Y = Interpolate(View->Y, View->TargetY, 0.4f);
            View->CursorRect = Interpolate(View->CursorRect, View->CursorTargetRect, 0.5f);
        }
        
        //ClearBackground(ProgramState->BGColor);
        
        for(int i = 0; i < Views->Count; i++)
        {
            view *View = &Views->Data[i];
            DrawView(ProgramState, View);
        }
        //u64 BeforeTime = GetNanoseconds();
        //printf("Fill time: %lu\n", (GetNanoseconds() - BeforeTime) /1000000);
        
        
        EndDrawing();
    }
}