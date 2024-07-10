
rect
DrawChar(program_state *ProgramState, u32 Codepoint, v2 Pos, color BGColor, color FGColor)
{
    font *Font = &ProgramState->Font;
    
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
        v2 CharDim = GetCharDim(ProgramState);
        DestRect = Rect(Pos.x, Pos.y, CharDim.x, CharDim.y);
        rect DrawRect = Rect(DestRect.x + 2, DestRect.y + 2, DestRect.w - 4, DestRect.h - 4);
        //DrawRectangleLinesEx(R(DrawRect), 1, FGColor);
    }
    
    return DestRect;
}

void
DrawChar(program_state *ProgramState, int Char, v2 Pos, color FGColor)
{
    DrawChar(ProgramState, Char, Pos, RGBA(0, 0, 0, 0), FGColor);
}

#define Style_Underline 1
#define Style_Overline 2

v2
DrawString(program_state *ProgramState, string String, v2 Pos, color BGColor, color FGColor, int Style)
{
    font *Font = &ProgramState->Font;
    
    v2 OriginalPos = Pos;
    
    for(int i = 0; i < String.Length; i++)
    {
        DrawChar(ProgramState, String[i], Pos, BGColor, FGColor).w;
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
DrawString(program_state *ProgramState, string String, v2 Pos, color FGColor, int Style)
{
    return DrawString(ProgramState, String, Pos, RGBA(0, 0, 0, 0), FGColor, Style);
}

v2
DrawString(program_state *ProgramState, string String, v2 Pos, color FGColor)
{
    return DrawString(ProgramState, String, Pos, RGBA(0, 0, 0, 0), FGColor, 0);
}

v2
DrawString(program_state *ProgramState, string String, v2 Pos, color BGColor, color FGColor)
{
    return DrawString(ProgramState, String, Pos, BGColor, FGColor, 0);
}

void DrawProfiles(program_state *ProgramState) {
    int Y = 400;
    v2 CharDim = GetCharDim(ProgramState);
    int CharHeight = CharDim.y;
    for(int i = 0; i < ProfileNames.Length; i++)
    {
        double Total = 0;
        for(int a = 0; a < ProfileCycleFrameCount; a++)
        {
            Total += ProfileResultFrames[i][a];
        }
        string ProfileString = String("%S: %f", ProfileNames[i], Total/ProfileCycleFrameCount);
        DrawString(ProgramState, ProfileString, V2(400, 200+CharHeight*i), BLACK, RED);
        FreeString(ProfileString);
    }
}

void
DrawView(program_state *ProgramState, view *View)
{
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    b32 ViewIsSelected = View == &ProgramState->Views[ProgramState->SelectedViewIndex];
    rect ViewRect = View->Rect;
    rect TextRect = View->TextRect;
    
    BeginScissorMode(ViewRect.x, ViewRect.y, ViewRect.w, ViewRect.h);
    
    // draw background
    DrawRectangleRec(R(View->Rect), ProgramState->BGColor);
    
    // draw line numbers
    rect LineNumbersRect = Rect(ViewRect.x, View->TextRect.y, 4*CharWidth, View->TextRect.h);
    DrawRectangleRec(R(LineNumbersRect), ProgramState->LineNumberBGColor);
    
    // draw title bar
    // TODO: proper colors
    if(View->ListerIsOpen)
    {
        lister *Lister = &View->Lister;
        DrawRectangle(ViewRect.x, ViewRect.y, ViewRect.w, CharHeight, BLUE);
        int InputX = DrawString(ProgramState, Lister->InputLabel, V2(ViewRect.x, ViewRect.y), WHITE).x;
        DrawString(ProgramState, Lister->Input, V2(InputX, ViewRect.y),
                   WHITE);
    }
    else
    {
        DrawRectangle(ViewRect.x, ViewRect.y, ViewRect.w, CharHeight, GRAY);
        string TitleString = String("%S   %d,%d", View->Buffer->FileName, View->CursorPos.l, View->CursorPos.c);
        DrawString(ProgramState, TitleString, V2(ViewRect.x, ViewRect.y), BLACK);
        FreeString(TitleString);
    }
    
    BeginScissorMode(TextRect.x, TextRect.y, TextRect.w, TextRect.h);
    // TODO: Cross-view cursor interpolation
    // draw cursor
    rect CursorDrawRect = CharToScreenSpace(View, View->CursorRect);
    
    font *Font = &ProgramState->Font;
    
    char_draw_info CursorCharInfo = GetCharDrawInfo(Font, CharAt(View, View->CursorPos));
    if(CursorCharInfo.IsValid)
        CursorDrawRect.w = CursorCharInfo.Glyph.advanceX;
    
    color CursorColor = ProgramState->CursorBGColor;
    color SelectionAreaColor = ORANGE;
    if(!ViewIsSelected)
    {
        CursorColor = GRAY;
        SelectionAreaColor = LIGHTGRAY;
    }
    
    switch(ProgramState->InputMode)
    {
        case InputMode_Nav:
        {
            if(&ProgramState->Views[ProgramState->SelectedViewIndex] == View)
                DrawRectangleRec(R(CursorDrawRect), ProgramState->CursorBGColor);
            else
                DrawRectangleLinesEx(R(CursorDrawRect), 1, ProgramState->CursorBGColor);
        }
        break; case InputMode_Select:
        {
            rect SelectionStartDrawRect = CharToScreenSpace(View, CharRectAt(View, View->SelectionStartPos));
            DrawRectangleRec(R(SelectionStartDrawRect), RED);
            DrawRectangleRec(R(CursorDrawRect), RED);
        }
        break; case InputMode_Insert:
        {
            if(&ProgramState->Views[ProgramState->SelectedViewIndex] == View)
                DrawRectangleRec(R(Rect(CursorDrawRect.x, CursorDrawRect.y,
                                        CursorDrawRect.w/3, CursorDrawRect.h)),
                                 ProgramState->CursorBGColor);
            else
                DrawRectangleLinesEx(R(CursorDrawRect), 1, ProgramState->CursorBGColor);
        }
        break; case InputMode_EntryBar:
        {
            // TODO: don't draw buffer if we're in a lister?
            DrawRectangleLinesEx(R(CursorDrawRect), 2, ProgramState->CursorBGColor);
        }
        break;
    }
    EndScissorMode();
    
    // Draw text
    EndScissorMode();
    
    for(int l = 0; l < LineCount(View); l++)
    {
        line_data LineData = LineDataAt(View, l);
        int LineY = LineData.LineRect.y;
        
        if(LineY - View->Y > View->TextRect.y + View->TextRect.h)
        {
            break;
        }
        //if(LineRect(View, l).y + LineRect(View, l).h - View->Y < View->TextRect.y)
        if(LineRect(View, l).y + LineRect(View, l).h - View->Y < 0)
        {
            // TODO: it's this
            continue;
        }
        
        BeginScissorMode(LineNumbersRect.x, LineNumbersRect.y,
                         LineNumbersRect.w, LineNumbersRect.h);
        string NumberString = String("%d", l);
        v2 LineNumberPos = V2(View->Rect.x, LineData.LineRect.y - View->Y + View->TextRect.y);
        DrawString(ProgramState, NumberString, LineNumberPos, ProgramState->LineNumberFGColor);
        FreeString(NumberString);
        EndScissorMode();
        
        BeginScissorMode(TextRect.x, TextRect.y, TextRect.w, TextRect.h);
        for(int c = 0; c < LineData.CharRects.Length; c++)
        {
            
            rect ScreenCharRect = ScreenRectAt(View, l, c);
            
            color CharColor = ProgramState->FGColor;
            if(ViewIsSelected && BufferPos(l, c) == View->CursorPos && ProgramState->InputMode != InputMode_Insert)
            {
                CharColor = ProgramState->CursorFGColor;
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
            
            DrawChar(ProgramState, CharAt(View, l, c), V2(ScreenCharRect), CharBGColor, CharColor);
            
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
            DrawString(ProgramState, Lister->Entries[Lister->MatchingEntries[i]].Name,
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
        int InfoCharHeight = GetCharDim(ProgramState).y;
        int InfoCharWidth = GetCharDim(ProgramState).x;
        int KeyValueDistance = KeyValueDistanceChars * InfoCharWidth;
        
        DrawRectangleRec(R(View->Rect), {0, 0, 0, 200});
        {
            v2 TextPos = V2(View->Rect.x + 10, View->Rect.y + 10);
            DrawString(ProgramState, TempString("id"), TextPos, WHITE);
            DrawString(ProgramState, TempString("%d", View->Id), TextPos + V2(KeyValueDistance, 0), YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(ProgramState, TempString("parent"), TextPos, WHITE);
            DrawString(ProgramState, TempString("%d", View->ParentId), TextPos + V2(KeyValueDistance, 0), YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(ProgramState, TempString("birth #"), TextPos, WHITE);
            DrawString(ProgramState, TempString("%d", View->BirthOrdinal), TextPos + V2(KeyValueDistance, 0), YELLOW);
            TextPos.y += InfoCharHeight;
            DrawString(ProgramState, TempString("area"), TextPos, WHITE);
            DrawString(ProgramState, TempString("%d", View->Area), TextPos + V2(KeyValueDistance, 0), YELLOW);
        }
    }
    
    if(ProgramState->ShowViewRects)
    {
        DrawRectangleLinesEx(R(View->Rect), 2, {216, 50, 10, 255});
    }
    
    EndScissorMode();
}





void
DrawSuperDebugMenu(program_state *ProgramState)
{
    int Margin = 10;
    DrawRectangle(0, 0, ProgramState->ScreenWidth, ProgramState->ScreenHeight, WHITE);
    DrawRectangleLinesEx(R(Rect(0, 0, ProgramState->ScreenWidth, ProgramState->ScreenHeight)), Margin, YELLOW);
    int Y = -ProgramState->SuperDebugMenuY + Margin;
    int X = Margin;
    
    Y += 2 * DrawString(ProgramState, TempString("Super Debug Menu"), V2(X, Y), BLACK).y;
    
    Y += DrawString(ProgramState, TempString("Buffers"), V2(X, Y), BLACK, Style_Underline).y;
    for(int i = 0; i < ProgramState->Buffers.Length; i++)
    {
        buffer *Buffer = &ProgramState->Buffers[i];
        Y += DrawString(ProgramState, TempString("%S%S", Buffer->DirPath, Buffer->FileName), V2(X, Y), BLACK).y;
    }
    
    Y += DrawString(ProgramState, TempString("Views"), V2(X, Y), BLACK, Style_Underline).y;
    for(int i = 0; i < ProgramState->Views.Length; i++)
    {
        view *View = &ProgramState->Views[i];
        Y += DrawString(ProgramState, TempString("%d, parent=%d,", View->Id, View->ParentId), V2(X, Y), BLACK).y;
    }
    
    Y += 20;
    Y += DrawString(ProgramState, TempString("Font Texture"), V2(X, Y), BLACK, Style_Underline).y;
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

