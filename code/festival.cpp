#include <stdio.h>

#include "raylib.h"

#include "festival_platform.h"

#include "festival_math.h"
#include "festival_lists.h"
#include "festival_string.h"
#include "festival_filesystem.h"
#include "festival_undo.h"
#include "festival.h"

#include "festival_functions.cpp"
#include "festival_profiling.h"
#include "festival_commands.h"

void
UndoAction(buffer *Buffer, action A)
{
}

void
RedoAction(buffer *Buffer, action A)
{
    if(A.Delete)
    {
        buffer_pos Start = A.DeleteStart;
        buffer_pos End = A.DeleteEnd;
        Print("Deleting from %d,%d to %d,%d", Start.l, Start.c, End.l, End.c);
        
        // delete in-between lines
        for(int i = Start.l + 1; i < End.l; i++)
        {
            ListRemoveAt(&Buffer->Lines, i);
            End.l--;
            i--;
        }
        
        // slice start line
        Buffer->Lines[Start.l].RemoveRange(Start.c, Buffer->Lines[Start.l].Length);
        
        // slice and join next line
        if(End.l > Start.l)
        {
            Buffer->Lines[End.l].RemoveRange(0, End.c);
            Buffer->Lines[Start.l].AppendString(Buffer->Lines[End.l]);
            ListRemoveAt(&Buffer->Lines, End.l);
        }
    }
}


rect
DrawChar(program_state *ProgramState, int Char, v2 Pos, color BGColor, color FGColor)
{
    font *Font = &ProgramState->FontMonospace;
    if(ProgramState->FontType == FontType_Sans)
        Font = &ProgramState->FontSans;
    else if(ProgramState->FontType == FontType_Serif)
        Font = &ProgramState->FontSerif;
    
    GlyphInfo Info = {0};
    int GlyphIndex = CharIndex(Font, Char);
    Info = Font->RFont.glyphs[GlyphIndex];
    
    
    
    rect DestRect = Rect(Pos.x + Info.offsetX, Pos.y + Info.offsetY,
                         Font->RFont.recs[GlyphIndex].width,
                         Font->RFont.recs[GlyphIndex].height);
    
    if(BGColor.a != 0)
    {
        rect BGRect = Rect(Pos.x, Pos.y,
                           ProgramState->FontSize/2,
                           ProgramState->FontSize);
        //if(Font->RFont.recs[GlyphIndex].width > ProgramState->FontSize/2)
        if(Info.advanceX > ProgramState->FontSize/2)
            BGRect.w = Info.advanceX;
        DrawRectangleRec(R(BGRect), BGColor);
    }
    
    DrawTexturePro(Font->RFont.texture,
                   Font->RFont.recs[GlyphIndex],
                   R(DestRect),
                   {0, 0}, 0, FGColor);
    return DestRect;
}

void
DrawChar(program_state *ProgramState, int Char, v2 Pos, color FGColor)
{
    DrawChar(ProgramState, Char, Pos, RGBA(0, 0, 0, 0), FGColor);
}

#define Style_Underline 1
#define Style_Overline 2

v2
DrawString(program_state *ProgramState, string String, v2 Pos, color BGColor, color FGColor, int Style)
{
    font *Font = &ProgramState->FontMonospace;
    if(ProgramState->FontType == FontType_Sans)
        Font = &ProgramState->FontSans;
    else if(ProgramState->FontType == FontType_Serif)
        Font = &ProgramState->FontSerif;
    
    v2 OriginalPos = Pos;
    
    for(int i = 0; i < String.Length; i++)
    {
        DrawChar(ProgramState, String[i], Pos, BGColor, FGColor);
        
        int GlyphIndex = CharIndex(Font, String[i]);
        GlyphInfo Info = Font->RFont.glyphs[GlyphIndex];
        Pos.x += Info.advanceX;
    }
    if(Style & Style_Underline)
    {
        v2 YDiff = V2(0, ProgramState->FontSize - 3);
        DrawLineEx(V(OriginalPos + YDiff), V(Pos + YDiff), 2, FGColor);
    }
    return V2(Pos.x - OriginalPos.x, Pos.y + ProgramState->FontSize - OriginalPos.y);
}

v2
DrawString(program_state *ProgramState, string String, v2 Pos, color FGColor, int Style)
{
    return DrawString(ProgramState, String, Pos, RGBA(0, 0, 0, 0), FGColor, Style);
}

v2
DrawString(program_state *ProgramState, string String, v2 Pos, color FGColor)
{
    return DrawString(ProgramState, String, Pos, RGBA(0, 0, 0, 0), FGColor, 0);
}

v2
DrawString(program_state *ProgramState, string String, v2 Pos, color BGColor, color FGColor)
{
    return DrawString(ProgramState, String, Pos, BGColor, FGColor, 0);
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
        DrawString(ProgramState, ProfileString, V2(400, 200+CharHeight*i), BLACK, RED);
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
        int InputX = DrawString(ProgramState, Lister->InputLabel, V2(ViewRect.x, ViewRect.y), WHITE).x;
        DrawString(ProgramState, Lister->Input, V2(InputX, ViewRect.y),
                   WHITE);
    }
    else
    {
        DrawRectangle(ViewRect.x, ViewRect.y, ViewRect.w, CharHeight, GRAY);
        string TitleString = String("%S   %d,%d", View->Buffer->FileName, View->CursorPos.l, View->CursorPos.c);
        DrawString(ProgramState, TitleString, V2(ViewRect.x, ViewRect.y), BLACK);
        FreeString(TitleString);
    }
    
    
    BeginScissorMode(TextRect.x, TextRect.y, TextRect.w, TextRect.h);
    // TODO: Cross-view cursor drawing
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
        {
            font *Font = &ProgramState->FontMonospace;
            if(ProgramState->FontType == FontType_Sans)
                Font = &ProgramState->FontSans;
            else if(ProgramState->FontType == FontType_Serif)
                Font = &ProgramState->FontSerif;
            
            GlyphInfo Info = {0};
            int GlyphIndex = CharIndex(Font, CharAt(View, View->CursorPos));
            Info = Font->RFont.glyphs[GlyphIndex];
            
            CursorDrawRect.w = Info.advanceX;
            if(View->Selecting)
            {
                DrawRectangleRec(R(CursorDrawRect), RED);
            }
            else
            {
                DrawRectangleRec(R(CursorDrawRect), ProgramState->CursorBGColor);
            }
        }
    }
    else
    {
        DrawRectangleLinesEx(R(CursorDrawRect), 2, ProgramState->CursorBGColor);
    }
    EndScissorMode();
    
    // Draw text
    EndScissorMode();
    
    for(int l = 0; l < LineCount(View); l++)
    {
        line_data LineData = LineDataAt(View, l);
        int LineY = LineData.LineRect.y;
        if(LineY - View->Y > View->TextRect.y + View->TextRect.h)
            break;
        if(LineRect(View, l).y + LineRect(View, l).h - View->Y < View->TextRect.y)
            continue;
        
        BeginScissorMode(LineNumbersRect.x, LineNumbersRect.y,
                         LineNumbersRect.w, LineNumbersRect.h);
        string NumberString = String("%d", l);
        v2 LineNumberPos = V2(View->Rect.x, LineData.LineRect.y - View->Y + View->TextRect.y);
        DrawString(ProgramState, NumberString, LineNumberPos, ProgramState->LineNumberFGColor);
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
            
            DrawChar(ProgramState, CharAt(View, l, c), V2(Rect), CharColor);
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
            DrawString(ProgramState, Lister->Entries[Lister->MatchingEntries[i]].Name,
                       V2(EntryRect.x, EntryRect.y), FGColor);
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
        int KeyValueDistanceChars = 10;
        int InfoCharHeight = GetCharDim(ProgramState).y;
        int InfoCharWidth = GetCharDim(ProgramState).x;
        int KeyValueDistance = KeyValueDistanceChars * InfoCharWidth;
        
        DrawRectangleRec(R(View->Rect), {0, 0, 0, 200});
        {
            v2 TextPos = V2(View->Rect.x + 10, View->Rect.y + 10);
            DrawString(ProgramState, String("id"), TextPos, WHITE);
            DrawString(ProgramState, String("%d", View->Id), TextPos + V2(KeyValueDistance, 0), YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(ProgramState, String("parent"), TextPos, WHITE);
            DrawString(ProgramState, String("%d", View->ParentId), TextPos + V2(KeyValueDistance, 0), YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(ProgramState, String("birth #"), TextPos, WHITE);
            DrawString(ProgramState, String("%d", View->BirthOrdinal), TextPos + V2(KeyValueDistance, 0), YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(ProgramState, String("area"), TextPos, WHITE);
            DrawString(ProgramState, String("%d", View->Area), TextPos + V2(KeyValueDistance, 0), YELLOW);
        }
    }
    
    if(ProgramState->ShowViewRects)
    {
        DrawRectangleLinesEx(R(View->Rect), 2, {216, 50, 10, 255});
    }
    
    EndScissorMode();
}


buffer
LoadFileToBuffer(const char *RawPath)
{
    if(!RawPath || RawPath[0] == 0)
        return {};
    
    string FileName = String(RawPath);
    CleanUpPath(&FileName);
    string Path = String(RawPath);
    AbsolutizePath(&Path);
    
    char *FullRawPath = RawString(Path);
    
    // TODO: check if file exists and is readable, etc
    if(!FileExists(FullRawPath))
        Print("ERROR: no such file exists!");
    
    u32 FileSize = (u32)GetFileLength(FullRawPath);
    Print("File size: %d", (int)FileSize);
    
    char *FileData = LoadFileText(FullRawPath);
    
    free(FullRawPath);
    
    buffer Buffer = {};
    Buffer.FileName = FileName;
    Buffer.DirPath = GetDirOfFile(Path);
    Buffer.ActionStack = ActionList();
    
    Buffer.Lines = StringList();
    printf("Gathering text\n");
    for(int i = 0; i < FileSize; i++)
    {
        int LineStart = i;
        // TODO: need to add AppendString to easily do this
        for(; FileData[i] != '\n' && i < FileSize; i++) {}
        
        ListAdd(&(Buffer.Lines), AllocString(i-LineStart));
        
        int InLine = 0;
        for(int a = LineStart; a < i; a++)
        {
            Buffer.Lines[Buffer.Lines.Count-1].Data[InLine] = FileData[a];
            InLine++;
        }
    }
    if(Buffer.Lines.Count == 0)
    {
        ListAdd(&Buffer.Lines, String(""));
    }
    printf("Finished with text\n");
    
    UnloadFileText(FileData);
    
    return Buffer;
}

buffer *
GetBufferForPath(program_state *ProgramState, string PathString)
{
    // see if we already have a buffer
    
    string PathCopy = CopyString(PathString);
    AbsolutizePath(&PathCopy);
    int Index = -1;
    
    for(int i = 0; i < ProgramState->Buffers.Count; i++)
    {
        if(PathCopy == ProgramState->Buffers[i].DirPath)
        {
            Index = i;
            break;
        }
    }
    
    FreeString(PathCopy);
    
    if(Index != -1)
        return &ProgramState->Buffers[Index];
    return 0;
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
OpenEditFileLister(program_state *ProgramState, view *View, const char *Directory)
{
    View->Lister = Lister(ListerType_String);
    lister *Lister = &View->Lister;
    
    Lister->InputLabel = String("Open file: ");
    Lister->Input = String(Directory);
    AbsolutizePath(&Lister->Input);
    Lister->Purpose = ListerPurpose_EditFile;
    
    FilePathList FilesInDir = LoadDirectoryFiles(Directory);
    for(int i = 0; i < FilesInDir.count; i++)
    {
        b32 IsDirectory = false;
        
        lister_entry NewEntry;
        NewEntry.String = String(FilesInDir.paths[i]);
        AbsolutizePath(&NewEntry.String);
        if(DirectoryExists(TempRawString(NewEntry.String)))
        {
            IsDirectory = true;
            NewEntry.String.AppendChar('/');
        }
        
        string Name = TempString(FilesInDir.paths[i]);
        CleanUpPath(&Name);
        NewEntry.Name = GetFileName(Name);
        if(IsDirectory)
            NewEntry.Name.AppendChar('/');
        
        
        ListAdd(&Lister->Entries, NewEntry);
        ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
        ListAdd(&Lister->MatchingEntries, i);
    }
    UnloadDirectoryFiles(FilesInDir);
    
    View->ListerIsOpen = true;
    Lister->ShouldExecute = false;
}

// TODO: policy: all strings used in lister are uniquely allocated and freed upon closure

void
OpenSwitchBufferLister(program_state *ProgramState, view *View)
{
    View->Lister = Lister(ListerType_BufferPointer);
    lister *Lister = &View->Lister;
    
    Lister->InputLabel = String("Switch to Buffer: ");
    Lister->Input = String("");
    Lister->Purpose = ListerPurpose_SwitchBuffer;
    
    for(int i = 0; i < ProgramState->Buffers.Count; i++)
    {
        lister_entry NewEntry;
        NewEntry.Buffer = &ProgramState->Buffers[i];
        NewEntry.Name = CopyString(ProgramState->Buffers[i].DirPath);
        ListAdd(&Lister->Entries, NewEntry);
        ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
        ListAdd(&Lister->MatchingEntries, i);
    }
    
    View->ListerIsOpen = true;
    Lister->ShouldExecute = false;
}

void
OpenCommandLister(program_state *ProgramState, view *View)
{
    View->Lister = Lister(ListerType_Command);
    lister *Lister = &View->Lister;
    
    Lister->InputLabel = String("Run Command: ");
    Lister->Input = String("");
    Lister->Purpose = ListerPurpose_RunCommand;
    
    for(int i = 0; i < sizeof(Commands) / sizeof(command); i++)
    {
        lister_entry NewEntry;
        NewEntry.Command = Commands[i];
        NewEntry.Name = String(Commands[i].Name);
        ListAdd(&Lister->Entries, NewEntry);
        ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
        ListAdd(&Lister->MatchingEntries, i);
    }
    
    View->ListerIsOpen = true;
    Lister->ShouldExecute = false;
}

void
OpenSwitchFontTypeLister(program_state *ProgramState, view *View)
{
    View->Lister = Lister(ListerType_FontType);
    lister *Lister = &View->Lister;
    
    Lister->InputLabel = String("Run Command: ");
    Lister->Input = String("");
    Lister->Purpose = ListerPurpose_SwitchFontType;
    
    lister_entry NewEntry;
    NewEntry.FontType = FontType_Monospace;
    NewEntry.Name = String("Monospace");
    ListAdd(&Lister->Entries, NewEntry);
    ListAdd(&Lister->MatchingEntries, 0);
    
    NewEntry.FontType = FontType_Sans;
    NewEntry.Name = String("Sans");
    ListAdd(&Lister->Entries, NewEntry);
    ListAdd(&Lister->MatchingEntries, 1);
    
    NewEntry.FontType = FontType_Serif;
    NewEntry.Name = String("Serif");
    ListAdd(&Lister->Entries, NewEntry);
    ListAdd(&Lister->MatchingEntries, 2);
    
    ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
    ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
    ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
    
    View->ListerIsOpen = true;
    Lister->ShouldExecute = false;
}


void
ExecLister(program_state *ProgramState, view *View)
{
    Print("Exec lister!");
    lister *Lister = &View->Lister;
    switch(Lister->Purpose)
    {
        case ListerPurpose_EditFile: {
            Print("Purpose: Edit file");
            if(Lister->Type != ListerType_String) {
                Print("Lister is for Edit File but type isn't String!");
                break;
            }
            
            if(Lister->MatchingEntries.Count == 0)
            {
                // Create buffer for non-existant file
                buffer NewBuffer;
                NewBuffer.FileName = CopyString(Lister->Input);
                NewBuffer.DirPath = CopyString(Lister->Input);
                NewBuffer.Lines = StringList();
                ListAdd(&NewBuffer.Lines, String(""));
                ListAdd(&ProgramState->Buffers, NewBuffer);
                View->Buffer = &ProgramState->Buffers[ProgramState->Buffers.Count - 1];
            }
            else
            {
                // If selection is a directory, we'll open a new lister for opening files in that
                // directory, otherwise open the file
                
                string Selection = CopyString(Lister->Entries[Lister->MatchingEntries[Lister->SelectedIndex]].String);
                
                if(DirectoryExists(TempRawString(Selection)))
                {
                    Print("Switching to directory");
                    // We've selected a directory, so make a new lister for opening files in it
                    
                    Lister->ShouldExecute = false;
                    CloseLister(ProgramState, View);
                    
                    OpenEditFileLister(ProgramState, View, TempRawString(Selection));
                    
                    return;
                }
                else
                {
                    // Open file
                    char *RawFileName = TempRawString(Selection);
                    buffer *ExistingBuffer = GetBufferForPath(ProgramState, Selection);
                    
                    if(ExistingBuffer)
                    {
                        View->Buffer = ExistingBuffer;
                    }
                    else
                    {
                        ListAdd(&ProgramState->Buffers, LoadFileToBuffer(RawFileName));
                        View->Buffer = &ProgramState->Buffers[ProgramState->Buffers.Count - 1];
                    }
                }
            }
        } break;
        case ListerPurpose_SwitchBuffer: {
            Print("Purpose: Switch buffer");
            if(Lister->Type != ListerType_BufferPointer) {
                Print("Lister is for Switch buffer but type isn't Buffer Pointer!");
                break;
            }
            
            View->Buffer = Lister->Entries[Lister->MatchingEntries[Lister->SelectedIndex]].Buffer;
            
        } break;
        case ListerPurpose_SwitchFontType: {
            if(Lister->Type != ListerType_FontType) {
                Print("Lister is for Switch font type but type isn't FontType!");
                break;
            }
            
            ProgramState->FontType = Lister->Entries[Lister->MatchingEntries[Lister->SelectedIndex]].FontType;
            
        } break;
        case ListerPurpose_RunCommand: {
            if(Lister->Type != ListerType_Command) {
                Print("Lister is for Switch font type but type isn't FontType!");
                break;
            }
            Lister->ShouldExecute = false;
            command CommandToExec = Lister->Entries[Lister->MatchingEntries[Lister->SelectedIndex]].Command;
            CloseLister(ProgramState, View);
            
            CommandToExec.Function(ProgramState, View);
            
            return;
            
        } break;
        default: {
            Print("Unknown lister purpose!");
        }
    }
    
    Lister->ShouldExecute = false;
    CloseLister(ProgramState, View);
}


DefineCommand(SwitchFontType)
{
    OpenSwitchFontTypeLister(ProgramState, View);
    return 0;
}


DefineCommand(EditFile)
{
    OpenEditFileLister(ProgramState, View, "./");
    return 0;
}

void
DrawSuperDebugMenu(program_state *ProgramState)
{
    DrawRectangleRec(R(Rect(0, 0, ProgramState->ScreenWidth, ProgramState->ScreenHeight)), ORANGE);
    int Y = -ProgramState->SuperDebugMenuY;
    int X = 0;
    
    Y += DrawString(ProgramState, TempString("Super Debug Menu"), V2(X, Y), BLACK).y;
    Y += ProgramState->FontSize;
    Y += DrawString(ProgramState, TempString("Buffers"), V2(X, Y), BLACK, Style_Underline).y;
    for(int i = 0; i < ProgramState->Buffers.Count; i++)
    {
        Y += DrawString(ProgramState, ProgramState->Buffers[i].FileName, V2(X, Y), BLACK).y;
        Y += DrawString(ProgramState, ProgramState->Buffers[i].DirPath, V2(X + 20, Y), BLACK).y;
    }
    
#if 0
    Y += DrawString(ProgramState, String, v2 Pos, color FGColor).y;
    Y += DrawString(ProgramState, String, v2 Pos, color FGColor).y;
#endif
}

#include "festival_editing.h"

extern "C"
{
    PROGRAM_UPDATE_AND_RENDER(ProgramUpdateAndRender)
    {
        program_state *ProgramState = (program_state *)Memory->Data;
        ProgramState->ScreenWidth = Memory->WindowWidth;
        ProgramState->ScreenHeight = Memory->WindowHeight;
        buffer_list *Buffers = &ProgramState->Buffers;
        view_list *Views = &ProgramState->Views;
        buffer *Buffer = &ProgramState->Buffers[0];
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            ProgramState->ShouldExit = false;
            
            for(int i = 0; i < MAX_BUFFERS; i++) {
                Buffers[i] = {0};
            };
            
            // TEXT FILE
            ProgramState->Buffers = BufferList();
            ListAdd(Buffers, LoadFileToBuffer("./test.cpp"));
            
            
            ProgramState->FontSize = 18;
            LoadFonts(ProgramState);
            ProgramState->FontType = FontType_Monospace;
            
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
            ListAdd(Views, View(ProgramState, &Buffers->Data[0], -1, Location_Below));
            
            ProgramState->SelectedViewIndex = 0;
            
            ProgramState->ShowViewInfo = false;
            ProgramState->ShowViewRects = false;
            ProgramState->ShowSuperDebugMenu = false;
            ProgramState->SuperDebugMenuY = 0;
            
            FillLineData(&ProgramState->Views[0], ProgramState);
            
            DefineProfile(FillLineData);
            DefineProfile(Rendering);
            DefineProfile(Total);
            
            TempStrings = StringList();
        }
        
        PurgeTempStrings();
        
        for(int i = 0; i < Views->Count; i++)
        {
            view *View = &ProgramState->Views[i];
            if(View->ListerIsOpen && View->Lister.ShouldExecute)
                ExecLister(ProgramState, View);
        }
        
        
        // Re-load fonts if needed
        if(ProgramState->PrevFontSize != ProgramState->FontSize)
            LoadFonts(ProgramState);
        ProgramState->PrevFontSize = ProgramState->FontSize;
        
        StartProfiling();
        StartProfile(Total);
        
        view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
        
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
            lister *Lister = &View->Lister;
            
            if(View->ListerIsOpen)
            {
                if(Lister->Purpose == ListerPurpose_EditFile)
                {
                    int SelectedIndex = Lister->SelectedIndex;
                    b32 ShouldExecute = Lister->ShouldExecute;
                    string FileName = GetFileName(Lister->Input);
                    string DirToSearch = GetDirOfFile(Lister->Input);
                    
                    char *NewPath = TempRawString(DirToSearch);
                    
                    CloseLister(ProgramState, View);
                    OpenEditFileLister(ProgramState, View, NewPath);
                    
                    Lister->Input.AppendString(FileName);
                    Lister->SelectedIndex = SelectedIndex;
                    Lister->ShouldExecute = ShouldExecute;
                    
                    FreeString(FileName);
                    FreeString(DirToSearch);
                }
                
                
                ListFree(&Lister->MatchingEntries);
                Lister->MatchingEntries = IntList();
                
                for(int i = 0; i < Lister->Entries.Count; i++)
                {
                    string StringToCheck = Lister->Entries[i].Name;
                    if(Lister->Purpose == ListerPurpose_EditFile)
                        StringToCheck = Lister->Entries[i].String;
                    
                    b32 Matches = true;
                    for(int j = 0; j < Lister->Input.Length && j < StringToCheck.Length; j++)
                    {
                        if(Lister->Input[j] != StringToCheck[j])
                        {
                            Matches = false;
                            break;
                        }
                    }
                    if(Matches)
                        ListAdd(&Lister->MatchingEntries, i);
                }
            }
            
            Clamp(View->Lister.SelectedIndex, 0, View->Lister.MatchingEntries.Count - 1);
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
        
        
        if(IsMouseButtonDown(0))
        {
            v2 MousePos = V2(GetMousePosition());
            
            buffer_pos MouseBufferPos = ClosestBufferPos(View, ScreenToCharSpace(View, MousePos));
            View->CursorPos = MouseBufferPos;
            View->IdealCursorCol = View->CursorPos.c;
        }
        
        
        Clamp(View->CursorPos.l, 0, LineCount(View)-1);
        Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
        
        
        StartProfile(FillLineData);
        for(int i = 0; i < Views->Count; i++)
        {
            view *View = &Views->Data[i];
            FillLineData(View, ProgramState);
            
            View->CursorTargetRect = CharRectAt(View, View->CursorPos.l, View->CursorPos.c);
            View->CursorRect = Interpolate(View->CursorRect, View->CursorTargetRect, 0.5f);
            
            AdjustView(ProgramState, View);
            View->Y = Interpolate(View->Y, View->TargetY, 0.4f);
        }
        EndProfile(FillLineData);
        
        StartProfile(Rendering);
        BeginDrawing();
        for(int i = 0; i < Views->Count; i++)
        {
            view *View = &Views->Data[i];
            DrawView(ProgramState, View);
        }
        
#if 0
        int Y = 300;
        for(int i = 0; i < ProgramState->Buffers.Count; i++)
        {
            string Str = String("%d: %S", i, ProgramState->Buffers[i].DirPath);
            color BGColor = BLACK;
            if(ProgramState->Views[ProgramState->SelectedViewIndex].Buffer == &ProgramState->Buffers[i])
                BGColor = GRAY;
            DrawString(ProgramState, Str, V2(100, Y), BGColor, ORANGE);
            Y+=20;
            FreeString(Str);
        }
#endif
        
        if(ProgramState->ShowSuperDebugMenu)
            DrawSuperDebugMenu(ProgramState);
        
        //DrawFPS(400, 100);
        //DrawProfiles(ProgramState);
        
        EndDrawing();
        EndProfile(Rendering);
        
        EndProfile(Total);
        
        
    }
}