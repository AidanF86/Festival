
line_data
LineData() {
    line_data Result = {0};
    Result.CharRects = RectList();
    return Result;
};

// TODO: move UserMovedCursor into view?
void SetCursorPos(program_state *ProgramState, view *View, buffer_pos Pos)
{
    ProgramState->UserMovedCursor = true;
    View->CursorPos = Pos;
    View->CursorPos.l = Clamp(View->CursorPos.l, 0, View->LineCount());
    View->CursorPos.c = Clamp(View->CursorPos.c, 0, View->LineLength(View->CursorPos.l));
}

view
View(program_state *ProgramState, buffer *Buffer, int ParentId, view_spawn_location SpawnLocation, f32 Area)
{
    view View = {0};
    //View.FontType = FontType_Monospace;
    View.CursorPos.l = 0;
    View.CursorPos.c = 0;
    View.Buffer = Buffer;
    View.ParentId = ParentId;
    View.LineDataList = {0};
    
    if(ParentId == -1)
    {
        // TODO: check if there are any views in existence (there shouldn't be)
        View.Id = 0;
        View.Area = 1;
        View.SpawnLocation = Location_Below;
        View.BirthOrdinal = 0;
    }
    else
    {
        // TODO: test
        
        // Get a unique Id
        int Id = 0;
        for(; Id <= ProgramState->Views.Length; Id++)
        {
            b32 IsIdTaken = false;
            for(int a = 0; a < ProgramState->Views.Length; a++)
            {
                if(ProgramState->Views[a].Id == Id)
                    IsIdTaken = true;
            }
            if(!IsIdTaken)
                break;
        }
        
        View.Id = Id;
        View.SpawnLocation = SpawnLocation;
        View.Area = 0.5f; // Default
        
        int SiblingCount = 0;
        for(int i = 0; i < ProgramState->Views.Length; i++)
        {
            if(ProgramState->Views[i].ParentId == ParentId)
                SiblingCount++;
        }
        
        View.BirthOrdinal = SiblingCount;
    }
    
    return View;
}

view View(program_state *ProgramState, buffer *Buffer, int ParentId, view_spawn_location SpawnLocation)
{
    return View(ProgramState, Buffer, ParentId, SpawnLocation, 0.5f);
}


void
AdjustView(program_state *ProgramState, view *View)
{
    int CharHeight = ProgramState->Settings.Font.Size;
    buffer_pos CursorPos = View->CursorPos;
    int Y = View->Y;
    int TargetY = View->TargetY;
    
    rect CursorTargetRect = View->CursorTargetRect;
    b32 MovedCursorUpOrDown = false;
    
    if(ProgramState->UserMovedCursor && &ProgramState->Views[ProgramState->SelectedViewIndex] == View)
    { // Adjust based on cursor
        if(CursorTargetRect.y < TargetY)
        {
            TargetY = CursorTargetRect.y;
        }
        else if(CursorTargetRect.y > TargetY + View->TextRect.h - CharHeight)
        {
            TargetY = CursorTargetRect.y - View->TextRect.h + CharHeight;
        }
    }
    else
    { // Adjust based on view
        if(View->CursorTargetRect.y < TargetY)
        {
            View->CursorPos.l = View->YToLine(TargetY) + 2;
            MovedCursorUpOrDown = true;
        }
        else if(View->CursorTargetRect.y > TargetY + View->TextRect.h - CharHeight)
        {
            View->CursorPos.l = View->YToLine(TargetY + View->TextRect.h) - 2;
            MovedCursorUpOrDown = true;
        }
    }
    
    View->TargetY = TargetY;
    
    View->TargetY = Clamp(View->TargetY, 0, View->LineDataAt(View->LineCount()-1).EndLineRect.y);
    View->CursorPos.l = Clamp(View->CursorPos.l, 0, View->LineCount()-1);
    View->CursorPos.c = Clamp(View->CursorPos.c, 0, View->LineLength(View->CursorPos.l));
    
#if 0
    if(MovedCursorUpOrDown && ColAt(ProgramState, View, View->CursorPos) < View->IdealCursorCol)
    {
        int Diff = View->IdealCursorCol - ColAt(ProgramState, View, View->CursorPos);
        int DistToEnd = LineLength(View, View->CursorPos.l) - View->CursorPos.c;
        if(Diff > DistToEnd)
            Diff = DistToEnd;
        View->CursorPos.c += Diff;
    }
#endif
}


void
RemoveView(program_state *ProgramState, int Index)
{
    // TODO: decrement birth ordinal of children?
    
    view_list *Views = &ProgramState->Views;
    if(Views->Length <= 1)
    {
        // TODO: set view to no buffer?
        return;
    }
    
    int RemovedViewId = Views->Data[Index].Id;
    int RemovedViewParentId = Views->Data[Index].ParentId;
    view_spawn_location RemovedViewSpawnLocation = Views->Data[Index].SpawnLocation;
    
    ListRemoveAt(Views, Index);
    
    // find a suitable heir
    view *Heir = NULL;
    int HeirIndex = 0;
    int ChildCount = 0;
    int SmallestBirthOrdinal = 256;
    for(int i = 0; i < Views->Length; i++)
    {
        view *View = &Views->Data[i];
        if(View->ParentId == RemovedViewId)
        {
            ChildCount++;
            if(View->BirthOrdinal < SmallestBirthOrdinal)
            {
                Heir = View;
                HeirIndex = i;
                SmallestBirthOrdinal = View->BirthOrdinal;
            }
        }
    }
    
    if(Heir != NULL && ChildCount > 0)
    {
        Print("Has Heir");
        Heir->Id = RemovedViewId;
        Heir->ParentId = RemovedViewParentId;
        Heir->SpawnLocation = RemovedViewSpawnLocation;
        
        if(ProgramState->SelectedViewIndex == Index || ProgramState->SelectedViewIndex >= Views->Length)
        {
            // set to heir
            ProgramState->SelectedViewIndex = HeirIndex;
        }
    }
    if(ProgramState->SelectedViewIndex >= Views->Length)
    {
        // set to parent
        int ParentIndex = 0;
        for(int i = 0; i < Views->Length; i++)
        {
            view *View = &Views->Data[i];
            if(View->Id == RemovedViewParentId)
                ParentIndex = i;
        }
        ProgramState->SelectedViewIndex = ParentIndex;
    }
}






void
MoveCursorPos(program_state *ProgramState, view *View, buffer_pos dPos)
{
    ProgramState->UserMovedCursor = true;
    View->CursorPos += dPos;
    View->CursorPos.l = Clamp(View->CursorPos.l, 0, View->LineCount() - 1);
    View->CursorPos.c = Clamp(View->CursorPos.c, 0, View->LineLength(View->CursorPos.l));
    
    if(ProgramState->ShouldChangeIdealCursorCol)
    {
        // TODO: change this to a function argument
        ProgramState->ShouldChangeIdealCursorCol = false;
        View->IdealCursorCol = View->CursorPos.c;
    }
    else
    {
        View->CursorPos.c = View->IdealCursorCol;
    }
}


void
MoveBackNonWhitespace(program_state *ProgramState, view *View)
{
    b32 StartedAtSpace = false;
    if(View->CharAt(View->CursorPos) == ' ' ||
       View->CharAt(View->CursorPos - BufferPos(0, 1)) == ' ')
    {
        StartedAtSpace = true;
    }
    
    if(StartedAtSpace)
    {
        do {
            View->CursorPos -= BufferPos(0, 1);
        }while(View->CharAt(View->CursorPos) == ' ' && View->CursorPos.c > 0);
    }
    
    while(View->CharAt(View->CursorPos) != ' ' && View->CursorPos.c > 0)
    {
        View->CursorPos -= BufferPos(0, 1);
    }
}

void
MoveForwardNonWhitespace(program_state *ProgramState, view *View)
{
    b32 StartedAtSpace = false;
    if(View->CharAt(View->CursorPos) == ' ' ||
       View->CharAt(View->CursorPos + BufferPos(0, 1)) == ' ')
    {
        StartedAtSpace = true;
    }
    
    if(StartedAtSpace)
    {
        do {
            //View->CursorPos += BufferPos(0, 1);
            MoveCursorPos(ProgramState, View, BufferPos(0, 1));
        }while(View->CharAt(View->CursorPos) == ' ' && View->CursorPos.c < View->LineLength(View->CursorPos.l));
    }
    
    while(View->CharAt(View->CursorPos) != ' ' && View->CursorPos.c < View->LineLength(View->CursorPos.l))
    {
        //View->CursorPos += BufferPos(0, 1);
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
}

buffer_pos
SeekBackBorder(view *View, buffer_pos From)
{
    buffer_pos Result = From;
    if(View->CharAt(Result) == 0)
        Result.c--;
    if(Result.c <= 0 || (Result.c == 0 && Result.l == 0))
        return From;
    
    b32 StartedAtSpace = false;
    // TODO: go to prev line
    if(View->CharAt(Result + BufferPos(0, -1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        Print("Started at space");
        Result.c--;
        while(Result.c > 0 && View->CharAt(BufferPos(Result.l, Result.c - 1)))
        {
            Result.c--;
        }
        
        return Result;
    }
    
    b32 StartedAtSpecial = false;
    if(!IsNonSpecial(View->CharAt(Result)))
    {
        Print("Special");
        StartedAtSpecial = true;
    }
    
    if(StartedAtSpecial)
    {
        while(!IsNonSpecial(View->CharAt(Result)) && 
              Result.c < View->LineLength(Result.l))
            Result.c--;
        return Result;
    }
    
    char c = View->CharAt(Result);
    while(( c != ' ' && (IsNonSpecial(c)) )
          && Result.c < View->LineLength(Result.l))
    {
        Result.c--;
        c = View->CharAt(Result);
    }
    
    return Result;
}

buffer_pos
SeekForwardBorder(view *View, buffer_pos From)
{
    buffer_pos Result = From;
    
    if(Result.c == View->LineLength(Result.l))
        return From;
    
    b32 StartedAtSpace = false;
    if(View->CharAt(Result) == ' ' || View->CharAt(Result + BufferPos(0, 1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        do {
            Result.c++;
        }while(View->CharAt(Result) == ' ' && Result.c < View->LineLength(Result.l));
        
        return Result;
    }
    
    b32 StartedAtSpecial = false;
    if(!IsNonSpecial(View->CharAt(Result)))
    {
        StartedAtSpecial = true;
    }
    
    if(StartedAtSpecial)
    {
        while(!IsNonSpecial(View->CharAt(Result))
              && Result.c < View->LineLength(Result.l))
            Result.c++;
        return Result;
    }
    
    char c = View->CharAt(Result);
    while(( c != ' ' && (IsNonSpecial(c)) )
          && Result.c < View->LineLength(Result.l))
    {
        Result.c++;
        c = View->CharAt(Result);
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
    return Pos.c == View->LineLength(Pos.l);
}

buffer_pos
SeekLineBeginning(view *View, int L)
{
    return BufferPos(L, 0);
}
buffer_pos
SeekLineEnd(view *View, int L)
{
    return BufferPos(L, View->LineLength(L));
}

buffer_pos
SeekPrevEmptyLine(view *View, int L)
{
    int ResultLine = L;
    
    while(View->LineLength(ResultLine) == 0 && ResultLine > 0)
    {
        ResultLine--;
        if(View->LineLength(ResultLine) != 0)
            break;
    }
    
    while(ResultLine > 0)
    {
        ResultLine--;
        if(View->LineLength(ResultLine) == 0)
            break;
    }
    return BufferPos(ResultLine, 0);
}
buffer_pos
SeekNextEmptyLine(view *View, int L)
{
    int ResultLine = L;
    
    while(View->LineLength(ResultLine) == 0 && ResultLine < View->LineCount() - 1)
    {
        ResultLine++;
    }
    
    while(ResultLine < View->LineCount() - 1)
    {
        ResultLine++;
        if(View->LineLength(ResultLine) == 0)
            break;
    }
    return BufferPos(ResultLine, View->LineLength(ResultLine));
}



void
SplitView(program_state *ProgramState, view_spawn_location Location)
{
    buffer *BufferToUse = ProgramState->Views.Data[ProgramState->SelectedViewIndex].Buffer;
    ListAdd(&ProgramState->Views, View(ProgramState, BufferToUse, ProgramState->Views.Data[ProgramState->SelectedViewIndex].Id, Location));
}




void
FillLineData(view *View, settings *Settings)
{
    font *Font = &(Settings->Font);
    line_data_list *DataList = &View->LineDataList;
    
    int MarginLeft = Settings->TextMarginLeft;
    int NumbersWidth = Settings->LineNumberWidth;
    int SubLineOffset = Settings->TextSubLineOffset;
    v2 CharDim = GetCharDim(Font);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    // TODO: formalize char-exclusion-zone (right-margin?)
    int WrapPoint = View->TextRect.w - CharWidth;
    
    // DeAllocation
    if(DataList->IsAllocated)
    {
        // Deallocate all lists
        for(int i = 0; i < DataList->Length; i++)
        {
            if(DataList->Data[i].CharRects.IsAllocated)
            {
                ListFree(&(DataList->Data[i].CharRects));
            }
            else
            {
                Print("Unallocated rect list???\n");
            }
            
        }
        ListFree(DataList);
    }
    // Allocation
    *DataList = LineDataList();
    
    int y = 0;
    int CharsProcessed = 0;
    for(int l = 0; l < View->LineCount(); l++)
    {
        ListAdd(DataList, LineData());
        
        line_data *RectData = &(DataList->Data[l]);
        int x = 0;
        
        RectData->LineRect.x = x;
        RectData->LineRect.y = y;
        RectData->LineRect.w = View->TextRect.w;
        RectData->DisplayLines = 1;
        
        for(int c = 0; c < View->LineLength(l); c++)
        {
            CharsProcessed++;
            // Rect is within the space of textrect
            // so when drawing, offset by textrect.x and textrect.y
            // as well as buffer viewpos
            
            GlyphInfo Info = GetCharDrawInfo(Font, View->CharAt(l, c)).Glyph;
            
            if(x+Info.advanceX >= WrapPoint)
            {
                x = SubLineOffset*CharWidth;
                y += CharHeight;
                RectData->DisplayLines++;
            }
            
            ListAdd(&(RectData->CharRects), Rect(x, y, CharWidth, CharHeight));
            
            x += Info.advanceX;
            
        }
        RectData->EndLineRect = Rect(x, y, CharWidth, CharHeight);
        
        y += CharHeight;
        
        RectData->LineRect.h = RectData->DisplayLines * CharHeight;
    }
    //Print("%d", CharsProcessed);
}


void
ComputeTextRect(settings *Settings, view *View)
{
    v2 CharDim = GetCharDim(Settings);
    
    View->TextRect.x = View->Rect.x + Settings->LineNumberWidth*CharDim.x + Settings->TextMarginLeft;
    View->TextRect.y = View->Rect.y + CharDim.y;
    View->TextRect.w = View->Rect.w - (View->TextRect.x - View->Rect.x);
    View->TextRect.h = View->Rect.h - CharDim.y;
}

void
ComputeViewGeometry(program_state *ProgramState, view *View)
{
    // TODO
}



