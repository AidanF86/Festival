/* date = September 12th 2023 9:45 pm */

#ifndef FESTIVAL_STRING_H
#define FESTIVAL_STRING_H

#include <cstdarg>
#include <cstring>

// UTF-32 string
struct string
{
    int Length;
    int ArraySize;
    u32 *Data;
    inline u32& operator[](size_t Index) { return Data[Index]; }
    
    u32 CharAt(int Index)
    {
        if(Index < 0 || Index >= Length)
        {
            // TODO: warning logging?
            printerror("Attempting to access out-of-bounds index");
            return 0;
        }
        return Data[Index];
    }
    void SetChar(int Index, u32 NewChar)
    {
        if(Index < 0 || Index >= Length)
        {
            // TODO: warning logging?
            printerror("Attempting to set out-of-bounds index");
            return;
        }
        Data[Index] = NewChar;
    }
    
    void InsertChar(int Index, u32 Char)
    {
        if(Index < 0 || Index > Length)
        {
            // TODO: warning logging?
            printerror("Trying to insert at an out-of-bounds index: %d", Index);
            return;
        }
        
        if(Length + 5 >= ArraySize)
        {
            printf("Reallocating string\n");
            ArraySize = ArraySize+64;
            Data = (u32 *)TryRealloc(Data, sizeof(u32) * ArraySize);
        }
        
        if(Length >= ArraySize)
        {
            printerror("Length is >= ArraySize (%d Length vs %d ArraySize)", Length, ArraySize);
            return;
        }
        
        // shift right
        for(int i = Length; i > Index && i > 0; i--)
        {
            Data[i] = Data[i-1];
        }
        
        Data[Index] = Char;
        Length++;
    }
    
    void AppendChar(u32 Char)
    {
        InsertChar(Length, Char);
    }
    
    void RemoveChar(int Index)
    {
        if(Index < 0 || Index > Length)
            return;
        
        // shift left
        for(int i = Index; i < Length-1; i++)
        {
            Data[i] = Data[i+1];
        }
        
        Length--;
    }
    
    void RemoveCharFromEnd()
    {
        RemoveChar(Length);
    }
    
    // inc, ex
    void Slice(int Start, int End)
    {
        if(Start > End || Start < 0 || End > Length)
        {
            printwarning("Invalid slice range: %d-%d", Start, End);
            Assert(false);
            return;
        }
        for(int i = 0; i < Start; i++)
        {
            RemoveChar(0);
        }
        while(Length > End-Start)
        {
            RemoveChar(End - Start);
        }
    }
    
    // inc, ex
    void RemoveRange(int Start, int End)
    {
        if(Start > End || Start < 0 || End > Length)
        {
            printwarning("Invalid removal range: %d-%d", Start, End);
            return;
        }
        
        int OriginalLength = Length;
        for(int i = Start; i < End && i < OriginalLength; i++)
        {
            RemoveChar(Start);
        }
    }
    
    void InsertString(int Index, string Other)
    {
        for(int i = 0; i < Other.Length; i++)
        {
            InsertChar(i+Index, Other[i]);
        }
    }
    
    void AppendString(string Other)
    {
        for(int i = 0; i < Other.Length; i++)
        {
            AppendChar(Other.Data[i]);
        }
    }
    
    void PrependString(string Other)
    {
        for(int i = Other.Length - 1; i >= 0; i--)
        {
            InsertChar(0, Other.Data[i]);
        }
    }
    
    bool BeginningMatches(string Other)
    {
        for(int i = 0; i < Length && i < Other.Length; i++)
        {
            if(Data[i] != Other.Data[i]) return false;
        }
        return true;
    }
    
    void Free()
    {
        TryFree(Data);
    }
    
};

inline bool
operator==(string A, string B)
{
    if(A.Length != B.Length) return false;
    for(int i = 0; i < A.Length; i++)
    {
        if(A[i] != B[i]) return false;
    }
    return true;
}

inline bool
operator!=(string A, string B)
{
    return !(A == B);
}

int
NullTerminatedStringLength(const char *Contents)
{
    int Result = 0;
    while(*Contents++) { Result++; }
    return Result;
}

int
NullTerminatedStringLength(u32 *Contents)
{
    int Result = 0;
    while(*Contents++) { Result++; }
    return Result;
}

string
AllocString(int Length)
{
    string Result;
    
    Result.Length = Length;
    if(Length < 32)
        Result.ArraySize = 32;
    else
        Result.ArraySize = Length;
    
    Result.Data = (u32 *)TryMalloc(sizeof(u32) * Result.ArraySize);
    
    return Result;
}

string
CopyString(string S)
{
    string Copy = AllocString(S.Length);
    
    memcpy(Copy.Data, S.Data, sizeof(u32) * S.Length);
    Copy.Length = S.Length;
    Copy.ArraySize = Copy.Length;
    
    return Copy;
}

void
FreeString(string String)
{
    String.Free();
}

char *
RawString(string String)
{
    char *Result = (char *)TryMalloc(sizeof(char) * (String.Length + 1));
    for(int i = 0; i < String.Length; i++)
    {
        Result[i] = (char)String.Data[i];
    }
    Result[String.Length] = 0;
    return Result;
}

#if 0
string
_String(const char *Contents)
{
    string Result = AllocString(NullTerminatedStringLength(Contents));
    
    for(int i = 0; i < Result.Length; i++)
    {
        Result.Data[i] = (u32)(Contents[i]);
    }
    
    return Result;
}
#endif

string
__String(u32 *Contents)
{
    string Result = AllocString(NullTerminatedStringLength(Contents));
    
    for(int i = 0; i < Result.Length; i++)
    {
        Result.Data[i] = (u32)(Contents[i]);
    }
    
    return Result;
}

#define StringFormatBufferLength 1024
u32 StringFormatBuffer[StringFormatBufferLength];
u32 StringVarBuffer[StringFormatBufferLength];
char SprintfBuffer[StringFormatBufferLength];

int
_U32_Sprintf(u32 *Dest, const char *Format, va_list Args)
{
    int Length = vsprintf(SprintfBuffer, Format, Args);
    if(Length >= StringFormatBufferLength)
    {
        printf("Length >= StringFormatBufferLength");
    }
    int i;
    for(i = 0; i < Length; i++)
    {
        Dest[i] = (int)SprintfBuffer[i];
    }
    Dest[i] = 0;
    return Length;
}

int
Sprintf(u32 *Dest, const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    return _U32_Sprintf(Dest, Format, Args);
}

b32 StringDebug = false;
string
_String(const char *Format, va_list Args)
{
    StringFormatBuffer[0] = 0;
    if(StringDebug)
    {
        printf("String creation log\n");
        printf("-----------------------------\n");
    }
    int Index = 0;
    
    b32 NextIsVariable = false;
    while(*Format != 0)
    {
        if(*Format == '%')
        {
            NextIsVariable = true;
        }
        else
        {
            if(NextIsVariable)
            {
                if(StringDebug)
                {
                    printf("Var: ");
                    printf("%c, ", *Format);
                }
                // Do proper printing
                int Length = 0;
                switch(*Format)
                {
                    case 'd':
                    {
                        int Var = va_arg(Args, int);
                        Length = Sprintf(StringVarBuffer, "%d", Var);
                    }break;
                    case 'D':
                    {
                        int Var = va_arg(Args, u32);
                        Length = Sprintf(StringVarBuffer, "%ud", Var);
                    }break;
                    case 'f':
                    {
                        f64 Var = va_arg(Args, f64);
                        Length = Sprintf(StringVarBuffer, "%.3lf", Var);
                    }break;
                    case 's':
                    {
                        char *Var = va_arg(Args, char *);
                        Length = Sprintf(StringVarBuffer, "%s", Var);
                    }break;
                    case 'S':
                    {
                        if(StringDebug)
                            printf("type String, ");
                        string Var = va_arg(Args, string);
                        char *TempStr = RawString(Var);
                        if(StringDebug)
                            printf("value \"%s\"", TempStr);
                        Length = Sprintf(StringVarBuffer, "%s", TempStr);
                        TryFree(TempStr);
                    }break;
                    // TODO(cheryl): add our string (%S)
                    case 'v':
                    {
                        v2 Var = va_arg(Args, v2);
                        Length = Sprintf(StringVarBuffer, "(%.3f, %.3f)", Var.x, Var.y);
                    }break;
                    case 'r':
                    {
                        rect Var = va_arg(Args, rect);
                        Length = Sprintf(StringVarBuffer, "(%d, %d, %d, %d)",
                                         Var.x, Var.y, Var.w, Var.h);
                    }break;
                    case 'R':
                    {
                        Rectangle Var = va_arg(Args, Rectangle);
                        Length = Sprintf(StringVarBuffer, "(%f, %f, %f, %f)",
                                         Var.x, Var.y, Var.width, Var.height);
                    }break;
                }
                
                for(int i = 0; i < Length; i++)
                {
                    StringFormatBuffer[Index+i] = StringVarBuffer[i];
                }
                if(StringDebug)
                    printf("length %d\n", Length);
                Index += Length;//NullTerminatedStringLength(StringVarBuffer);
                StringFormatBuffer[Index] = 0;
            }
            else
            {
                StringFormatBuffer[Index] = (u32)(*Format);
                Index++;
                StringFormatBuffer[Index] = 0;
            }
            
            NextIsVariable = false;
        }
        
        Format++;
    }
    if(StringDebug)
        printf("-----------------------------\n\n\n");
    
    return __String(StringFormatBuffer);
}

string
String(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    return _String(Format, Args);
}

void
Print(string String)
{
    for(int i = 0; i < String.Length; i++)
    {
        // NOTE(cheryl): does stdout work on windows?
        putc(String.Data[i], stdout);
    }
    putc('\n', stdout);
}

void
Print(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    string Str = _String(Format, Args);
    Print(Str);
    FreeString(Str);
}

void
PrintFile(FILE *File, string String)
{
    for(int i = 0; i < String.Length; i++)
    {
        // NOTE(cheryl): does stdout work on windows?
        putc(String.Data[i], File);
    }
    putc('\n', stdout);
}

void
Print(FILE *File, const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    string Str = _String(Format, Args);
    PrintFile(File, Str);
    FreeString(Str);
}

//#define PrintError(...) Print("%s", AnsiColor_Red); Print(__VA_ARGS__); Print("%s", AnsiColor_Reset);

DefineList(string, String);



string_list TempStrings;

string
TempString(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    string Result = _String(Format, Args);
    
    ListAdd(&TempStrings, Result);
    return Result;
}

string
TempString(string Str)
{
    string Result = CopyString(Str);
    ListAdd(&TempStrings, Result);
    return Result;
}

// Janky backend nonsense
struct raw_string
{
    char *Data;
};
raw_string RawString(char *Data)
{
    raw_string Result;
    Result.Data = Data;
    return Result;
}
DefineList(raw_string, RawString);

raw_string_list TempRawStrings;

char *
TempRawString(string Str)
{
    char *Data = RawString(Str);
    ListAdd(&TempRawStrings, RawString(Data));
    return Data;
}

void
PurgeTempStrings()
{
    for(int i = 0; i < TempStrings.Length; i++)
    {
        FreeString(TempStrings[i]);
    }
    ListFree(&TempStrings);
    TempStrings = StringList();
    
    for(int i = 0; i < TempRawStrings.Length; i++)
    {
        TryFree(TempRawStrings[i].Data);
    }
    ListFree(&TempRawStrings);
    TempRawStrings = RawStringList();
}

string_list
CopyStringList(string_list A)
{
    string_list Result = StringList();
    for(int i = 0; i < A.Length; i++)
    {
        ListAdd(&Result, CopyString(A[i]));
    }
    return Result;
}

string_list
CopyStringListRange(string_list A, int StartRow, int StartCol, int EndRow, int EndCol)
{
    string_list Result = StringList();
    
    // if only part of one line
    if(StartRow == EndRow)
    {
        ListAdd(&Result, CopyString(A[StartRow]));
        if(A[StartRow].Length > 0)
            Result[0].Slice(StartCol, EndCol);
        return Result;
    }
    // if only one whole line
    if(StartCol == 0 && EndCol == 0 && EndRow == StartRow + 1)
    {
        ListAdd(&Result, CopyString(A[StartRow]));
        return Result;
    }
    
    Print("BBBBBBBBB");
    if(StartCol < A[StartRow].Length)
    {
        Print("111111.1");
        ListAdd(&Result, CopyString(A[StartRow]));
        Result[0].Slice(StartCol, Result[0].Length);
    }
    else if(A[StartRow].Length == 0 || StartCol == A[StartRow].Length)
    {
        Print("111111.2");
        ListAdd(&Result, String(""));
    }
    
    for(int i = 1; i < EndRow - StartRow; i++)
    {
        Print("2222222");
        ListAdd(&Result, CopyString(A[StartRow + i]));
    }
    
    if(EndCol > 0)
    {
        Print("3333333");
        ListAdd(&Result, CopyString(A[EndRow]));
        Result[Result.Length-1].Slice(0, Min(EndCol, Result[Result.Length-1].Length));
    }
    
    return Result;
}

void
InsertStringList(string_list *Bottom, string_list Top, int Row, int Col)
{
    int MoveLinePos = 0;
    
    Print("===================");
    for(int i = 0; i < Top.Length; i++)
    {
        if(i == 0)
        {
            Bottom->Data[Row].InsertString(Col, Top[0]);
            MoveLinePos = Col + Top[0].Length;
            /*
                        if(Top[0].Length == 0)
                            MoveLinePos = Bottom->Data[Row].Length;
            */
            //if(MoveLinePos
        }
        else
        {
            // TEARS
            ListInsert(Bottom, Row+i, CopyString(Bottom->Data[Row+i-1]));
            
            int AboveIndex = Row+i-1;
            int Index = Row+i;
            
#if 1
            if(Bottom->Data[AboveIndex].Length > 0)
                Bottom->Data[AboveIndex].Slice(0, Min(Bottom->Data[AboveIndex].Length, MoveLinePos));
            if(Bottom->Data[Index].Length > 0)
                Bottom->Data[Index].Slice(MoveLinePos, Bottom->Data[Index].Length);
#endif
            
            Print(Top[i]);
            Bottom->Data[Index].InsertString(0, Top[i]);
            MoveLinePos = Top[i].Length;
        }
    }
    Print("===================");
}

#endif //FESTIVAL_STRING_H
