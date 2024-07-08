#include <stdio.h>

#include "raylib.h"
#include "chardet.h"
#include <sys/stat.h>

#include "festival_base.h"
#include "festival_platform.h"

#include "festival_math.h"
#include "festival_lists.h"
#include "festival_string.h"
#include "festival_input.h"

#include "festival_actions.h"
#include "festival_encoding.h"
#include "festival_encoding.cpp"
#include "festival.h"
#include "festival_filesystem.h"

#include "festival_functions.cpp"
#include "festival_profiling.h"
#include "festival_commands.h"
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
            ListAdd(Buffers, LoadFileToBuffer("./testing_files/utf-8.txt"));
            
            
            ProgramState->FontSize = 18;
            LoadFonts(ProgramState);
            ProgramState->FontType = FontType_Monospace;
            
            ProgramState->KeyFirstRepeatTime = 0.4f;
            ProgramState->KeyRepeatSpeed = 0.02f;
            
            FillKeyData(&ProgramState->Input);
            
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
        
        for(int i = 0; i < Views->Length; i++)
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
        
        UpdateKeyInput(&ProgramState->Input);
        
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
        for(int i = 0; i < Views->Length; i++)
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
        
        
        for(int i = 0; i < Views->Length; i++)
        {
            Views->Data[i].ComputedFromParentThisFrame = false;
        }
        
        RootView->Rect.x = 0;
        RootView->Rect.y = 0;
        RootView->Rect.w = ProgramState->ScreenWidth;
        RootView->Rect.h = ProgramState->ScreenHeight;
        ComputeTextRect(ProgramState, RootView);
        RootView->ComputedFromParentThisFrame = true;
        
        for(int ViewIndex = 0; ViewIndex < Views->Length; ViewIndex++)
        {
            int Id = Views->Data[ViewIndex].Id;
            // Compute child count
            int ChildCount = 0;
            for(int i = 0; i < Views->Length; i++)
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
                for(int i = 0; i < Views->Length; i++)
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
        
        for(int i = 0; i < Views->Length; i++)
        {
            view *View = &Views->Data[i];
            if(View->ListerIsOpen)
            {
                lister *Lister = &View->Lister;
                int Y = View->TextRect.y;
                ListFree(&Lister->Rects);
                Lister->Rects = RectList(Lister->MatchingEntries.Length);
                for(int i = 0; i < Lister->MatchingEntries.Length; i++)
                {
                    ListAdd(&Lister->Rects, Rect(View->Rect.x, Y, View->Rect.w, CharHeight));
                    
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
        
        
        View->CursorPos.l = Clamp(View->CursorPos.l, 0, LineCount(View)-1);
        View->CursorPos.c = Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
        
        
        StartProfile(FillLineData);
        for(int i = 0; i < Views->Length; i++)
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
        for(int i = 0; i < Views->Length; i++)
        {
            view *View = &Views->Data[i];
            DrawView(ProgramState, View);
        }
        
        {
            int X = ProgramState->ScreenWidth - 200;
            int Y = 20;
            DrawString(ProgramState, TempString("%d Actions", View->Buffer->ActionStack.Length), V2(X, Y+=20), BLACK, PURPLE);
            for(int i = 0; i < View->Buffer->ActionStack.Length; i++)
            {
                action A = View->Buffer->ActionStack[i];
                
                if(A.Delete)
                {
                    DrawString(ProgramState, TempString("Delete %d,%d to %d,%d", A.DeleteStart.l, A.DeleteStart.c, A.DeleteEnd.l, A.DeleteEnd.c), V2(X, Y+=20), BLACK, YELLOW);
                }
                else if(A.Add)
                {
                    DrawString(ProgramState, TempString("Add %S", A.AddContent), V2(X, Y+=20), BLACK, YELLOW);
                }
            }
        }
        
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
