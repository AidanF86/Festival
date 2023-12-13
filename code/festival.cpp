#include <stdio.h>

#include "raylib.h"

#include "festival_platform.h"

#include "festival_math.h"
#include "festival_string.h"
#include "festival.h"
#include "festival_lists.h"

#include "festival_functions.cpp"
#include "festival_profiling.h"

#if 0
void
LoadCharTexture(program_state *ProgramState, int Char, int Size)
{
    if(Char < 0 || Char > 255)
        return;
    
    RenderTexture2D *Tex = &ProgramState->CharTextures[Char];
    if(ProgramState->CharTexturesExist[Char])
        UnloadRenderTexture(*Tex);
    
    v2 CharDim = GetCharDim(ProgramState, Size);
    GlyphInfo Info = ProgramState->FontMain.glyphs[Char];
    
    *Tex = LoadRenderTexture(CharDim.x + Info.advanceX, CharDim.y);
    
    {
        BeginTextureMode(*Tex);
        BeginShaderMode(ProgramState->ShaderSDF);
        
        char CharBuffer[2] = {(char)Char, 0};
        DrawTextEx(ProgramState->FontSDF, CharBuffer, {0,0}, Size, 0, WHITE);
        
        EndShaderMode();
        EndTextureMode();
    }
    
    printf("Yeah\n");
    ProgramState->CharTexturesExist[Char] = true;
}
#endif


void
DrawChar(program_state *ProgramState, int Char, v2 Pos, int Size, color BGColor, color FGColor)
{
    /*
        if(!ProgramState->CharTexturesExist[Char])
            return;
    */
    
    v2 CharDim = GetCharDim(ProgramState, Size);
    
    if(BGColor.a != 0)
        DrawRectangleV(V(Pos), V(CharDim), BGColor);
    
    //DrawTextureV(ProgramState->CharTextures[Char].texture, V(Pos), BLACK);
    
    // TODO: replace w/ ascii index
    GlyphInfo Info = {0};
    int GlyphIndex = CharIndex(&ProgramState->FontMain, Char);
    Info = ProgramState->FontMain.RFont.glyphs[GlyphIndex];
#if 0
    for(int i = 0; i < ProgramState->FontMain.RFont.glyphCount; i++)
    {
        if(ProgramState->FontMain.RFont.glyphs[i].value == Char)
        {
            Info = ProgramState->FontMain.RFont.glyphs[i];
            GlyphIndex = i;
            break;
        }
    }
#endif
    
    //rect DestRect = Rect(Pos.x, Pos.y, CharDim.x, CharDim.y);
    rect DestRect = Rect(Pos.x + Info.offsetX, Pos.y + Info.offsetY,
                         ProgramState->FontMain.RFont.recs[GlyphIndex].width,
                         ProgramState->FontMain.RFont.recs[GlyphIndex].height);
    
    DrawTexturePro(ProgramState->FontMain.RFont.texture,
                   //DrawTexturePro(ProgramState->CharTextures[Char].texture,
                   ProgramState->FontMain.RFont.recs[GlyphIndex],
                   R(DestRect),
                   {0, 0}, 0, FGColor);
}

void
DrawChar(program_state *ProgramState, int Char, v2 Pos, int Size, color FGColor)
{
    DrawChar(ProgramState, Char, Pos, Size, RGBA(0, 0, 0, 0), FGColor);
}

int
DrawString(program_state *ProgramState, string String, v2 Pos, int Size, color BGColor, color FGColor)
{
    v2 CharDim = GetCharDim(ProgramState, Size);
    for(int i = 0; i < String.Length; i++)
    {
        DrawChar(ProgramState, String[i], Pos, Size, BGColor, FGColor);
        Pos.x += CharDim.x;
    }
    return Pos.x;
}

int
DrawString(program_state *ProgramState, string String, v2 Pos, int Size, color FGColor)
{
    return DrawString(ProgramState, String, Pos, Size, RGBA(0, 0, 0, 0), FGColor);
}

void DrawProfiles(program_state *ProgramState) {
    int Y = 400;
    v2 CharDim = GetCharDim(ProgramState);
    int CharHeight = CharDim.y;
    for(int i = 0; i < ProfileNames.Count; i++)
    {
        double Total = 0;
        for(int a = 0; a < ProfileCycleFrameCount; a++)
        {
            Total += ProfileResultFrames[i][a];
        }
        string ProfileString = String("%S: %f", ProfileNames[i], Total/ProfileCycleFrameCount);
        DrawString(ProgramState, ProfileString, V2(400, 200+CharHeight*i), ProgramState->FontSize, BLACK, RED);
        FreeString(ProfileString);
    }
}

void
DrawView(program_state *ProgramState, view *View)
{
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    b32 ViewIsSelected = View == &ProgramState->Views[ProgramState->SelectedViewIndex];
    rect ViewRect = View->Rect;
    rect TextRect = View->TextRect;
    
    BeginScissorMode(ViewRect.x, ViewRect.y, ViewRect.w, ViewRect.h);
    
    // draw background
    DrawRectangleRec(R(View->Rect), ProgramState->BGColor);
    
    // draw line numbers
    rect LineNumbersRect = Rect(ViewRect.x, View->TextRect.y, 4*CharWidth, View->TextRect.h);
    DrawRectangleRec(R(LineNumbersRect), ProgramState->LineNumberBGColor);
    
    // draw title bar
    // TODO: proper colors
    
    if(View->ListerIsOpen)
    {
        lister *Lister = &View->Lister;
        DrawRectangle(ViewRect.x, ViewRect.y, ViewRect.w, CharHeight, BLUE);
        int InputX = DrawString(ProgramState, Lister->InputLabel, V2(ViewRect.x, ViewRect.y),
                                ProgramState->FontSize, WHITE);
        DrawString(ProgramState, Lister->Input, V2(InputX, ViewRect.y),
                   ProgramState->FontSize, WHITE);
    }
    else
    {
        DrawRectangle(ViewRect.x, ViewRect.y, ViewRect.w, CharHeight, GRAY);
        string TitleString = String("%S   %d,%d", View->Buffer->FileName, View->CursorPos.l, View->CursorPos.c);
        DrawString(ProgramState, TitleString, V2(ViewRect.x, ViewRect.y), ProgramState->FontSize, BLACK);
        FreeString(TitleString);
    }
    
    
    // draw cursor
    rect CursorDrawRect = CharToScreenSpace(View, View->CursorRect);
    if(ViewIsSelected)
    {
        if(ProgramState->InputMode == InputMode_Insert)
        {
            DrawRectangleRec(R(Rect(CursorDrawRect.x, CursorDrawRect.y,
                                    CursorDrawRect.w/3, CursorDrawRect.h)),
                             ProgramState->CursorBGColor);
        }
        else
            DrawRectangleRec(R(CursorDrawRect), ProgramState->CursorBGColor);
    }
    else
    {
        DrawRectangleLinesEx(R(CursorDrawRect), 2, ProgramState->CursorBGColor);
    }
    
    // Draw text
    EndScissorMode();
    
    for(int l = 0; l < LineCount(View); l++)
    {
        line_data LineData = LineDataAt(View, l);
        int LineY = LineData.LineRect.y;
        if(LineY - View->Y > View->TextRect.y + View->TextRect.h)
            break;
        if(LineY + CharHeight*2 - View->Y < View->TextRect.y)
            continue;
        
        BeginScissorMode(LineNumbersRect.x, LineNumbersRect.y,
                         LineNumbersRect.w, LineNumbersRect.h);
        string NumberString = String("%d", l);
        v2 LineNumberPos = V2(View->Rect.x, LineData.LineRect.y - View->Y + View->TextRect.y);
        DrawString(ProgramState, NumberString, LineNumberPos, ProgramState->FontSize, ProgramState->LineNumberFGColor);
        FreeString(NumberString);
        EndScissorMode();
        
        BeginScissorMode(TextRect.x, TextRect.y, TextRect.w, TextRect.h);
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
        EndScissorMode();
    }
    
    if(View->ListerIsOpen) {
        lister *Lister = &View->Lister;
        
        DrawRectangle(ViewRect.x, TextRect.y, ViewRect.w, Lister->MatchingEntries.Count*CharHeight, GRAY);
        
        int Y = TextRect.y;
        for(int i = 0; i < Lister->MatchingEntries.Count; i++) {
            Color BGColor = GRAY;
            Color FGColor = WHITE;
            
            if(Lister->HoverIndex == i)
                BGColor = LIGHTGRAY;
            if(Lister->SelectedIndex == i)
            {
                BGColor = WHITE;
                FGColor = RED;
            }
            rect EntryRect = Lister->Rects[i];
            
            DrawRectangleRec(R(EntryRect), BGColor);
            DrawString(ProgramState, Lister->Entries[i].Name,
                       V2(EntryRect.x, EntryRect.y), ProgramState->FontSize, FGColor);
            Y += CharHeight;
        }
    }
    
    
    // TODO: redraw chars drawn over by cursor 
    // invert color and draw with scissor set to cursor rect
    
    BeginScissorMode(ViewRect.x, ViewRect.y, ViewRect.w, ViewRect.h);
    if(ProgramState->ShowViewRects)
    {
        DrawRectangleLinesEx(R(View->TextRect), 2, {159, 192, 123, 255});
    }
    
    if(ProgramState->ShowViewInfo)
    {
        int InfoFontSize = 20;
        int KeyValueDistanceChars = 10;
        int InfoCharHeight = GetCharDim(ProgramState).y;
        int InfoCharWidth = GetCharDim(ProgramState).x;
        int KeyValueDistance = KeyValueDistanceChars * InfoCharWidth;
        
        DrawRectangleRec(R(View->Rect), {0, 0, 0, 200});
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
    }
    
    if(ProgramState->ShowViewRects)
    {
        DrawRectangleLinesEx(R(View->Rect), 2, {216, 50, 10, 255});
    }
    
    EndScissorMode();
}


void
LoadFileToBuffer(buffer *Buffer, const char *Path)
{
    // TODO: check if file exists and is readable, etc
    u32 FileSize = (u32)GetFileLength(Path);
    char *FileData = LoadFileText(Path);
    
    Buffer->FileName = String(Path);
    
    Buffer->Lines = StringList();
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
}

void
CloseLister(program_state *ProgramState, view *View)
{
    lister *Lister = &View->Lister;
    ProgramState->InputMode = InputMode_Nav;
    
    View->ListerIsOpen = false;
    ListFree(&Lister->Rects);
    ListFree(&Lister->Entries);
    ListFree(&Lister->MatchingEntries);
    FreeString(Lister->Input);
    FreeString(Lister->InputLabel);
}

void
OpenEditFileLister(program_state *ProgramState, view *View)
{
    View->Lister = Lister(ListerType_String);
    lister *Lister = &View->Lister;
    
    FilePathList FilesInDir = LoadDirectoryFiles("./");
    
    for(int i = 0; i < FilesInDir.count; i++)
    {
        lister_entry NewEntry;
        NewEntry.String = String(FilesInDir.paths[i]);
        NewEntry.Name = NewEntry.String;
        ListAdd(&Lister->Entries, NewEntry);
        ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
        ListAdd(&Lister->MatchingEntries, i);
    }
    
    UnloadDirectoryFiles(FilesInDir);
    View->ListerIsOpen = true;
}

#include "festival_editing.h"

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
        font *FontMain = &ProgramState->FontMain;
        //Font *FontSDF = &ProgramState->FontSDF;
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            ProgramState->ShouldExit = false;
            
            for(int i = 0; i < MAX_BUFFERS; i++) {
                Buffers[i] = {0};
            };
            
            // TEXT FILE
            LoadFileToBuffer(&Buffers[0], "test.cpp");
            
            ProgramState->FontSize = 22;
            LoadFont(ProgramState, ProgramState->FontSize);
            ProgramState->KeyFirstRepeatTime = 0.4f;
            ProgramState->KeyRepeatSpeed = 0.02f;
            
            FillKeyData(ProgramState);
            
            ProgramState->PrevFontSize = ProgramState->FontSize;
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
            
            DefineProfile(FillLineData);
            DefineProfile(Rendering);
            DefineProfile(Total);
            
            //LoadAllCharTextures(ProgramState);
        }
        
        // Re-render font chars if needed
        if(ProgramState->PrevFontSize != ProgramState->FontSize)
            LoadFont(ProgramState, ProgramState->FontSize);
        //LoadAllCharTextures(ProgramState);
        ProgramState->PrevFontSize = ProgramState->FontSize;
        
        StartProfiling();
        StartProfile(Total);
        
        view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
        
        if(View->ListerIsOpen && View->Lister.ShouldExecute)
        {
            // TODO: Execute lister
        }
        
        UpdateKeyInput(ProgramState);
        
        ProgramState->ScreenWidth = Memory->WindowWidth;
        ProgramState->ScreenHeight = Memory->WindowHeight;
        
        
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
        
        HandleInput(ProgramState);
        
        
        // lister set matchingentries
        // TODO: only do this when Input has changed
        // TODO: better string matching
        for(int a = 0; a < Views->Count; a++)
        {
            view *View = &Views->Data[a];
            if(View->ListerIsOpen)
            {
                lister *Lister = &View->Lister;
                ListFree(&Lister->MatchingEntries);
                Lister->MatchingEntries = IntList();
                
                for(int i = 0; i < Lister->Entries.Count; i++)
                {
                    b32 Matches = true;
                    for(int j = 0; j < Lister->Input.Length && j < Lister->Entries[i].Name.Length; i++)
                    {
                        if(Lister->Input[j] != Lister->Entries[i].Name[j])
                        {
                            Matches = false;
                            break;
                        }
                    }
                    if(Matches)
                        ListAdd(&Lister->MatchingEntries, i);
                }
            }
        }
        
        v2 MousePos = V2(GetMousePosition());
        if(IsMouseButtonDown(0))
        {
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
        
        for(int i = 0; i < Views->Count; i++)
        {
            view *View = &Views->Data[i];
            if(View->ListerIsOpen)
            {
                lister *Lister = &View->Lister;
                int Y = View->TextRect.y;
                ListFree(&Lister->Rects);
                Lister->Rects = RectList();//Lister->MatchingEntries.Count);
                for(int i = 0; i < Lister->MatchingEntries.Count; i++)
                {
                    /*Lister->Rects[i] = */ListAdd(&Lister->Rects, Rect(View->Rect.x, Y, View->Rect.w, CharHeight));
                    
                    Y += CharHeight;
                    if(CheckCollisionPointRec(V(MousePos), R(Lister->Rects[i])))
                    {
                        Lister->HoverIndex = i;
                        if(IsMouseButtonPressed(0))
                        {
                            Lister->SelectedIndex = i;
                            Lister->ShouldExecute = true;
                        }
                    }
                }
            }
        }
        
        
        if(IsAnyControlKeyDown)
        {
            ProgramState->FontSize += GetMouseWheelMove();
            if(ProgramState->FontSize < 6) ProgramState->FontSize = 6;
            if(ProgramState->FontSize > 100) ProgramState->FontSize = 100;
        }
        
        
#if 0
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
#endif
    
    if(IsMouseButtonDown(0))
    {
        v2 MousePos = V2(GetMousePosition());
        
        buffer_pos MouseBufferPos = ClosestBufferPos(View, ScreenToCharSpace(View, MousePos));
        View->CursorPos = MouseBufferPos;
        View->IdealCursorCol = View->CursorPos.c;
    }
    
    
    
    
    
    Clamp(View->CursorPos.l, 0, LineCount(View)-1);
    Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
    
    AdjustView(ProgramState, View);
    
    
    
    StartProfile(FillLineData);
    for(int i = 0; i < Views->Count; i++)
    {
        view *View = &Views->Data[i];
        FillLineData(View, ProgramState);
        View->CursorTargetRect = CharRectAt(View, View->CursorPos.l, View->CursorPos.c);
        View->Y = Interpolate(View->Y, View->TargetY, 0.4f);
        View->CursorRect = Interpolate(View->CursorRect, View->CursorTargetRect, 0.5f);
    }
    EndProfile(FillLineData);
    
    StartProfile(Rendering);
    BeginDrawing();
    for(int i = 0; i < Views->Count; i++)
    {
        view *View = &Views->Data[i];
        DrawView(ProgramState, View);
    }
    
    //DrawFPS(400, 100);
    //DrawProfiles(ProgramState);
    
    EndDrawing();
    EndProfile(Rendering);
    
    EndProfile(Total);
    
    
}
}