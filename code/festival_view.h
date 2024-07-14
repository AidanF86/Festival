/* date = July 11th 2024 11:16 pm */

#ifndef FESTIVAL_VIEW_H
#define FESTIVAL_VIEW_H

struct line_data
{
    rect_list CharRects;
    color_list CharColors;
    // TODO: merge EndLineRect into CharRects?
    rect EndLineRect;
    rect LineRect;
    
    int DisplayLines;
};
DefineList(line_data, LineData)


enum view_spawn_location {
    Location_Below,
    Location_Right,
};

struct view
{
    buffer *Buffer;
    
    int Id;
    int ParentId; // -1 means this is root view
    view_spawn_location SpawnLocation;
    int BirthOrdinal; // [this]-th child. Determines placement and parental succession
    f32 Area; // fraction of parent
    b32 ComputedFromParentThisFrame;
    
    b32 HasComputedGeometry;
    rect TotalRect;
    rect BufferRect;
    rect TitlebarRect;
    rect LineNumberRect;
    rect ScrollbarAreaRect;
    rect ScrollbarRect;
    
    buffer_pos CursorPos;
    int IdealCursorCol;
    
    b32 Selecting;
    buffer_pos SelectionStartPos;
    
    string InsertModeString;
    buffer_pos InsertModeStartPos;
    b32 InsertModeLineBelow;
    
    int Y;
    int TargetY;
    rect CursorRect;
    rect CursorTargetRect;
    
    line_data_list LineDataList;
    
    b32 ListerIsOpen;
    lister Lister;
    
    u32 CharAt(int l, int c)
    {
        if(!Buffer)
        {
            printwarning("View doesn't have a buffer");
            return 0;
        }
        return Buffer->CharAt(l, c);
    }
    u32 CharAt(buffer_pos Pos)
    {
        return CharAt(Pos.l, Pos.c);
    }
    
    int LineCount()
    {
        if(!Buffer)
        {
            printwarning("View doesn't have a buffer");
            return 0;
        }
        return Buffer->LineCount();
    }
    int LineLength(int l)
    {
        if(!Buffer)
        {
            printwarning("View doesn't have a buffer");
            return 0;
        }
        return Buffer->LineLength(l);
    }
    
    line_data LineDataAt(int l)
    {
        if(l < 0 || l >= LineCount())
        {
            printerror("Line %d out of bounds", l);
            return {0};
        }
        return LineDataList[l];
    }
    rect CharRectAt(int l, int c)
    {
        if(c < 0 || c > LineLength(l))
        {
            printwarning("Col %d out of range", l);
            return {0};
        }
        
        // TODO: merge EndLineRect into CharRects (mentioned in view.h)
        if(c == LineLength(l))
            return LineDataAt(l).EndLineRect;
        return LineDataAt(l).CharRects[c];
    }
    rect CharRectAt(buffer_pos Pos)
    {
        return CharRectAt(Pos.l, Pos.c);
    }
    rect LineRectAt(int l)
    {
        return LineDataAt(l).LineRect;
    }
    
    v2 CharPosToScreenPos(v2 Pos)
    {
        v2 Result = Pos;
        Result.x += BufferRect.x;
        Result.y += BufferRect.y;
        Result.y -= Y;
        return Result;
    }
    v2 ScreenPosToCharPos(v2 Pos)
    {
        v2 Result = Pos;
        Result.y += Y;
        Result.y -= BufferRect.y;
        Result.x -= BufferRect.x;
        return Result;
    }
    rect CharRectToScreenRect(rect A)
    {
        rect Result = A;
        Result.x += BufferRect.x;
        Result.y += BufferRect.y;
        Result.y -= Y;
        return Result;
    }
    rect ScreenRectToCharRect(rect A)
    {
        rect Result = A;
        Result.y += Y;
        Result.y -= BufferRect.y;
        Result.x -= BufferRect.x;
        return Result;
    }
    
    rect ScreenRectAt(int l, int c)
    {
        return CharRectToScreenRect(CharRectAt(l, c));
    }
    rect ScreenRectAt(buffer_pos Pos)
    {
        return ScreenRectAt(Pos.l, Pos.c);
    }
    
    int YToLine(int Y)
    {
        int l;
        int PrevLineY = LineRectAt(0).y;
        // TODO: this is inefficient, maybe do a binary search
        for(l = 0; l < LineCount(); l++)
        {
            rect LineRect = LineRectAt(l);
            int LineY = LineRect.y;
            
            if(LineY > Y)
            {
                if(l > 0)
                    l--;
                break;
            }
        }
        
        if(l >= LineCount())
            return LineCount()-1;
        
        return l;
    }
    
    int ColAt(settings *Settings, buffer_pos P)
    {
        int Col = 0;
        int PrevY = CharRectAt(BufferPos(P.l, 0)).y;
        
        for(int c = 1; c < LineLength(P.l) && c <= P.c; c++)
        {
            Col++;
            if(CharRectAt(BufferPos(P.l, c)).y > PrevY)
            {
                Col = Settings->TextSubLineOffset;
            }
            PrevY = CharRectAt(BufferPos(P.l, c)).y;
        }
        
        return Col;
    }
    
    
    buffer_pos ClosestBufferPos(v2 P)
    { // P is in char space
        int l = YToLine(P.y);
        
        buffer_pos ClosestBufferP = BufferPos(l, 0);
        rect ClosestRect = CharRectAt(ClosestBufferP);
        v2 ClosestP = V2(ClosestRect.x+ClosestRect.w/2, ClosestRect.y+ClosestRect.h/2);
        
        for(int c = 0; c <= LineLength(l); c++)
        {
            rect TestRect = CharRectAt(l, c);
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
    
    buffer_pos ClosestBufferPos(rect Rect)
    {
        return ClosestBufferPos(V2(Rect.x+Rect.w/2, Rect.y+Rect.h/2));
    }
    
    
    void ComputeInternalGeometry(settings *Settings)
    {
        HasComputedGeometry = true;
        
        v2 CharDim = GetCharDim(Settings);
        
        // Text area
        BufferRect.x = TotalRect.x + Settings->LineNumberWidth*CharDim.x + Settings->TextMarginLeft;
        BufferRect.y = TotalRect.y + CharDim.y;
        BufferRect.w = TotalRect.w - (BufferRect.x - TotalRect.x);
        BufferRect.h = TotalRect.h - CharDim.y;
        
        // Scrollbar
        {
            v2 TopLeft = V2(TotalRect.x+TotalRect.w - Settings->ScrollbarWidth, BufferRect.y);
            v2 Dim = V2(Settings->ScrollbarWidth, BufferRect.h);
            
            int ViewportH = BufferRect.h;
            line_data LineData = LineDataAt(LineCount()-1);
            
            int TotalH = LineData.LineRect.y + (ViewportH);
            int AdditionalH = LineData.LineRect.h / LineData.DisplayLines * (LineData.DisplayLines - 1);
            TotalH += AdditionalH;
            
            f32 BarYPortion = (f32)Y / (f32)TotalH;
            int BarH = ((f32)ViewportH / (f32)TotalH) * (f32)Dim.h + 1;
            
            ScrollbarAreaRect = Rect(TopLeft.x, TopLeft.y, Dim.w, Dim.h);
            ScrollbarRect = Rect(TopLeft.x, TopLeft.y + BarYPortion*Dim.h, Dim.w, BarH);
        }
        
        // Titlebar
        TitlebarRect = Rect(TotalRect.x, TotalRect.y, TotalRect.w, CharDim.y);
    }
    
    
};
DefineList(view, View)



#endif //FESTIVAL_VIEW_H
