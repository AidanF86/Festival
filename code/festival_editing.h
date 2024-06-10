/* date = December 2nd 2023 1:45 pm */

#ifndef FESTIVAL_EDITING_H
#define FESTIVAL_EDITING_H

string
GetStringToInsert(program_state *ProgramState)
{
    string Result = String("");
    
    for(int i = 0; i < 26; i++)
    {
        if(KeyShouldExecute(ProgramState->LetterKeys[i]))
        {
            char CharToAdd;
            if(IsAnyShiftKeyDown)
                Result.AppendChar('A' + i);
            else
                Result.AppendChar('a' + i);
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
            
            Result.AppendChar(CharToAdd);
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
            
            Result.AppendChar(CharToAdd);
        }
    }
    
    return Result;
    
#if 0
    
#endif
}



void
HandleInput_Insert(program_state *ProgramState)
{
    buffer *Buffer = ProgramState->Views[ProgramState->SelectedViewIndex].Buffer;
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    if(KeyShouldExecute(ProgramState->Escape_Key))
    {
        SwitchToNavMode(ProgramState);
        View->TotalInsertString.Free();
        return;
    }
    
    if(KeyShouldExecute(ProgramState->UpKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
    }
    if(KeyShouldExecute(ProgramState->DownKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
    }
    if(KeyShouldExecute(ProgramState->LeftKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
    }
    if(KeyShouldExecute(ProgramState->RightKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
    
    string StringToInsert = GetStringToInsert(ProgramState);
    if(StringToInsert.Length > 0)
        ProgramState->ShouldChangeIdealCursorCol = true;
    
    Buffer->Lines[View->CursorPos.l].InsertString(View->CursorPos.c, StringToInsert);
    View->CursorPos.c += StringToInsert.Length;
    
    StringToInsert.Free();
    
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
                ProgramState->ShouldChangeIdealCursorCol = true;
            }
            else if(i == 1)
            { // Backspace
                if(View->CursorPos.c > 0)
                {
                    Buffer->Lines[View->CursorPos.l].RemoveChar(View->CursorPos.c-1);
                    View->CursorPos.c--;
                    Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
                }
                ProgramState->ShouldChangeIdealCursorCol = true;
            }
            else if(i == 2)
            { // Delete
                Buffer->Lines[View->CursorPos.l].RemoveChar(View->CursorPos.c);
                ProgramState->ShouldChangeIdealCursorCol = true;
            }
            else if(i == 3)
            { // Tab
                // TODO: handling tab char
                for(int a = 0; a < 4; a++)
                {
                    Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, ' ');
                    View->CursorPos.c++;
                    ProgramState->ShouldChangeIdealCursorCol = true;
                }
            }
            else if(i == 4)
            { // Return
                InsertLine(Buffer, View->CursorPos.l+1, CopyString(Buffer->Lines[View->CursorPos.l]));
                if(Buffer->Lines[View->CursorPos.l].Length != 0)
                {
                    Buffer->Lines[View->CursorPos.l].Slice(0, View->CursorPos.c);
                    Buffer->Lines[View->CursorPos.l+1].Slice(View->CursorPos.c, Buffer->Lines[View->CursorPos.l+1].Length);
                }
                
                View->CursorPos.l++;
                View->CursorPos.c = 0;
                
                ProgramState->UserMovedCursor = true;
                ProgramState->ShouldChangeIdealCursorCol = true;
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
HandleInput_EntryBar(program_state *ProgramState)
{
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    lister *Lister = &View->Lister;
    string *EntryString = &View->Lister.Input;
    
    if(KeyShouldExecute(ProgramState->Escape_Key))
    {
        CloseLister(ProgramState, View);
        return;
    }
    
    if(KeyShouldExecute(ProgramState->Return_Key) ||
       KeyShouldExecute(ProgramState->Tab_Key))
    {
        Lister->ShouldExecute = true;
    }
    if(KeyShouldExecute(ProgramState->UpKey))
    {
        if(Lister->SelectedIndex > 0)
            Lister->SelectedIndex--;
    }
    if(KeyShouldExecute(ProgramState->DownKey))
    {
        if(Lister->SelectedIndex < Lister->MatchingEntries.Count - 1)
            Lister->SelectedIndex++;
    }
    
    if(KeyShouldExecute(ProgramState->Backspace_Key))
    {
        if(Lister->Input.Length > 0)
            Lister->Input.Length--;
    }
    
    string StringToInsert = GetStringToInsert(ProgramState);
    EntryString->AppendString(StringToInsert);
    StringToInsert.Free();
}



void
ProcessModalMovement(program_state *ProgramState)
{
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    if(KeyShouldExecute(ProgramState->UpKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
    }
    if(KeyShouldExecute(ProgramState->DownKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
    }
    if(KeyShouldExecute(ProgramState->LeftKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
    }
    if(KeyShouldExecute(ProgramState->RightKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
    
    if(KeyShouldExecute(ProgramState->IKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
    }
    if(KeyShouldExecute(ProgramState->KKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
    }
    if(KeyShouldExecute(ProgramState->JKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
    }
    if(KeyShouldExecute(ProgramState->LKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
    
    if(KeyShouldExecute(ProgramState->UKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, View, SeekBackBorder(View, View->CursorPos));
    }
    if(KeyShouldExecute(ProgramState->OKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, View, SeekForwardBorder(View, View->CursorPos));
    }
    
    if(KeyShouldExecute(ProgramState->HKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        if(!AtLineBeginning(View, View->CursorPos))
            SetCursorPos(ProgramState, View, SeekLineBeginning(View, View->CursorPos));
        else
            SetCursorPos(ProgramState, View, SeekPrevEmptyLine(View, View->CursorPos));
    }
    if(KeyShouldExecute(ProgramState->Semicolon_Key))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        if(!AtLineEnd(View, View->CursorPos))
            SetCursorPos(ProgramState, View, SeekLineEnd(View, View->CursorPos));
        else
            SetCursorPos(ProgramState, View, SeekNextEmptyLine(View, View->CursorPos));
    }
}

void
HandleInput_Nav(program_state *ProgramState)
{
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    if(KeyShouldExecute(ProgramState->FKey) && !IsAnyShiftKeyDown && !IsAnyControlKeyDown)
    {
        SwitchToInsertMode(ProgramState);
        return;
    }
    if(KeyShouldExecute(ProgramState->SKey) && !IsAnyShiftKeyDown && !IsAnyControlKeyDown)
    {
        SwitchToSelectMode(ProgramState);
        return;
    }
    
    ProcessModalMovement(ProgramState);
    
    if(KeyShouldExecute(ProgramState->Slash_Key) && IsAnyShiftKeyDown)
    {
        ProgramState->ShowSuperDebugMenu = true;
    }
    
    if(KeyShouldExecute(ProgramState->NKey))
    {
        if(IsAnyShiftKeyDown)
        {
            SplitViewVertical(ProgramState);
        }
        else
        {
            SplitViewHorizontal(ProgramState);
        }
    }
    
    if(KeyShouldExecute(ProgramState->YKey))
    {
        if(IsAnyShiftKeyDown)
            MoveForwardActionStack(View->Buffer);
        else
            MoveBackActionStack(View->Buffer);
    }
    
    if(KeyShouldExecute(ProgramState->XKey))
    {
        DoAndAddAction(View->Buffer, ActionForDeleteLine(View->Buffer, View->CursorPos.l));
    }
    
    if(KeyShouldExecute(ProgramState->EKey)) {
        OpenEditFileLister(ProgramState, View, "./");
    }
    else if(KeyShouldExecute(ProgramState->GKey)) {
        OpenSwitchBufferLister(ProgramState, View);
    }
    else if(KeyShouldExecute(ProgramState->FKey) && IsAnyShiftKeyDown) {
        OpenSwitchFontTypeLister(ProgramState, View);
    }
    else if(KeyShouldExecute(ProgramState->AKey)) {
        OpenCommandLister(ProgramState, View);
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
HandleInput_Select(program_state *ProgramState)
{
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    if((KeyShouldExecute(ProgramState->SKey) && !IsAnyShiftKeyDown && !IsAnyControlKeyDown) || KeyShouldExecute(ProgramState->Escape_Key))
    {
        SwitchToNavMode(ProgramState);
        return;
    }
    
    ProcessModalMovement(ProgramState);
    
    if(KeyShouldExecute(ProgramState->XKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        DoAndAddAction(View->Buffer, ActionForDeleteRange(View->Buffer, View->SelectionStartPos, View->CursorPos));
        
        SwitchToNavMode(ProgramState);
        return;
    }
}

void
HandleInput_SuperDebugMenu(program_state *ProgramState)
{
    if(KeyShouldExecute(ProgramState->Slash_Key) && IsAnyShiftKeyDown)
    {
        ProgramState->ShowSuperDebugMenu = false;
    }
}



void
HandleInput(program_state *ProgramState)
{
    ProgramState->UserMovedCursor = false;
    if(ProgramState->ShowSuperDebugMenu)
    {
        HandleInput_SuperDebugMenu(ProgramState);
    }
    else if(ProgramState->Views[ProgramState->SelectedViewIndex].ListerIsOpen)
    {
        HandleInput_EntryBar(ProgramState);
    }
    else if(ProgramState->InputMode == InputMode_Nav)
    {
        HandleInput_Nav(ProgramState);
    }
    else if(ProgramState->InputMode == InputMode_Insert)
    {
        HandleInput_Insert(ProgramState);
    }
    else if(ProgramState->InputMode == InputMode_Select)
    {
        HandleInput_Select(ProgramState);
    }
    
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    int ScrollAmount = ProgramState->FontSize * 3;
    if(GetMouseWheelMoveV().y < 0)
        View->TargetY += ScrollAmount;
    else if(GetMouseWheelMoveV().y > 0)
        View->TargetY -= ScrollAmount;
    
    if(ProgramState->ShouldChangeIdealCursorCol)
    {
        ProgramState->ShouldChangeIdealCursorCol = false;
        View->IdealCursorCol = View->CursorPos.c;
    }
    else
    {
        View->CursorPos.c = View->IdealCursorCol;
        //Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
    }
    
    
    if(IsAnyControlKeyDown)
    {
        ProgramState->FontSize += GetMouseWheelMove();
        if(ProgramState->FontSize < 6) ProgramState->FontSize = 6;
        if(ProgramState->FontSize > 100) ProgramState->FontSize = 100;
    }
}



#endif //FESTIVAL_EDITING_H
