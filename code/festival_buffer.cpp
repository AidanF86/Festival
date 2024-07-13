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

buffer *
GetBufferForPath(program_state *ProgramState, string PathString)
{
    // see if we already have a buffer
    
    string PathCopy = CopyString(PathString);
    AbsolutizePath(&PathCopy);
    int Index = -1;
    
    for(int i = 0; i < ProgramState->Buffers.Length; i++)
    {
        if(PathCopy == ProgramState->Buffers[i].DirPath)
        {
            Index = i;
            break;
        }
    }
    
    FreeString(PathCopy);
    
    if(Index != -1)
        return &ProgramState->Buffers[Index];
    return 0;
}


buffer
CreateEmptyBuffer()
{
    buffer Buffer = {0};
    
    Buffer.HasAssociatedFile = true;
    Buffer.FileEncoding = "UTF8";
    Buffer.ActionIndex = -1;
    Buffer.ActionStack = ActionList();
    Buffer.Lines = StringList();
    Buffer.Lines.Add(String(""));
    
    return Buffer;
}

buffer
LoadFileToBuffer(const char *RawPath)
{
    if(!RawPath || RawPath[0] == 0)
        return {0};
    
    string Path = String(RawPath);
    AbsolutizePath(&Path);
    string FileName = GetFileName(Path);
    
    char *FullRawPath = RawString(Path);
    
    printf("Loading file '%s'... ", RawPath);
    if(!FileExists(FullRawPath))
    {
        printerror("file \"%s\" doesn't exist", FullRawPath);
        return {0};
    }
    
    print(AnsiColor_Green "Success" AnsiColor_Reset);
    
    u32 FileSize = (u32)GetFileLength(FullRawPath);
    
    char *FileData = LoadFileText(FullRawPath);
    
    free(FullRawPath);
    
    buffer Buffer = {0};
    Buffer.FileName = FileName;
    Buffer.DirPath = GetDirOfFile(Path);
    Buffer.ActionIndex = -1;
    Buffer.ActionStack = ActionList();
    
    Buffer.FileEncoding = GetTextEncodingType(FileData);
    u64 CharCount = 0;
    u32 *FileDataUTF32 = ConvertTextToUTF32(FileData, Buffer.FileEncoding, &CharCount);
    
    Buffer.Lines = StringList();
    for(int i = 0; i < CharCount; i++)
    {
        int LineStart = i;
        // TODO: use AppendString
        for(; FileDataUTF32[i] != '\n' && i < CharCount; i++) {}
        
        ListAdd(&(Buffer.Lines), AllocString(i-LineStart));
        
        int IndexInLine = 0;
        for(int a = LineStart; a < i; a++)
        {
            Buffer.Lines[Buffer.Lines.Length-1].SetChar(IndexInLine, FileDataUTF32[a]);
            IndexInLine++;
        }
    }
    if(Buffer.Lines.Length == 0)
    {
        ListAdd(&Buffer.Lines, String(""));
    }
    
    if((Buffer.Lines[0])[0] == 65279)
    {
        Buffer.Lines[0].RemoveChar(0);
    }
    
    UnloadFileText(FileData);
    
    Buffer.HasAssociatedFile = true;
    
    
    return Buffer;
}

void
WriteBufferToAssociatedFile(buffer *Buffer)
{
    Print("Writing buffer to file %S", Buffer->FileName);
    if(!Buffer->HasAssociatedFile)
    {
        printwarning("Buffer doesn't have an associated file");
        return;
    }
    
    string Path = String("%S%S", Buffer->DirPath, Buffer->FileName);
    b32 Success = WriteStringListToFile(Buffer->Lines, Path, Buffer->FileEncoding);
    if(!Success)
    {
        printwarning("Unknown file write issue");
    }
    Path.Free();
}