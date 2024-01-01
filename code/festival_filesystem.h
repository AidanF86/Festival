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

#endif //FESTIVAL_FILESYSTEM_H
