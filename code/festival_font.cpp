#if 0
void
LoadFont(font *Font, int Size, const char *Path)
{
    string FileName = GetFileName(TempString(Path));
    printf("Loading font '%S'...", FileName);
    FileName.Free();
    
    font LoadedFont = {0};
    
    u32 FontFileSize = 0;
    u8 *FontFileData = LoadFileData(Path, &FontFileSize);
    if(FontFileData == NULL)
    {
        printerror("Couldn't load font file '%s'", Path);
        return;
    }
    
    Font *RLoadedFont = &(LoadedFont.RFont);
    
    RLoadedFont->baseSize = Size;
    int GlyphCount = 255;
    RLoadedFont->glyphCount = GlyphCount;
    
    RLoadedFont->glyphs = LoadFontData(FontFileData, FontFileSize, RLoadedFont->baseSize, 0, GlyphCount, FONT_DEFAULT);
    if(RLoadedFont->glyphs == NULL)
    {
        printerror("Font glyphs couldn't be loaded");
        return;
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
    
    *Font = LoadedFont;
    
    print(AnsiColor_Green "Success" AnsiColor_Reset);
}
#endif

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
    
    return Result;
}





#if 0
GlyphInfo
GetGlyphForCodepoint(font *Font, u32 Codepoint)
{
    // TODO: optimize
    int GroupIndex = 0;
    for(int i = 0; i < sizeof(UnicodeGroups) / sizeof(unicode_group); i++)
    {
        unicode_group Group = UnicodeGroups[i];
        if(Codepoint > Group.Start && Codepoint < Group.End)
        {
            GroupIndex = i;
            break;
        }
    }
    
    if(!Font->GlyphGroups[GroupIndex].Loaded)
    {
        LoadGlyphGroup(Font, GroupIndex);
    }
    
    return Font->GlyphGroups[GroupIndex].RaylibFont.glyphs[Codepoint - UnicodeGroups[GroupIndex].Start];
}

Rectangle
GetRectForCodepoint(font *Font, u32 Codepoint)
{
    int GroupIndex = 0;
    for(int i = 0; i < sizeof(UnicodeGroups) / sizeof(unicode_group); i++)
    {
        unicode_group Group = UnicodeGroups[i];
        if(Codepoint > Group.Start && Codepoint < Group.End)
        {
            GroupIndex = i;
            break;
        }
    }
    
    if(!Font->GlyphGroups[GroupIndex].Loaded)
    {
        LoadGlyphGroup(Font, GroupIndex);
    }
    
    return Font->GlyphGroups[GroupIndex].RaylibFont.recs[Codepoint - UnicodeGroups[GroupIndex].Start];
}
#endif

v2 GetCharDim(program_state *ProgramState)
{
    return V2(ProgramState->Font.Size / 2, ProgramState->Font.Size);
}

