color
RGBA(int r, int g, int b, int a) {
    color Result;
    Result.r = (u8)r;
    Result.g = (u8)g;
    Result.b = (u8)b;
    Result.a = (u8)a;
    return Result;
}

color
RGB(int r, int g, int b) {
    return RGBA(r, g, b, 255);
}

/*
v2 GetCharDim(program_state *ProgramState, int Size)
{
    return V2(MeasureTextEx(ProgramState->FontMain.RFont, "_", Size, 0));
}
*/
v2 GetCharDim(program_state *ProgramState, font_type FontType)
{
    int Size = ProgramState->FontSize;
    return V2(MeasureTextEx(ProgramState->FontMonospace.RFont, "_", Size, 0));
    //return GetCharDim(ProgramState, ProgramState->FontSize);
}
v2 GetCharDim(program_state *ProgramState)
{
    return GetCharDim(ProgramState, FontType_Monospace);
}

line_data
LineData() {
    line_data Result = {0};
    Result.CharRects = RectList();
    return Result;
};

int 
LineLength(buffer *Buffer, int l)
{
    if(l < 0 || l >= Buffer->Lines.Length)
    {
        printerror("Line index out of bounds");
        return 0;
    }
    return Buffer->Lines.Data[l].Length;
}
int 
LineLength(view *View, int l)
{
    return LineLength(View->Buffer, l);
}
int
LineCount(buffer *Buffer)
{
    return Buffer->Lines.Length;
}
int
LineCount(view *View)
{
    if(!View->Buffer)
        return 0;
    return View->Buffer->Lines.Length;
}

inline rect
CharRectAt(view *View, int l, int c)
{
    if(c == LineLength(View, l))
        return View->LineDataList[l].EndLineRect;
    return View->LineDataList[l].CharRects[c];
}
inline rect
CharRectAt(view *View, buffer_pos p)
{
    return CharRectAt(View, p.l, p.c);
}

rect
LineRect(view *View, int l)
{
    if(l < 0 || l >= LineCount(View))
    {
        printerror("Line index out of bounds");
        return {0};
    }
    return View->LineDataList.Data[l].LineRect;
}

v2
CharToScreenSpace(view *View, v2 CharRect)
{
    v2 Result = CharRect;
    Result.x += View->TextRect.x;
    Result.y += View->TextRect.y;
    Result.y -= View->Y;
    return Result;
}
v2
ScreenToCharSpace(view *View, v2 ScreenRect)
{
    v2 Result = ScreenRect;
    Result.y += View->Y;
    Result.y -= View->TextRect.y;
    Result.x -= View->TextRect.x;
    return Result;
}
rect
CharToScreenSpace(view *View, rect CharRect)
{
    rect Result = CharRect;
    v2 V = CharToScreenSpace(View, V2(CharRect));
    Result.x = V.x;
    Result.y = V.y;
    return Result;
}
rect
ScreenToCharSpace(view *View, rect ScreenRect)
{
    rect Result = ScreenRect;
    v2 V = ScreenToCharSpace(View, V2(ScreenRect));
    Result.x = V.x;
    Result.y = V.y;
    return Result;
}

rect
ScreenRectAt(view *View, int l, int c)
{
    return CharToScreenSpace(View, CharRectAt(View, l, c));
}

rect
ScreenRectAt(view *View, buffer_pos Pos)
{
    return ScreenRectAt(View, Pos.l, Pos.c);
}

u32
CharAt(buffer *Buffer, int l, int c)
{
    if(l < 0 || l >= Buffer->Lines.Length)
    {
        printerror("Line index out of bounds: %d", l);
        return 0;
    }
    if(c < 0 || c > LineLength(Buffer, l))
    {
        printerror("Char index out of bounds: %d", c);
    }
    if(c == LineLength(Buffer, l))
        return 0;
    return Buffer->Lines[l].Data[c];
}
u32
CharAt(view *View, int l, int c)
{
    if(!View->Buffer)
    {
        printerror("View->Buffer is NULL");
        return 0;
    }
    return CharAt(View->Buffer, l, c);
}
u32
CharAt(buffer *Buffer, buffer_pos Pos)
{
    return CharAt(Buffer, Pos.l, Pos.c);
}
u32
CharAt(view *View, buffer_pos Pos)
{
    return CharAt(View, Pos.l, Pos.c);
}

void
InsertLine(buffer *Buffer, int l, string S)
{
    if(l > Buffer->Lines.Length - 1)
    {
        ListAdd(&Buffer->Lines, S);
    }
    else
    {
        ListInsert(&Buffer->Lines, l, S);
    }
}

line_data
LineDataAt(view *View, int l)
{
    if(l < 0 || l >= LineCount(View))
    {
        printerror("Line index out of bounds");
        return {0};
    }
    return View->LineDataList[l];
}

int
YToLine(view *View, int Y)
{
    int l;
    int PrevLineY = LineDataAt(View, 0).LineRect.y;
    for(l = 0; l < LineCount(View); l++)
    {
        line_data LineData = LineDataAt(View, l);
        int LineY = LineData.LineRect.y;
        
        if(LineY > Y)
        {
            if(l > 0)
                l--;
            break;
        }
    }
    
    if(l >= LineCount(View))
        return LineCount(View)-1;
    
    return l;
}

int
ColAt(program_state *ProgramState, view *View, buffer_pos P)
{
    int Col = 0;
    int PrevY = CharRectAt(View, BufferPos(P.l, 0)).y;
    
    for(int c = 1; c < LineLength(View, P.l) && c <= P.c; c++)
    {
        Col++;
        if(CharRectAt(View, BufferPos(P.l, c)).y > PrevY)
        {
            Col = ProgramState->SubLineOffset;
        }
        PrevY = CharRectAt(View, BufferPos(P.l, c)).y;
    }
    
    return Col;
}


buffer_pos
ClosestBufferPos(view *View, v2 P)
{ // P is in char space
    int l = YToLine(View, P.y);
    
    buffer_pos ClosestBufferP = BufferPos(l, 0);
    rect ClosestRect = CharRectAt(View, ClosestBufferP);
    v2 ClosestP = V2(ClosestRect.x+ClosestRect.w/2, ClosestRect.y+ClosestRect.h/2);
    
    for(int c = 0; c <= View->LineDataList[l].CharRects.Length; c++)
    {
        rect TestRect = CharRectAt(View, l, c);
        v2 TestP = V2(TestRect.x+TestRect.w/2, TestRect.y+TestRect.h/2);
        
        v2 Diff = TestP - P;
        v2 CompareDiff = ClosestP - P;
        if(abs(Diff.y) < abs(CompareDiff.y) ||
           ( !(abs(Diff.y) > abs(CompareDiff.y)) && abs(Diff.x) < abs(CompareDiff.x) )
           )
        {
            ClosestP = TestP;
            ClosestBufferP = BufferPos(l, c);
        }
    }
    
    return ClosestBufferP;
}

buffer_pos
ClosestBufferPos(view *View, rect Rect)
{
    return ClosestBufferPos(View, V2(Rect.x+Rect.w/2, Rect.y+Rect.h/2));
}

void
AdjustView(program_state *ProgramState, view *View)
{
    int CharHeight = ProgramState->FontSize;
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
            View->CursorPos.l = YToLine(View, TargetY) + 2;
            MovedCursorUpOrDown = true;
        }
        else if(View->CursorTargetRect.y > TargetY + View->TextRect.h - CharHeight)
        {
            View->CursorPos.l = YToLine(View, 
                                        TargetY + View->TextRect.h) - 2;
            MovedCursorUpOrDown = true;
        }
    }
    
    View->TargetY = TargetY;
    
    View->TargetY = Clamp(View->TargetY, 0, LineDataAt(View, LineCount(View)-1).EndLineRect.y);
    View->CursorPos.l = Clamp(View->CursorPos.l, 0, LineCount(View)-1);
    View->CursorPos.c = Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
    
    
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


font
LoadFont(program_state *ProgramState, int Size, const char *FileName)
{
    font LoadedFont = {0};
    
    u32 FontFileSize = 0;
    u8 *FontFileData = LoadFileData(FileName, &FontFileSize);
    if(FontFileData == NULL)
    {
        printerror("Couldn't load font file '%s'", FileName);
        return {0};
    }
    
    Font *RLoadedFont = &(LoadedFont.RFont);
    
    RLoadedFont->baseSize = Size;
    //RLoadedFont->glyphCount = 95;
    int GlyphCount = 250;
    RLoadedFont->glyphCount = GlyphCount;
    
    RLoadedFont->glyphs = LoadFontData(FontFileData, FontFileSize, RLoadedFont->baseSize, 0, GlyphCount, FONT_DEFAULT);
    if(RLoadedFont->glyphs == NULL)
    {
        printerror("Font glyphs couldn't be loaded");
        return {0};
    }
    
    Image Atlas = GenImageFontAtlas(RLoadedFont->glyphs, &RLoadedFont->recs, GlyphCount, RLoadedFont->baseSize, 4, 0);
    RLoadedFont->texture = LoadTextureFromImage(Atlas);
    
    UnloadImage(Atlas);
    UnloadFileData(FontFileData);
    
    
    int GlyphIndex = 0;
    for(int i = 0; i < 256; i++)
    {
        for(int a = 0; a < RLoadedFont->glyphCount; a++)
        {
            if(RLoadedFont->glyphs[a].value == i)
            {
                LoadedFont.AsciiGlyphIndexes[i] = a;
                break;
            }
        }
    }
    
    return LoadedFont;
}

void
LoadFonts(program_state *ProgramState)
{
    ProgramState->FontMonospace = LoadFont(ProgramState, ProgramState->FontSize, "./data/fonts/LiberationMono-Regular.ttf");
    ProgramState->FontSerif = LoadFont(ProgramState, ProgramState->FontSize, "./data/fonts/Georgia.ttf");
    ProgramState->FontSans = LoadFont(ProgramState, ProgramState->FontSize, "./data/fonts/HelveticaNeue-Regular.otf");
}

inline int
CodepointIndex(font *Font, u32 Codepoint)
{
    if(Codepoint < 256)
    {
        return Font->AsciiGlyphIndexes[Codepoint];
    }
    else
    {
        // TODO: make more efficient! Hash map?
        //Print("Searching for non-ascii char index (%d)", Codepoint);
        int GlyphIndex = -1;
        //Codepoint = Codepoint << 3 * 8;
        //Codepoint = Codepoint >> 3 * 8;
        
        if(Codepoint == 32)
            return 0;
        for(int i = 0; i < Font->RFont.glyphCount; i++)
        {
            if(Font->RFont.glyphs[i].value == Codepoint)
            {
                GlyphIndex = i;
                break;
            }
        }
        return GlyphIndex;
    }
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
        // TODO(cheryl): check if there are any views in existence (there shouldn't be)
        View.Id = 0;
        View.Area = 1;
        View.SpawnLocation = Location_Below;
        View.BirthOrdinal = 0;
    }
    else
    {
        // TODO(cheryl): test :3
        
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
ComputeTextRect(program_state *ProgramState, view *View)
{
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    
    View->TextRect.x = View->Rect.x + ProgramState->NumbersWidth*CharWidth + ProgramState->MarginLeft;
    View->TextRect.y = View->Rect.y + CharHeight;
    View->TextRect.w = View->Rect.w - (View->TextRect.x - View->Rect.x);
    View->TextRect.h = View->Rect.h - CharHeight;
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
FillLineData(view *View, program_state *ProgramState)
{
    line_data_list *DataList = &View->LineDataList;
    
    int MarginLeft = ProgramState->MarginLeft;
    int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
    int NumbersWidth = ProgramState->NumbersWidth;
    int SubLineOffset = ProgramState->SubLineOffset;
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    // TODO(cheryl): formalize char-exclusion-zone
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
    font *Font = &ProgramState->FontMonospace;
    if(ProgramState->FontType == FontType_Sans)
    {
        Font = &ProgramState->FontSans;
    }
    else if(ProgramState->FontType == FontType_Serif)
    {
        Font = &ProgramState->FontSerif;
    }
    
    int CharsProcessed = 0;
    for(int l = 0; l < LineCount(View); l++)
    {
        ListAdd(DataList, LineData());
        
        line_data *RectData = &(DataList->Data[l]);
        int x = 0;
        
        RectData->LineRect.x = x;
        RectData->LineRect.y = y;
        RectData->LineRect.w = View->TextRect.w;
        RectData->DisplayLines = 1;
        
        for(int c = 0; c < LineLength(View, l); c++)
        {
            CharsProcessed++;
            // Rect is within the space of textrect
            // so when drawing, offset by textrect.x and textrect.y
            // as well as buffer viewpos
            
            int GlyphIndex = CodepointIndex(Font, CharAt(View, BufferPos(l, c)));
            if(GlyphIndex >= 0)
            {
                GlyphInfo Info = Font->RFont.glyphs[GlyphIndex];
                
                if(x+Info.advanceX >= WrapPoint)
                {
                    x = SubLineOffset*CharWidth;
                    y += CharHeight;
                    RectData->DisplayLines++;
                }
                
                ListAdd(&(RectData->CharRects), Rect(x, y, CharWidth, CharHeight));
                
                x += Info.advanceX;
            }
            else
            {
                if(x+CharWidth >= WrapPoint)
                {
                    x = SubLineOffset*CharWidth;
                    y += CharHeight;
                    RectData->DisplayLines++;
                }
                
                ListAdd(&(RectData->CharRects), Rect(x, y, CharWidth, CharHeight));
                
                x += CharWidth;
            }
        }
        RectData->EndLineRect = Rect(x, y, CharWidth, CharHeight);
        
        y += CharHeight;
        
        RectData->LineRect.h = RectData->DisplayLines * CharHeight;
    }
    //Print("%d", CharsProcessed);
}



void
MoveCursorPos(program_state *ProgramState, view *View, buffer_pos dPos)
{
    ProgramState->UserMovedCursor = true;
    View->CursorPos += dPos;
    View->CursorPos.l = Clamp(View->CursorPos.l, 0, LineCount(View) - 1);
    View->CursorPos.c = Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
    
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
    if(Result.c <= 0 || (Result.c == 0 && Result.l == 0))
        return From;
    
    b32 StartedAtSpace = false;
    // TODO: go to prev line
    if(CharAt(View, Result + BufferPos(0, -1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        Print("Started at space");
        Result.c--;
        while(Result.c > 0 && CharAt(View, BufferPos(Result.l, Result.c - 1)))
        {
            Result.c--;
        }
        
        return Result;
    }
    
    b32 StartedAtSpecial = false;
    if(!IsNonSpecial(CharAt(View, Result)))
    {
        Print("Special");
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
    
    if(Result.c == LineLength(View->Buffer, Result.l))
        return From;
    
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
SeekLineBeginning(view *View, int L)
{
    return BufferPos(L, 0);
}
buffer_pos
SeekLineEnd(view *View, int L)
{
    return BufferPos(L, LineLength(View, L));
}

buffer_pos
SeekPrevEmptyLine(view *View, int L)
{
    int ResultLine = L;
    
    while(LineLength(View, ResultLine) == 0 && ResultLine > 0)
    {
        ResultLine--;
        if(LineLength(View, ResultLine) != 0)
            break;
    }
    
    while(ResultLine > 0)
    {
        ResultLine--;
        if(LineLength(View, ResultLine) == 0)
            break;
    }
    return BufferPos(ResultLine, 0);
}
buffer_pos
SeekNextEmptyLine(view *View, int L)
{
    int ResultLine = L;
    
    while(LineLength(View, ResultLine) == 0 && ResultLine < LineCount(View) - 1)
    {
        ResultLine++;
    }
    
    while(ResultLine < LineCount(View) - 1)
    {
        ResultLine++;
        if(LineLength(View, ResultLine) == 0)
            break;
    }
    return BufferPos(ResultLine, LineLength(View, ResultLine));
}

void
SetCursorPos(program_state *ProgramState, view *View, buffer_pos Pos)
{
    ProgramState->UserMovedCursor = true;
    View->CursorPos = Pos;
    View->CursorPos.l = Clamp(View->CursorPos.l, 0, LineCount(View));
    View->CursorPos.c = Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
}

void
SplitView(program_state *ProgramState, view_spawn_location Location)
{
    buffer *BufferToUse = ProgramState->Views.Data[ProgramState->SelectedViewIndex].Buffer;
    ListAdd(&ProgramState->Views, View(ProgramState, BufferToUse, ProgramState->Views.Data[ProgramState->SelectedViewIndex].Id, Location));
}
