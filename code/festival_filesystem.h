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
    Print("ABSOLUTIZING");
    CleanUpPath(Path);
    
    // If Path doesn't start with '/', we assume it's relative to the working directory
    if(Path->Length == 0 || Path->CharAt(0) != '/')
    {
        Path->InsertChar(0, '/');
        
        string DirStr = String(GetWorkingDirectory());
        
        Print(*Path);
        Print("AAAA");
        Print(DirStr);
        
        Path->PrependString(DirStr);
        
        Print(*Path);
        
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
    Print(Path);
    AbsolutizePath(&Path);
    
    char *FullRawPath = RawString(Path);
    
    Print("(LoadFileBuffer): Loading file %s", FullRawPath);
    // TODO: check if file exists and is readable, etc
    if(!FileExists(FullRawPath))
    {
        printerror("file \"%s\" doesn't exist", FullRawPath);
        return {};
    }
    
    u32 FileSize = (u32)GetFileLength(FullRawPath);
    Print("\tFile load successful. Total size: %d", (int)FileSize);
    
    char *FileData = LoadFileText(FullRawPath);
    
    free(FullRawPath);
    
    buffer Buffer = {};
    Buffer.FileName = FileName;
    Buffer.DirPath = GetDirOfFile(Path);
    Buffer.ActionIndex = -1;
    Buffer.ActionStack = ActionList();
    
    // TODO: convert to fixed-width 32-bit unicode based on encoding
    
    Buffer.FileEncoding = GetTextEncodingType(FileData);
    u32 *FileDataUTF32 = ConvertUTF8ToUTF32(FileData);
    
    Buffer.Lines = StringList();
    for(int i = 0; i < FileSize; i++)
    {
        int LineStart = i;
        // TODO: use AppendString
        for(; FileData[i] != '\n' && i < FileSize; i++) {}
        
        ListAdd(&(Buffer.Lines), AllocString(i-LineStart));
        
        int InLine = 0;
        for(int a = LineStart; a < i; a++)
        {
            //Buffer.Lines[Buffer.Lines.Length-1].SetChar(InLine, FileData[a]);
            Buffer.Lines[Buffer.Lines.Length-1].SetChar(InLine, FileDataUTF32[a]);
            InLine++;
        }
    }
    if(Buffer.Lines.Length == 0)
    {
        ListAdd(&Buffer.Lines, String(""));
    }
    
    UnloadFileText(FileData);
    
    Print("\tFile buffer successfully set up");
    
    return Buffer;
}


#endif //FESTIVAL_FILESYSTEM_H
