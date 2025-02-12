#include "raylib.h"
#include "chardet.h"
#include <sys/stat.h>
#include <iconv.h>
#include <errno.h>
#include <wchar.h>

#include "festival_base.h"
#include "festival_platform.h"

#include "festival_math.h"
#include "festival_lists.h"
#include "festival_string.h"
#include "festival_input.h"

#include "festival_font.h"
#include "festival_settings.h"
#include "festival_font.cpp"
#include "festival_actions.h"
#include "festival_encoding.h"
#include "festival_encoding.cpp"
#include "festival_commands.h"
#include "festival_buffer.h"
#include "festival_lister.h"
#include "festival_view.h"
#include "festival.h"
#include "festival_filesystem.cpp"
#include "festival_buffer.cpp"
#include "festival_view.cpp"

#include "festival_functions.cpp"
#include "festival_profiling.h"
#include "festival_lister.cpp"
#include "festival_actions.cpp"
#include "festival_commands.cpp"
#include "festival_drawing.cpp"
#include "festival_editing.h"


extern "C"
{
    PROGRAM_UPDATE_AND_RENDER(ProgramUpdateAndRender)
    {
        program_state *ProgramState = (program_state *)Memory->Data;
        ProgramState->ScreenWidth = Memory->WindowWidth;
        ProgramState->ScreenHeight = Memory->WindowHeight;
        buffer_list *Buffers = &(ProgramState->Buffers);
        view_list *Views = &(ProgramState->Views);
        buffer *Buffer = &(ProgramState->Buffers[0]);
        settings *Settings = &(ProgramState->Settings);
        font *Font = &(Settings->Font);
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            ProgramState->ShouldExit = false;
            
            u32 LargestSize = 0;
            const char *LargestName = "None";
            for(int i = 0; i < sizeof(UnicodeGroups) / sizeof(unicode_group); i++)
            {
                u32 Size = UnicodeGroups[i].End - UnicodeGroups[i].Start;
                if(Size > LargestSize)
                {
                    LargestSize = Size;
                    LargestName = UnicodeGroups[i].Name;
                }
            }
            Print("Largest Unicode group is %s, with %D items", LargestName, LargestSize);
            
            for(int i = 0; i < MAX_BUFFERS; i++) {
                Buffers[i] = {0};
            };
            
            // TEXT FILE
            ProgramState->Buffers = BufferList();
            ListAdd(Buffers, LoadFileToBuffer("./data/testing_files/utf-8.txt"));
            
            ProgramState->KeyFirstRepeatTime = 0.4f;
            ProgramState->KeyRepeatSpeed = 0.02f;
            
            FillKeyData(&ProgramState->Input);
            
            Settings->TextSubLineOffset = 4;
            Settings->TextMarginLeft = 10;
            Settings->LineNumberWidth = 4;
            Settings->ScrollbarWidth = 15;
            
            *Font = {0};
            //Settings->Font.Path = String("./data/fonts/LiberationMono-Regular.ttf");
            Font->Path = String("./data/fonts/HelveticaNeue-Regular.otf");
            Font->Size = 18;
            
            {// Colors
                Settings->Colors.ViewBG = RGB(255, 255, 255);
                Settings->Colors.TextBG = RGBA(0, 0, 0, 0);
                Settings->Colors.TextFG = RGB(0, 0, 0);
                Settings->Colors.LineNumberBG = LIGHTGRAY;
                Settings->Colors.LineNumberFG = GRAY;
                Settings->Colors.CursorActiveBG = Settings->Colors.TextFG;
                Settings->Colors.CursorActiveFG = Settings->Colors.TextBG;
                Settings->Colors.CursorInactiveBG = Settings->Colors.TextFG;
                Settings->Colors.CursorInactiveFG = Settings->Colors.TextFG;
                Settings->Colors.SelectionAreaActiveBG = ORANGE;
                Settings->Colors.SelectionAreaActiveFG = BLACK;
                Settings->Colors.SelectionAreaInactiveBG = ORANGE;
                Settings->Colors.SelectionAreaInactiveFG = BLACK;
            }
            
            ProgramState->ScreenHeight = Memory->WindowHeight;
            ProgramState->ScreenWidth = Memory->WindowWidth;
            
            ProgramState->Views = ViewList();
            ListAdd(Views, View(ProgramState, &Buffers->Data[0], -1, Location_Below));
            
            ProgramState->SelectedViewIndex = 0;
            
            ProgramState->HasTargetGlobalCursorPositionBeenSet = false;
            ProgramState->GlobalCursor = Rect(0, 0, 0, 0);
            ProgramState->TargetGlobalCursor = Rect(0, 0, 0, 0);
            
            ProgramState->ShowViewInfo = false;
            ProgramState->ShowViewRects = false;
            
            ProgramState->ShowSuperDebugMenu = false;
            ProgramState->SuperDebugMenuY = 0;
            
            FillLineData(&ProgramState->Views[0], Settings);
            
            DefineProfile(FillLineData);
            DefineProfile(Rendering);
            DefineProfile(Total);
            
            TempStrings = StringList();
        }
        
        PurgeTempStrings();
        
        for(int i = 0; i < Views->Length; i++)
        {
            view *View = &ProgramState->Views[i];
            if(View->ListerIsOpen && View->Lister.ShouldExecute)
                ExecLister(ProgramState, View);
        }
        
        
        // Re-load fonts if needed
        // TODO: reimplement
        //if(ProgramState->PrevFontSize != ProgramState->FontSize)
        //LoadFonts(ProgramState);
        //ProgramState->PrevFontSize = ProgramState->FontSize;
        
        StartProfiling();
        StartProfile(Total);
        
        view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
        
        UpdateKeyInput(&ProgramState->Input);
        
        ProgramState->ScreenWidth = Memory->WindowWidth;
        ProgramState->ScreenHeight = Memory->WindowHeight;
        
        v2 CharDim = GetCharDim(Font);
        int CharWidth = CharDim.x;
        int CharHeight = CharDim.y;
        
        if(WindowShouldClose() || ProgramState->ShouldExit)
        {
            Memory->IsRunning = false;
        }
        
        ComputeAllViewRects(ProgramState);
        
        for(int i = 0; i < ProgramState->Views.Length; i++)
        {
            ProgramState->Views[i].ComputeInternalGeometry(Settings);
        }
        
        HandleInput(ProgramState);
        
        
        // lister set matchingentries
        // TODO: only do this when Input has changed
        // TODO: better string matching
        for(int a = 0; a < Views->Length; a++)
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
                
                for(int i = 0; i < Lister->Entries.Length; i++)
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
            
            View->Lister.SelectedIndex = Clamp(View->Lister.SelectedIndex, 0, View->Lister.MatchingEntries.Length - 1);
        }
        
        
        v2 MousePos = V2(GetMousePosition());
        if(IsMouseButtonDown(0))
        {
            int NewViewIndex = 0;
            for(int i = 0; i < Views->Length; i++)
            {
                rect Rect = Views->Data[i].TotalRect;
                if(MousePos.x >= Rect.x && MousePos.y >= Rect.y &&
                   MousePos.x <= Rect.x + Rect.w && MousePos.y <= Rect.y + Rect.h)
                {
                    NewViewIndex = i;
                }
            }
            ProgramState->SelectedViewIndex = NewViewIndex;
            View = &ProgramState->Views[ProgramState->SelectedViewIndex];
        }
        
        for(int i = 0; i < Views->Length; i++)
        {
            view *View = &Views->Data[i];
            if(View->ListerIsOpen)
            {
                lister *Lister = &View->Lister;
                int Y = View->BufferRect.y;
                ListFree(&Lister->Rects);
                Lister->Rects = RectList(Lister->MatchingEntries.Length);
                for(int i = 0; i < Lister->MatchingEntries.Length; i++)
                {
                    ListAdd(&Lister->Rects, Rect(View->TotalRect.x, Y, View->TotalRect.w, CharHeight));
                    
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
            
            buffer_pos MouseBufferPos = View->ClosestBufferPos(View->ScreenPosToCharPos(MousePos));
            View->CursorPos = MouseBufferPos;
            View->IdealCursorCol = View->CursorPos.c;
        }
        
        
        View->CursorPos.l = Clamp(View->CursorPos.l, 0, View->LineCount()-1);
        View->CursorPos.c = Clamp(View->CursorPos.c, 0, View->LineLength(View->CursorPos.l));
        
        StartProfile(FillLineData);
        for(int i = 0; i < Views->Length; i++)
        {
            view *View = &Views->Data[i];
            FillLineData(View, Settings);
            
            // TODO: unnecessary to interpolate now?
            View->CursorTargetRect = View->CharRectAt(View->CursorPos.l, View->CursorPos.c);
            View->CursorRect = Interpolate(View->CursorRect, View->CursorTargetRect, 0.5f);
            
            AdjustView(ProgramState, View);
            View->Y = Interpolate(View->Y, View->TargetY, 0.4f);
        }
        EndProfile(FillLineData);
        
        ProgramState->TargetGlobalCursor = View->CharRectToScreenRect(View->CursorTargetRect);
        if(!ProgramState->HasTargetGlobalCursorPositionBeenSet)
        {
            ProgramState->GlobalCursor = ProgramState->TargetGlobalCursor;
            ProgramState->HasTargetGlobalCursorPositionBeenSet = true;
        }
        ProgramState->GlobalCursor = Interpolate(ProgramState->GlobalCursor,
                                                 ProgramState->TargetGlobalCursor,
                                                 0.5f);
        
        StartProfile(Rendering);
        BeginDrawing();
        for(int i = 0; i < Views->Length; i++)
        {
            view *View = &Views->Data[i];
            DrawView(ProgramState, View);
        }
        
        {
            int X = ProgramState->ScreenWidth - 200;
            int Y = 20;
            DrawString(Settings, TempString("%d Actions", View->Buffer->ActionStack.Length), V2(X, Y+=20), BLACK, PURPLE);
            for(int i = 0; i < View->Buffer->ActionStack.Length; i++)
            {
                action A = View->Buffer->ActionStack[i];
                
                if(A.Delete)
                {
                    DrawString(Settings, TempString("Delete %d,%d to %d,%d", A.DeleteStart.l, A.DeleteStart.c, A.DeleteEnd.l, A.DeleteEnd.c), V2(X, Y+=20), BLACK, YELLOW);
                }
                else if(A.Add)
                {
                    DrawString(Settings, TempString("Add string"), V2(X, Y+=20), BLACK, YELLOW);
                }
            }
        }
        
        
#if 0
        view *CurrentView = &ProgramState->Views[ProgramState->SelectedViewIndex];
        u32 CurrentCodepoint = CharAt(CurrentView, CurrentView->CursorPos);
        string CPBitString = String("");
        for(int i = 0; i < 32; i++)
        {
            CPBitString.AppendChar(IsolateBitInU32(CurrentCodepoint, i) ? '1' : '0');
        }
        DrawString(ProgramState, CPBitString, V2(CharToScreenSpace(CurrentView, CurrentView->CursorRect)), BLACK, ORANGE, 0);
        DrawString(ProgramState, TempString("%D", CurrentCodepoint),
                   V2(CharToScreenSpace(CurrentView, CurrentView->CursorRect)) + V2(0, ProgramState->FontSize), BLACK, ORANGE, 0);
        CPBitString.Free();
        
#endif
        
#if 0
        int Y = 300;
        for(int i = 0; i < ProgramState->Buffers.Length; i++)
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
        
#if 0
        DrawRectangle(0, 0, 500, 500, WHITE);
        DrawTexture(ProgramState->FontMonospace.RFont.texture, 0, 0, BLACK);
#endif
        
        EndDrawing();
        EndProfile(Rendering);
        
        EndProfile(Total);
        
        
    }
}
