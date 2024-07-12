inline buffer_pos
LargerBufferPos(buffer_pos A, buffer_pos B)
{
    if(A.l == B.l) return A.c > B.c ? A : B;
    return A.l > B.l ? A : B;
}

inline buffer_pos
SmallerBufferPos(buffer_pos A, buffer_pos B)
{
    if(A.l == B.l) return A.c < B.c ? A : B;
    return A.l < B.l ? A : B;
}

int 
LineLength(buffer *Buffer, int l)
{
    if(!Buffer)
        return 0;
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
    if(!Buffer)
        return 0;
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

