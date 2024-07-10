inline v2 GetCharDim(program_state *ProgramState)
{
    return V2(ProgramState->Font.Size / 2, ProgramState->Font.Size);
}

void
LoadGlyphGroup(font *Font, int Index)
{
    glyph_group *GlyphGroup = &(Font->GlyphGroups[Index]);
    unicode_group UnicodeGroup = UnicodeGroups[Index];
    
    printf("Loading glyph group '%s'...", UnicodeGroup);
    
    int CodepointCount = UnicodeGroup.End - UnicodeGroup.Start;
    int *Codepoints = (int *)TryMalloc(sizeof(int) * CodepointCount);
    for(int i = UnicodeGroup.Start; i < UnicodeGroup.End; i++)
    {
        Codepoints[i - UnicodeGroup.Start] = i;
    }
    char *RawPath = RawString(Font->Path);
    GlyphGroup->RaylibFont = LoadFontEx(RawPath, Font->Size, Codepoints, CodepointCount);
    TryFree(RawPath);
    
    if(GlyphGroup->RaylibFont.glyphs == NULL)
    {
        printerror("Font glyphs couldn't be loaded");
        return;
    }
    
    GlyphGroup->Loaded = true;
    
    print(AnsiColor_Green "Success" AnsiColor_Reset);
}

char_draw_info
GetCharDrawInfo(font *Font, u32 Codepoint)
{
    // TODO: optimize
    int GroupIndex = -1;
    for(int i = 0; i < sizeof(UnicodeGroups) / sizeof(unicode_group); i++)
    {
        unicode_group Group = UnicodeGroups[i];
        if(Codepoint > Group.Start && Codepoint < Group.End)
        {
            GroupIndex = i;
            break;
        }
    }
    
    
    if(GroupIndex == -1)
    {
        char_draw_info Result = {0};
        Result.IsValid = false;
        Result.GlyphGroup = &(Font->GlyphGroups[0]);
        Result.Glyph = {0};
        Result.SrcRect = {0,0,(float)(Font->Size/2),(float)(Font->Size)};
        return Result;
    }
    
    if(!Font->GlyphGroups[GroupIndex].Loaded)
    {
        LoadGlyphGroup(Font, GroupIndex);
    }
    
    char_draw_info Result = {0};
    
    Result.IsValid = true;
    Result.GlyphGroup = &(Font->GlyphGroups[GroupIndex]);
    int Index = Codepoint - UnicodeGroups[GroupIndex].Start;
    Result.Glyph = Font->GlyphGroups[GroupIndex].RaylibFont.glyphs[Index];
    Result.SrcRect = Font->GlyphGroups[GroupIndex].RaylibFont.recs[Index];
    
    if(Result.Glyph.value != Codepoint)
    {
        Print("Codepoint wasn't correct!");
    }
    
    return Result;
}

void
SetNewFont(font *Font, string Path)
{
    Font->Path.Free();
    for(int i = 0; i < sizeof(Font->GlyphGroups) / sizeof(glyph_group); i++)
    {
        Font->GlyphGroups[i].Loaded = false;
    }
    Font->Path = CopyString(Path);
}