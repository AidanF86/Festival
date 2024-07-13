
rect
DrawChar(settings *Settings, u32 Codepoint, v2 Pos, color BGColor, color FGColor)
{
    font *Font = &(Settings->Font);
    
    char_draw_info DrawInfo = GetCharDrawInfo(Font, Codepoint);
    
    rect DestRect = Rect(Pos.x + DrawInfo.Glyph.offsetX, Pos.y + DrawInfo.Glyph.offsetY,
                         DrawInfo.SrcRect.width,
                         DrawInfo.SrcRect.height);
    
    if(BGColor.a != 0)
    {
        rect BGRect = Rect(Pos.x, Pos.y,
                           Font->Size/2,
                           Font->Size);
        if(DrawInfo.Glyph.advanceX > Font->Size/2)
            BGRect.w = DrawInfo.Glyph.advanceX;
        DrawRectangleRec(R(BGRect), BGColor);
    }
    
    if(DrawInfo.IsValid)
    {
        DrawTexturePro(DrawInfo.GlyphGroup->RaylibFont.texture,
                       DrawInfo.SrcRect,
                       R(DestRect),
                       {0, 0}, 0, FGColor);
    }
    else
    {
        v2 CharDim = GetCharDim(Font);
        DestRect = Rect(Pos.x, Pos.y, CharDim.x, CharDim.y);
        rect DrawRect = Rect(DestRect.x + 2, DestRect.y + 2, DestRect.w - 4, DestRect.h - 4);
        //DrawRectangleLinesEx(R(DrawRect), 1, FGColor);
    }
    
    return DestRect;
}

void
DrawChar(settings *Settings, int Char, v2 Pos, color FGColor)
{
    DrawChar(Settings, Char, Pos, RGBA(0, 0, 0, 0), FGColor);
}

#define Style_Underline 1
#define Style_Overline 2

v2
DrawString(settings *Settings, string String, v2 Pos, color BGColor, color FGColor, int Style)
{
    font *Font = &(Settings->Font);
    
    v2 OriginalPos = Pos;
    
    for(int i = 0; i < String.Length; i++)
    {
        DrawChar(Settings, String[i], Pos, BGColor, FGColor).w;
        Pos.x += GetCharDrawInfo(Font, String[i]).Glyph.advanceX;
        //Print("%d", AdvanceX);
        
        /*
                int GlyphIndex = CodepointIndex(Font, String[i]);
                GlyphInfo Info = Font->RFont.glyphs[GlyphIndex];
                Pos.x += Info.advanceX;
        */
    }
    if(Style & Style_Underline)
    {
        v2 YDiff = V2(0, Font->Size - 3);
        DrawLineEx(V(OriginalPos + YDiff), V(Pos + YDiff), 2, FGColor);
    }
    return V2(Pos.x - OriginalPos.x, Pos.y + Font->Size - OriginalPos.y);
}

v2
DrawString(settings *Settings, string String, v2 Pos, color FGColor, int Style)
{
    return DrawString(Settings, String, Pos, RGBA(0, 0, 0, 0), FGColor, Style);
}

v2
DrawString(settings *Settings, string String, v2 Pos, color FGColor)
{
    return DrawString(Settings, String, Pos, RGBA(0, 0, 0, 0), FGColor, 0);
}

v2
DrawString(settings *Settings, string String, v2 Pos, color BGColor, color FGColor)
{
    return DrawString(Settings, String, Pos, BGColor, FGColor, 0);
}

void DrawProfiles(program_state *ProgramState) {
    int Y = 400;
    v2 CharDim = GetCharDim(&(ProgramState->Settings));
    int CharHeight = CharDim.y;
    for(int i = 0; i < ProfileNames.Length; i++)
    {
        double Total = 0;
        for(int a = 0; a < ProfileCycleFrameCount; a++)
        {
            Total += ProfileResultFrames[i][a];
        }
        string ProfileString = String("%S: %f", ProfileNames[i], Total/ProfileCycleFrameCount);
        DrawString(&(ProgramState->Settings), ProfileString, V2(400, 200+CharHeight*i), BLACK, RED);
        FreeString(ProfileString);
    }
}

void
DrawScrollbar(settings *Settings, view *View)
{
    // TODO: create function to work out all view geometry
    rect ViewRect = View->Rect;
    rect TextRect = View->TextRect;
    
    v2 TopLeft = V2(ViewRect.x+ViewRect.w - Settings->ScrollbarWidth, TextRect.y);
    v2 Dim = V2(Settings->ScrollbarWidth, TextRect.h);
    
    DrawRectangle(TopLeft.x, TopLeft.y, Dim.w, Dim.h, LIGHTGRAY);
    
    int ViewportH = View->TextRect.h;
    line_data LineData = View->LineDataAt(View->LineCount()-1);
    
    int TotalH = LineData.LineRect.y + (ViewportH);
    int AdditionalH = LineData.LineRect.h / LineData.DisplayLines * (LineData.DisplayLines - 1);
    TotalH += AdditionalH;
    
    f32 BarYPortion = (f32)View->Y / (f32)TotalH;
    int BarH = ((f32)ViewportH / (f32)TotalH) * (f32)Dim.h + 1;
    
    DrawRectangle(TopLeft.x, TopLeft.y + BarYPortion * (Dim.h), Dim.w, BarH, BLACK);
}

void
DrawView(program_state *ProgramState, view *View)
{
    settings *Settings = &(ProgramState->Settings);
    font *Font = &(Settings->Font);
    v2 CharDim = GetCharDim(Settings);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    b32 ViewIsSelected = View == &(ProgramState->Views[ProgramState->SelectedViewIndex]);
    rect ViewRect = View->Rect;
    rect TextRect = View->TextRect;
    
    BeginScissorMode(ViewRect.x, ViewRect.y, ViewRect.w, ViewRect.h);
    
    // draw background
    DrawRectangleRec(R(View->Rect), Settings->Colors.ViewBG);
    
    // draw line numbers
    rect LineNumbersRect = Rect(ViewRect.x, View->TextRect.y, 4*CharWidth, View->TextRect.h);
    DrawRectangleRec(R(LineNumbersRect), Settings->Colors.LineNumberBG);
    
    // draw title bar
    // TODO: proper colors
    if(View->ListerIsOpen)
    {
        lister *Lister = &View->Lister;
        DrawRectangle(ViewRect.x, ViewRect.y, ViewRect.w, CharHeight, BLUE);
        int InputX = DrawString(Settings, Lister->InputLabel, V2(ViewRect.x, ViewRect.y), WHITE).x;
        DrawString(Settings, Lister->Input, V2(InputX, ViewRect.y),
                   WHITE);
    }
    else
    {
        DrawRectangle(ViewRect.x, ViewRect.y, ViewRect.w, CharHeight, GRAY);
        string TitleString = String("%S   %d,%d", View->Buffer->FileName, View->CursorPos.l, View->CursorPos.c);
        DrawString(Settings, TitleString, V2(ViewRect.x, ViewRect.y), BLACK);
        FreeString(TitleString);
    }
    
    BeginScissorMode(TextRect.x, TextRect.y, TextRect.w, TextRect.h);
    // TODO: Cross-view cursor interpolation
    // draw cursor
    rect CursorDrawRect = View->CharRectToScreenRect(View->CursorRect);
    
    char_draw_info CursorCharInfo = GetCharDrawInfo(Font, View->CharAt(View->CursorPos));
    if(CursorCharInfo.IsValid)
        CursorDrawRect.w = CursorCharInfo.Glyph.advanceX;
    
    color CursorColor = Settings->Colors.CursorActiveBG;
    color SelectionAreaColor = Settings->Colors.SelectionAreaActiveBG;
    if(!ViewIsSelected)
    {
        CursorColor = Settings->Colors.CursorInactiveBG;
        SelectionAreaColor = Settings->Colors.SelectionAreaInactiveBG;
    }
    
    switch(ProgramState->InputMode)
    {
        case InputMode_Nav:
        {
            // TODO: view is active function
            // TODO: how to grapple with bg and outline?
            if(&ProgramState->Views[ProgramState->SelectedViewIndex] == View)
                DrawRectangleRec(R(CursorDrawRect), Settings->Colors.CursorActiveBG);
            else
                DrawRectangleLinesEx(R(CursorDrawRect), 1, Settings->Colors.CursorInactiveFG);
        }
        break; case InputMode_Select:
        {
            rect SelectionStartDrawRect = View->CharRectToScreenRect(View->CharRectAt(View->SelectionStartPos));
            DrawRectangleRec(R(SelectionStartDrawRect), RED);
            DrawRectangleRec(R(CursorDrawRect), RED);
        }
        break; case InputMode_Insert:
        {
            if(&ProgramState->Views[ProgramState->SelectedViewIndex] == View)
                DrawRectangleRec(R(Rect(CursorDrawRect.x, CursorDrawRect.y,
                                        CursorDrawRect.w/3, CursorDrawRect.h)),
                                 Settings->Colors.CursorActiveBG);
            else
                DrawRectangleLinesEx(R(CursorDrawRect), 1, Settings->Colors.CursorInactiveFG);
        }
        break; case InputMode_EntryBar:
        {
            // TODO: don't draw buffer if we're in a lister?
            DrawRectangleLinesEx(R(CursorDrawRect), 2, Settings->Colors.CursorInactiveFG);
        }
        break;
    }
    EndScissorMode();
    
    // Draw text
    EndScissorMode();
    
    for(int l = 0; l < View->LineCount(); l++)
    {
        line_data LineData = View->LineDataAt(l);
        int LineY = LineData.LineRect.y;
        
        if(LineY - View->Y > View->TextRect.y + View->TextRect.h)
        {
            break;
        }
        //if(LineRect(View, l).y + LineRect(View, l).h - View->Y < View->TextRect.y)
        if(View->LineRectAt(l).y + View->LineRectAt(l).h - View->Y < 0)
        {
            // TODO: it's this
            continue;
        }
        
        BeginScissorMode(LineNumbersRect.x, LineNumbersRect.y,
                         LineNumbersRect.w, LineNumbersRect.h);
        string NumberString = String("%d", l);
        v2 LineNumberPos = V2(View->Rect.x, LineData.LineRect.y - View->Y + View->TextRect.y);
        DrawString(Settings, NumberString, LineNumberPos, Settings->Colors.LineNumberFG);
        FreeString(NumberString);
        EndScissorMode();
        
        BeginScissorMode(TextRect.x, TextRect.y, TextRect.w, TextRect.h);
        for(int c = 0; c < LineData.CharRects.Length; c++)
        {
            
            rect ScreenCharRect = View->ScreenRectAt(l, c);
            
            color CharColor = Settings->Colors.TextFG;
            if(ViewIsSelected && BufferPos(l, c) == View->CursorPos && ProgramState->InputMode != InputMode_Insert)
            {
                CharColor = Settings->Colors.CursorActiveFG;
            }
            
            buffer_pos SmallerPos = View->SelectionStartPos.l < View->CursorPos.l || (View->SelectionStartPos.l == View->CursorPos.l && View->SelectionStartPos.c < View->CursorPos.c) ? View->SelectionStartPos : View->CursorPos;
            buffer_pos GreaterPos = SmallerPos == View->SelectionStartPos ? View->CursorPos : View->SelectionStartPos;
            bool CharIsSelected = (l > SmallerPos.l && l < GreaterPos.l)
                || (l == SmallerPos.l && l != GreaterPos.l && c > SmallerPos.c)
                || (l == GreaterPos.l && l != SmallerPos.l && c < GreaterPos.c)
                || (l == SmallerPos.l && l == GreaterPos.l && c > SmallerPos.c && c < GreaterPos.c);
            
            color CharBGColor = BLANK;
            if(View->Selecting && CharIsSelected)
            {
                CharBGColor = ORANGE;
            }
            
            DrawChar(Settings, View->CharAt(l, c), V2(ScreenCharRect), CharBGColor, CharColor);
            
        }
        EndScissorMode();
    }
    
    if(View->ListerIsOpen) {
        lister *Lister = &View->Lister;
        
        DrawRectangle(ViewRect.x, TextRect.y, ViewRect.w, Lister->MatchingEntries.Length*CharHeight, GRAY);
        
        int Y = TextRect.y;
        for(int i = 0; i < Lister->MatchingEntries.Length; i++) {
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
            DrawString(Settings, Lister->Entries[Lister->MatchingEntries[i]].Name,
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
        int InfoCharHeight = GetCharDim(Font).y;
        int InfoCharWidth = GetCharDim(Font).x;
        int KeyValueDistance = KeyValueDistanceChars * InfoCharWidth;
        
        DrawRectangleRec(R(View->Rect), {0, 0, 0, 200});
        {
            v2 TextPos = V2(View->Rect.x + 10, View->Rect.y + 10);
            DrawString(Settings, TempString("id"), TextPos, WHITE);
            DrawString(Settings, TempString("%d", View->Id), TextPos + V2(KeyValueDistance, 0), YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(Settings, TempString("parent"), TextPos, WHITE);
            DrawString(Settings, TempString("%d", View->ParentId), TextPos + V2(KeyValueDistance, 0), YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(Settings, TempString("birth #"), TextPos, WHITE);
            DrawString(Settings, TempString("%d", View->BirthOrdinal), TextPos + V2(KeyValueDistance, 0), YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(Settings, TempString("area"), TextPos, WHITE);
            DrawString(Settings, TempString("%d", View->Area), TextPos + V2(KeyValueDistance, 0), YELLOW);
        }
    }
    
    if(ProgramState->ShowViewRects)
    {
        DrawRectangleLinesEx(R(View->Rect), 2, {216, 50, 10, 255});
    }
    
    EndScissorMode();
    
    DrawScrollbar(Settings, View);
}





void
DrawSuperDebugMenu(program_state *ProgramState)
{
    settings *Settings = &(ProgramState->Settings);
    int Margin = 10;
    DrawRectangle(0, 0, ProgramState->ScreenWidth, ProgramState->ScreenHeight, WHITE);
    DrawRectangleLinesEx(R(Rect(0, 0, ProgramState->ScreenWidth, ProgramState->ScreenHeight)), Margin, YELLOW);
    int Y = -ProgramState->SuperDebugMenuY + Margin;
    int X = Margin;
    
    Y += 2 * DrawString(Settings, TempString("Super Debug Menu"), V2(X, Y), BLACK).y;
    
    Y += DrawString(Settings, TempString("Buffers"), V2(X, Y), BLACK, Style_Underline).y;
    for(int i = 0; i < ProgramState->Buffers.Length; i++)
    {
        buffer *Buffer = &ProgramState->Buffers[i];
        Y += DrawString(Settings, TempString("%S%S", Buffer->DirPath, Buffer->FileName), V2(X, Y), BLACK).y;
    }
    
    Y += DrawString(Settings, TempString("Views"), V2(X, Y), BLACK, Style_Underline).y;
    for(int i = 0; i < ProgramState->Views.Length; i++)
    {
        view *View = &ProgramState->Views[i];
        Y += DrawString(Settings, TempString("%d, parent=%d,", View->Id, View->ParentId), V2(X, Y), BLACK).y;
    }
    
    Y += 20;
    Y += DrawString(Settings, TempString("Font Texture"), V2(X, Y), BLACK, Style_Underline).y;
    /*
        DrawTexture(ProgramState->Font.RFont.texture, X, Y, BLACK);
        DrawRectangleLinesEx(R(Rect(X, Y,
                                    ProgramState->Font.RFont.texture.width,
                                    ProgramState->Font.RFont.texture.height)),
                             3, BLACK);
        Y += ProgramState->Font.RFont.texture.height;
    */
    
    ProgramState->SuperDebugMenuH = Y + ProgramState->SuperDebugMenuY;
}

