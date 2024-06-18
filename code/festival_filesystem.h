/* date = December 31st 2023 5:51 pm */

#ifndef FESTIVAL_FILESYSTEM_H
#define FESTIVAL_FILESYSTEM_H

void
CleanUpPath(string *Path)
{
    // remove duplicate '/'s
    for(int i = 0; i < Path->Length - 1; i++)
    {
        if(Path->Data[i] == '/' && Path->Data[i+1] == '/')
        {
            Path->RemoveChar(i+1);
            i--;
        }
    }
    
    // remove beginning './'
    if(Path->Length >= 2 && Path->Data[0] == '.' && Path->Data[1] == '/')
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
    if(Path->Length == 0 || Path->Data[0] != '/')
    {
        Path->InsertChar(0, '/');
        string DirStr = String(GetWorkingDirectory());
        Path->PrependString(DirStr);
        FreeString(DirStr);
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
    
    for(int i = 0; i < ProgramState->Buffers.Count; i++)
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
    
    // TODO: check if file exists and is readable, etc
    if(!FileExists(FullRawPath))
        Print("ERROR: no such file exists!");
    
    u32 FileSize = (u32)GetFileLength(FullRawPath);
    Print("File size: %d", (int)FileSize);
    
    char *FileData = LoadFileText(FullRawPath);
    
    free(FullRawPath);
    
    buffer Buffer = {};
    Buffer.FileName = FileName;
    Buffer.DirPath = GetDirOfFile(Path);
    Buffer.ActionIndex = -1;
    Buffer.ActionStack = ActionList();
    
    Buffer.Lines = StringList();
    printf("Gathering text\n");
    for(int i = 0; i < FileSize; i++)
    {
        int LineStart = i;
        // TODO: need to add AppendString to easily do this
        for(; FileData[i] != '\n' && i < FileSize; i++) {}
        
        ListAdd(&(Buffer.Lines), AllocString(i-LineStart));
        
        int InLine = 0;
        for(int a = LineStart; a < i; a++)
        {
            Buffer.Lines[Buffer.Lines.Count-1].Data[InLine] = FileData[a];
            InLine++;
        }
    }
    if(Buffer.Lines.Count == 0)
    {
        ListAdd(&Buffer.Lines, String(""));
    }
    printf("Finished with text\n");
    
    UnloadFileText(FileData);
    
    return Buffer;
}


#endif //FESTIVAL_FILESYSTEM_H
