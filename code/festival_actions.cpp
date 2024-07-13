

// inc, inc
action
ActionForDeleteRange(buffer *Buffer, buffer_pos Start, buffer_pos End)
{
    buffer_pos ActualStart = Start.l < End.l || (Start.l == End.l && Start.c < End.c) ? Start : End;
    buffer_pos ActualEnd = ActualStart == Start ? End : Start;
    ActualEnd.c++;
    ActualEnd.c = Clamp(ActualEnd.c, 0, Buffer->LineLength(ActualEnd.l));
    
    action Result;
    
    Result.Delete = true;
    Result.DeleteSingleLine = false;
    Result.DeleteStart = ActualStart;
    Result.DeleteEnd = ActualEnd;
    
    Result.DeleteContent = CopyStringListRange(Buffer->Lines, ActualStart.l, ActualStart.c, ActualEnd.l, ActualEnd.c);
    
    Result.Add = false;
    
    return Result;
}

action
ActionForDeleteLine(buffer *Buffer, int Line)
{
    action Result;
    
    Result.Delete = true;
    Result.DeleteSingleLine = true;
    Result.DeleteContent = CopyStringListRange(Buffer->Lines, Line, 0, Line + 1, 0);
    Result.DeleteStart = BufferPos(Line, 0);
    Result.DeleteEnd = BufferPos(Line + 1, 0);
    
    for(int i = 0; i < Result.DeleteContent.Length; i++)
    {
        Print(Result.DeleteContent[i]);
    }
    
    Result.Add = false;
    
    return Result;
}

action
ActionForInsertStringList(buffer_pos Pos, string_list List, b32 LineBelow)
{
    action Result;
    
    Result.Delete = false;
    Result.Add = true;
    Result.AddPos = Pos;
    Result.AddContent = CopyStringList(List);
    Result.AddLineBelow = LineBelow;
    
    return Result;
}

void
AddAction(buffer *Buffer, action Action)
{
    Buffer->ActionStack.Length = Buffer->ActionIndex + 1;
    ListAdd(&(Buffer->ActionStack), Action);
    Buffer->ActionIndex++;
}

void
UndoAction(program_state *ProgramState, buffer *Buffer, action A)
{
    if(A.Delete)
    {
        if(A.DeleteSingleLine)
        {
            Print("Re-placing line %d", A.DeleteStart.l);
            Print(A.DeleteContent[0]);
            ListInsert(&Buffer->Lines, A.DeleteStart.l, A.DeleteContent[0]);
        }
        else
        {
            buffer_pos Start = A.DeleteStart;
            buffer_pos End = A.DeleteEnd;
            Print("Re-placing Text at %d,%d: %d lines", Start.l, Start.c, A.DeleteContent.Length);
            InsertStringList(&Buffer->Lines, A.DeleteContent, Start.l, Start.c);
        }
    }
    if(A.Add)
    {
        //Buffer->Lines[A.AddPos.l].RemoveRange(A.AddPos.c, A.AddPos.c + A.AddContent.Length);
        // TODO: move cursor
        buffer_pos End = BufferPos(A.AddPos.l + A.AddContent.Length-1,
                                   A.AddContent[A.AddContent.Length-1].Length);
        Buffer->DeleteRange(A.AddPos, End);
        
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, &ProgramState->Views[ProgramState->SelectedViewIndex], A.AddPos);
    }
}

void
DoAction(program_state *ProgramState, buffer *Buffer, action A)
{
    if(A.Delete)
    {
        if(A.DeleteSingleLine)
        {
            Print("Deleting single line: %d", AnsiColor_Red, A.DeleteStart.l);
            ListRemoveAt(&Buffer->Lines, A.DeleteStart.l);
        }
        else
        {
            Buffer->DeleteRange(A.DeleteStart, A.DeleteEnd);
#if 0
            buffer_pos Start = A.DeleteStart;
            buffer_pos End = A.DeleteEnd;
            
            Print("");
            Print("Deleting content in range %d,%d to %d,%d:",
                  Start.l, Start.c, End.l, End.c);
            Print("=======================");
            for(int i = 0; i < A.DeleteContent.Length; i++)
            {
                Print("\t%S", A.DeleteContent[i]);
            }
            Print("=======================");
            Print("");
            
            
            // delete in-between lines
            for(int i = Start.l + 1; i < End.l; i++)
            {
                ListRemoveAt(&Buffer->Lines, i);
                End.l--;
                i--;
            }
            
            // slice and join next line
            if(End.l > Start.l)
            {
                if(Buffer->Lines[Start.l].Length > 0)
                    Buffer->Lines[Start.l].RemoveRange(Start.c, Buffer->Lines[Start.l].Length);
                if(Buffer->Lines[End.l].Length > 0)
                    Buffer->Lines[End.l].RemoveRange(0, End.c);
                Buffer->Lines[Start.l].AppendString(Buffer->Lines[End.l]);
                ListRemoveAt(&Buffer->Lines, End.l);
            }
            else
            {
                if(Buffer->Lines[Start.l].Length > 0)
                    Buffer->Lines[Start.l].RemoveRange(Start.c, End.c);
            }
#endif
        }
    }
    
    if(A.Add)
    {
        // TODO: does this work
#if 0
        for(int i = 0; i < A.AddContent.Length; i++)
        {
            int Col = Clamp(0, A.AddPos.c, LineLength(Buffer, A.AddPos.l + i) - 1);
            Buffer->Lines[A.AddPos.l + i].InsertString(Col, A.AddContent[i]);
            Buffer->Lines.Insert(A.AddPos.l + i + 1, String(""));
        }
#endif
        InsertStringList(&Buffer->Lines, A.AddContent, A.AddPos.l, A.AddPos.c);
        // TODO: move cursor
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, &ProgramState->Views[ProgramState->SelectedViewIndex],
                     BufferPos(A.AddPos.l, A.AddPos.c + A.AddContent.Length));
    }
}

void
AddAndDoAction(program_state *ProgramState, buffer *Buffer, action Action)
{
    AddAction(Buffer, Action);
    DoAction(ProgramState, Buffer, Action);
}

// undo: index
// redo: index + 1
// index can be -1, in which case we're at the bottom

void
MoveBackActionStack(program_state *ProgramState, buffer *Buffer)
{
    if(Buffer->ActionStack.Length > 0 && Buffer->ActionIndex > -1)
    {
        Print("Moving back action stack!");
        UndoAction(ProgramState, Buffer, Buffer->ActionStack[Buffer->ActionIndex]);
        Buffer->ActionIndex--;
    }
}

void
MoveForwardActionStack(program_state *ProgramState, buffer *Buffer)
{
    if(Buffer->ActionStack.Length > 0 && Buffer->ActionIndex < Buffer->ActionStack.Length - 1)
    {
        Print("Moving forward action stack!");
        DoAction(ProgramState, Buffer, Buffer->ActionStack[Buffer->ActionIndex + 1]);
        Buffer->ActionIndex++;
    }
}
