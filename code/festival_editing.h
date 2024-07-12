/* date = December 2nd 2023 1:45 pm */

#ifndef FESTIVAL_EDITING_H
#define FESTIVAL_EDITING_H

void
PasteFromClipboard(program_state *ProgramState, view *View)
{
    // TODO: Is it ok to cast from const char* to char*?
    char *ClipboardText = (char *)GetClipboardText();
    if(strlen(ClipboardText) == 0)
        return;
    
    Print("Pasting text: '%s'", ClipboardText);
    u64 TextLength = 0;
    u32 *Text = ConvertTextToUTF32(ClipboardText, GetTextEncodingType(ClipboardText), &TextLength);
    
    // TODO: add conversion function to string
    //string TextString = String("");
    
    string_list TextStringList = StringList();
    TextStringList.Add(String(""));
    int StringIndex = 0;
    
    for(int i = 0; i < TextLength; i++)
    {
        if((char)Text[i] == '\n')
        {
            StringIndex++;
            TextStringList.Add(String(""));
        }
        else
        {
            TextStringList[StringIndex].AppendChar(Text[i]);
        }
    }
    
    AddAndDoAction(ProgramState, View->Buffer,
                   ActionForInsertStringList(View->CursorPos,
                                             TextStringList,
                                             /* TODO: split text if it has newline */ false));
    
    FreeStringList(TextStringList);
}

void
CopyToClipboard(program_state *ProgramState, view *View, buffer_pos Start, buffer_pos End, bool AddNewline)
{
    string StringToCopy = String("");
    for(int l = Start.l; l <= End.l; l++)
    {
        if(l > Start.l)
            StringToCopy.AppendChar('\n');
        
        int EndColumn = LineLength(View, l) - 1;
        if(l == End.l)
            EndColumn = End.c;
        
        for(int c = Start.c; c <= EndColumn; c++)
        {
            StringToCopy.AppendChar(CharAt(View, BufferPos(l, c)));
        }
    }
    if(AddNewline)
        StringToCopy.AppendChar('\n');
    
    u64 CharCount;
    char *StringToCopyUTF8 = ConvertUTF32StringToEncoding(StringToCopy, "UTF8", &CharCount);
    
    SetClipboardText(StringToCopyUTF8);
    
    TryFree(StringToCopyUTF8);
    StringToCopy.Free();
}

void
SwitchInputMode(program_state *ProgramState, input_mode NewMode)
{
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    if(ProgramState->InputMode == InputMode_Insert)
    {
        if(View->InsertModeString.Length > 0)
        {
            string_list List = StringList();
            List.Add(CopyString(View->InsertModeString));
            AddAction(View->Buffer,
                      ActionForInsertStringList(View->InsertModeStartPos,
                                                List,
                                                View->InsertModeLineBelow));
            FreeStringList(List);
        }
        View->InsertModeString.Free();
        // check adlinebelow
    }
    
    if(NewMode == InputMode_Select)
    {
        View->Selecting = true;
        View->SelectionStartPos = View->CursorPos;
    }
    else
    {
        View->Selecting = false;
    }
    
    if(NewMode == InputMode_Insert)
    {
        View->InsertModeStartPos = View->CursorPos;
        View->InsertModeLineBelow = false;
        View->InsertModeString = String("");
    }
    
    ProgramState->InputMode = NewMode;
}

string
GetStringToInsert(program_state *ProgramState)
{
    input *Input = &ProgramState->Input;
    string Result = String("");
    
    for(int i = 0; i < 26; i++)
    {
        if(KeyShouldExecute(Input->LetterKeys[i]))
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
        if(KeyShouldExecute(Input->NumberKeys[i]))
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
        if(KeyShouldExecute(Input->SymbolKeys[i]))
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
    
    if(KeyShouldExecute(Input->Space_Key))
        Result.AppendChar(' ');
    if(KeyShouldExecute(Input->Tab_Key))
        Result.AppendChar('\t');
    
    return Result;
}



void
HandleInput_Insert(program_state *ProgramState)
{
    buffer *Buffer = ProgramState->Views[ProgramState->SelectedViewIndex].Buffer;
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    input *Input = &ProgramState->Input;
    
    if(KeyShouldExecute(Input->Escape_Key))
    {
        SwitchInputMode(ProgramState, InputMode_Nav);
        return;
    }
    
    if(KeyShouldExecute(Input->UpKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
        SwitchInputMode(ProgramState, InputMode_Insert);
    }
    if(KeyShouldExecute(Input->DownKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
        SwitchInputMode(ProgramState, InputMode_Insert);
    }
    if(KeyShouldExecute(Input->LeftKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
        SwitchInputMode(ProgramState, InputMode_Insert);
    }
    if(KeyShouldExecute(Input->RightKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
        
        SwitchInputMode(ProgramState, InputMode_Insert);
    }
    
    string StringToInsert = GetStringToInsert(ProgramState);
    if(StringToInsert.Length > 0)
        ProgramState->ShouldChangeIdealCursorCol = true;
    
    
    if(KeyShouldExecute(Input->Backspace_Key))
    { // Backspace
        if(StringToInsert.Length > 0)
        {
            StringToInsert.RemoveCharFromEnd();
        }
        else if(View->CursorPos.c > 0)
        {
#if 0
            Buffer->Lines[View->CursorPos.l].RemoveChar(View->CursorPos.c-1);
#endif
            View->CursorPos.c--;
            View->CursorPos.c = Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
            AddAndDoAction(ProgramState, View->Buffer,
                           ActionForDeleteRange(View->Buffer, View->CursorPos, View->CursorPos));
            SwitchInputMode(ProgramState, InputMode_Insert);
        }
        ProgramState->ShouldChangeIdealCursorCol = true;
    }
    
    if(KeyShouldExecute(Input->Delete_Key))
    {
        // TODO: merge lines
        if(Buffer->Lines[View->CursorPos.l].Length > View->CursorPos.c)
        {
            Buffer->Lines[View->CursorPos.l].RemoveChar(View->CursorPos.c);
            
            SwitchInputMode(ProgramState, InputMode_Insert);
        }
        else if(View->CursorPos.l < Buffer->Lines.Length)
        {
            buffer_pos PStart = BufferPos(View->CursorPos.l, View->CursorPos.c + 1);
            buffer_pos PEnd = BufferPos(View->CursorPos.l + 1, -1);
            AddAndDoAction(ProgramState, Buffer, ActionForDeleteRange(Buffer, PStart, PEnd));
            
            SwitchInputMode(ProgramState, InputMode_Insert);
        }
        //ProgramState->ShouldChangeIdealCursorCol = true;
    }
    
    View->InsertModeString.AppendString(StringToInsert);
    
    Buffer->Lines[View->CursorPos.l].InsertString(View->CursorPos.c, StringToInsert);
    View->CursorPos.c += StringToInsert.Length;
    
    StringToInsert.Free();
    
    
    // TODO: improve key names
    if(KeyShouldExecute(Input->Return_Key))
    {
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
        
        View->InsertModeLineBelow = true;
        SwitchInputMode(ProgramState, InputMode_Insert);
    }
}

void
HandleInput_EntryBar(program_state *ProgramState)
{
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    input *Input = &ProgramState->Input;
    lister *Lister = &View->Lister;
    string *EntryString = &View->Lister.Input;
    
    if(KeyShouldExecute(Input->Escape_Key))
    {
        CloseLister(ProgramState, View);
        return;
    }
    
    if(KeyShouldExecute(Input->Return_Key) ||
       KeyShouldExecute(Input->Tab_Key))
    {
        Lister->ShouldExecute = true;
    }
    if(KeyShouldExecute(Input->UpKey))
    {
        if(Lister->SelectedIndex > 0)
            Lister->SelectedIndex--;
    }
    if(KeyShouldExecute(Input->DownKey))
    {
        if(Lister->SelectedIndex < Lister->MatchingEntries.Length - 1)
            Lister->SelectedIndex++;
    }
    
    if(KeyShouldExecute(Input->Backspace_Key))
    {
        if(Lister->Input.Length > 0)
            Lister->Input.Length--;
    }
    
    string StringToInsert = GetStringToInsert(ProgramState);
    EntryString->AppendString(StringToInsert);
    StringToInsert.Free();
}



void
ProcessNavMovement(program_state *ProgramState)
{
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    input *Input = &ProgramState->Input;
    
    if(KeyShouldExecute(Input->UpKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
    }
    if(KeyShouldExecute(Input->DownKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
    }
    if(KeyShouldExecute(Input->LeftKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
    }
    if(KeyShouldExecute(Input->RightKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
    
    if(KeyShouldExecute(Input->IKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
    }
    if(KeyShouldExecute(Input->KKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
    }
    if(KeyShouldExecute(Input->JKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
    }
    if(KeyShouldExecute(Input->LKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
    
    if(KeyShouldExecute(Input->UKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, View, SeekBackBorder(View, View->CursorPos));
    }
    if(KeyShouldExecute(Input->OKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, View, SeekForwardBorder(View, View->CursorPos));
    }
    
    if(KeyShouldExecute(Input->HKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        if(!AtLineBeginning(View, View->CursorPos))
            SetCursorPos(ProgramState, View, SeekLineBeginning(View, View->CursorPos.l));
        else
            SetCursorPos(ProgramState, View, SeekPrevEmptyLine(View, View->CursorPos.l));
    }
    if(KeyShouldExecute(Input->Semicolon_Key))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        if(!AtLineEnd(View, View->CursorPos))
            SetCursorPos(ProgramState, View, SeekLineEnd(View, View->CursorPos.l));
        else
            SetCursorPos(ProgramState, View, SeekNextEmptyLine(View, View->CursorPos.l));
    }
    if(KeyShouldExecute(Input->PageDown_Key))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, View, SeekLineEnd(View, View->Buffer->Lines.Length - 1));
    }
    if(KeyShouldExecute(Input->PageUp_Key))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, View, SeekLineBeginning(View, 0));
    }
}

void
HandleInput_Nav(program_state *ProgramState)
{
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    input *Input = &ProgramState->Input;
    
    if(KeyShouldExecute(Input->FKey) && !IsAnyShiftKeyDown && !IsAnyControlKeyDown)
    {
        SwitchInputMode(ProgramState, InputMode_Insert);
        return;
    }
    if(KeyShouldExecute(Input->SKey) && !IsAnyShiftKeyDown && !IsAnyControlKeyDown)
    {
        SwitchInputMode(ProgramState, InputMode_Select);
        return;
    }
    
    ProcessNavMovement(ProgramState);
    
    if(KeyShouldExecute(Input->CKey) && !IsAnyShiftKeyDown && !IsAnyControlKeyDown)
    {
        buffer_pos Start = BufferPos(View->CursorPos.l, 0);
        buffer_pos End = BufferPos(View->CursorPos.l, LineLength(View, View->CursorPos.l));
        CopyToClipboard(ProgramState, View, Start, End, true);
    }
    
    if(KeyShouldExecute(Input->Slash_Key) && IsAnyShiftKeyDown)
    {
        ProgramState->ShowSuperDebugMenu = true;
    }
    
    if(KeyShouldExecute(Input->NKey) && IsAnyControlKeyDown)
    {
        if(IsAnyShiftKeyDown)
        {
            SplitView(ProgramState, Location_Below);
        }
        else
        {
            SplitView(ProgramState, Location_Right);
        }
    }
    
    if(KeyShouldExecute(Input->VKey))
    {
        PasteFromClipboard(ProgramState, View);
    }
    
    if(KeyShouldExecute(Input->YKey))
    {
        if(IsAnyShiftKeyDown)
            MoveForwardActionStack(ProgramState, View->Buffer);
        else
            MoveBackActionStack(ProgramState, View->Buffer);
    }
    
    if(KeyShouldExecute(Input->XKey))
    {
        AddAndDoAction(ProgramState, View->Buffer, ActionForDeleteLine(View->Buffer, View->CursorPos.l));
    }
    
    if(KeyShouldExecute(Input->EKey)) {
        OpenEditFileLister(ProgramState, View, "./");
    }
    else if(KeyShouldExecute(Input->GKey)) {
        OpenSwitchBufferLister(ProgramState, View);
    }
    else if(KeyShouldExecute(Input->FKey) && IsAnyShiftKeyDown) {
        OpenSwitchFontTypeLister(ProgramState, View);
    }
    else if(KeyShouldExecute(Input->AKey)) {
        OpenCommandLister(ProgramState, View);
    }
    
    
    
    if(KeyShouldExecute(Input->WKey))
    {// TODO: write current buffer to file
    }
    
    if(KeyShouldExecute(Input->QKey))
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
    input *Input = &ProgramState->Input;
    
    if((KeyShouldExecute(Input->SKey) && !IsAnyShiftKeyDown && !IsAnyControlKeyDown) || KeyShouldExecute(Input->Escape_Key))
    {
        SwitchInputMode(ProgramState, InputMode_Nav);
        return;
    }
    
    ProcessNavMovement(ProgramState);
    
    if(KeyShouldExecute(Input->CKey) && !IsAnyShiftKeyDown && !IsAnyControlKeyDown)
    {
        b32 AddNewLine = (View->CursorPos.c == LineLength(View, View->CursorPos.l));
        buffer_pos Start = SmallerBufferPos(View->SelectionStartPos, View->CursorPos);
        buffer_pos End = LargerBufferPos(View->SelectionStartPos, View->CursorPos);
        CopyToClipboard(ProgramState, View, Start, End, AddNewLine);
        SwitchInputMode(ProgramState, InputMode_Nav);
    }
    
    if(KeyShouldExecute(Input->XKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        AddAndDoAction(ProgramState, View->Buffer, ActionForDeleteRange(View->Buffer, View->SelectionStartPos, View->CursorPos));
        
        SwitchInputMode(ProgramState, InputMode_Nav);
        return;
    }
}

void
HandleInput_SuperDebugMenu(program_state *ProgramState)
{
    input *Input = &ProgramState->Input;
    if(KeyShouldExecute(Input->Slash_Key) && IsAnyShiftKeyDown)
    {
        ProgramState->ShowSuperDebugMenu = false;
    }
    
    int ScrollAmount = ProgramState->Settings.Font.Size * 3;
    ProgramState->SuperDebugMenuY += GetMouseWheelMove() * ScrollAmount * -1;
    ProgramState->SuperDebugMenuY = Clamp(ProgramState->SuperDebugMenuY,
                                          0, ProgramState->SuperDebugMenuH - 100);
}



void
HandleInput(program_state *ProgramState)
{
    input *Input = &ProgramState->Input;
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
    
    int ScrollAmount = ProgramState->Settings.Font.Size * 3;
    if(GetMouseWheelMoveV().y < 0)
        View->TargetY += ScrollAmount;
    else if(GetMouseWheelMoveV().y > 0)
        View->TargetY -= ScrollAmount;
    
    if(IsAnyControlKeyDown)
    {
        // TODO: implement font size changing
        /*
                ProgramState->Font.Size += GetMouseWheelMove();
                if(ProgramState->Font.Size < 6) ProgramState->Font.Size = 6;
                if(ProgramState->Font.Size > 100) ProgramState->Font.Size = 100;
        */
    }
    
    
    
}



#endif //FESTIVAL_EDITING_H
