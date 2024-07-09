/* date = December 31st 2023 5:51 pm */

#ifndef FESTIVAL_FILESYSTEM_H
#define FESTIVAL_FILESYSTEM_H

void
CleanUpPath(string *Path)
{
    // remove duplicate '/'s
    for(int i = 0; i < Path->Length - 1; i++)
    {
        if(Path->CharAt(i) == '/' && Path->CharAt(i+1) == '/')
        {
            Path->RemoveChar(i+1);
            i--;
        }
    }
    
    // remove beginning './'
    if(Path->Length >= 2 && Path->CharAt(0) == '.' && Path->CharAt(1) == '/')
    {
        Path->RemoveChar(0);
        Path->RemoveChar(0);
    }
}

void
AbsolutizePath(string *Path)
{
    CleanUpPath(Path);
    
    // If Path doesn't start with '/', we assume it's relative to the working directory
    if(Path->Length == 0 || Path->CharAt(0) != '/')
    {
        Path->InsertChar(0, '/');
        
        string DirStr = String(GetWorkingDirectory());
        
        Path->PrependString(DirStr);
        
        DirStr.Free();
    }
}

string
GetDirOfFile(string Path)
{
    // TODO
    string Result = CopyString(Path);
    
    int LastSlashIndex = Path.Length - 1;
    for(; LastSlashIndex > 0; LastSlashIndex--)
    {
        if(Path[LastSlashIndex] == '/') break;
    }
    Result.Slice(0, LastSlashIndex + 1);
    
    return Result;
}

string
GetFileName(string Path)
{
    string Result = CopyString(Path);
    
    int LastSlashIndex = Path.Length - 1;
    for(; LastSlashIndex > 0; LastSlashIndex--)
    {
        if(Path[LastSlashIndex] == '/') break;
    }
    Result.Slice(LastSlashIndex + 1, Result.Length);
    
    return Result;
}

string
GetFileOrDirectoryName(string Path)
{
    string Result = CopyString(Path);
    
    int LastSlashIndex = Path.Length - 1;
    for(; LastSlashIndex > 0; LastSlashIndex--)
    {
        if(Path[LastSlashIndex] == '/') break;
    }
    Result.Slice(LastSlashIndex + 1, Result.Length);
    
    return Result;
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
LoadFileToBuffer(const char *RawPath)
{
    if(!RawPath || RawPath[0] == 0)
        return {};
    
    string FileName = String(RawPath);
    CleanUpPath(&FileName);
    
    string Path = String(RawPath);
    AbsolutizePath(&Path);
    
    char *FullRawPath = RawString(Path);
    
    printf("Loading file '%s'... ", RawPath);
    // TODO: check if file exists and is readable, etc
    if(!FileExists(FullRawPath))
    {
        printerror("file \"%s\" doesn't exist", FullRawPath);
        return {};
    }
    
    print(AnsiColor_Green "Success" AnsiColor_Reset);
    
    u32 FileSize = (u32)GetFileLength(FullRawPath);
    
    char *FileData = LoadFileText(FullRawPath);
    
    free(FullRawPath);
    
    buffer Buffer = {};
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
    
    
    return Buffer;
}


#endif //FESTIVAL_FILESYSTEM_H
