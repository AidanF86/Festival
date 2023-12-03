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
    
    void InsertChar(int Index, u32 Char)
    {
        if(Index < 0 || Index > Length)
            return;
        
        if(Length >= ArraySize)
        { // realloc
            int NewSize = (ArraySize+64)*sizeof(32);
            Data = (u32 *)realloc(Data, NewSize);
            ArraySize = NewSize;
        }
        
        // shift right
        for(int i = Length; i > Index; i--)
        {
            Data[i] = Data[i-1];
        }
        
        Data[Index] = Char;
        Length++;
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
    
    // inc, ex
    void Slice(int Start, int End)
    {
        for(int i = 0; i < Start; i++)
        {
            RemoveChar(0);
        }
        while(Length > End-Start)
        {
            RemoveChar(End - Start);
        }
    }
    
};

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
    Result.ArraySize = Length;
    Result.Data = (u32 *)malloc(sizeof(u32) * Result.ArraySize);
    
    return Result;
}

string
CopyString(string S)
{
    string Copy = AllocString(S.Length);
    
    memcpy(Copy.Data, S.Data, Copy.Length);
    Copy.Length = S.Length;
    Copy.ArraySize = Copy.Length;
    
    return Copy;
}

void
FreeString(string String)
{
    free(String.Data);
}

char *
RawString(string String)
{
    char *Result = (char *)malloc(sizeof(String.Length) + 1);
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

u32 StringFormatBuffer[1024];
u32 StringVarBuffer[128];
char SprintfBuffer[1024];

int
_U32_Sprintf(u32 *Dest, const char *Format, va_list Args)
{
    int Length = vsprintf(SprintfBuffer, Format, Args);
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
                        free(TempStr);
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
                    }
                    case 'R':
                    {
                        Rectangle Var = va_arg(Args, Rectangle);
                        Length = Sprintf(StringVarBuffer, "(%d, %d, %d, %d)",
                                         Var.x, Var.y, Var.width, Var.height);
                    }
                }
                
                for(int i = 0; i < Length; i++)
                {
                    StringFormatBuffer[Index+i] = StringVarBuffer[i];
                }
                //strcpy(&(StringFormatBuffer[Index]), StringVarBuffer);
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


/*
    Lists present:
    - string
*/

#include <stdlib.h>

#define ListDoubleSize(List) {\
(List)->ArraySize *= 2;\
(List)->Data = (typeof((List)->Data))realloc((List)->Data, sizeof((List)->Data[0]) * (List)->ArraySize);\
}

#define ListAdd(List, E) {\
if((List)->Count + 1 > (List)->ArraySize) { ListDoubleSize(List); }\
(List)->Data[(List)->Count] = (E);\
(List)->Count++;\
}

#define ListInsert(List, Index, E) {\
if((List)->Count + 1 > (List)->ArraySize) { ListDoubleSize(List); }\
for(int _i = (List)->Count; _i > Index; _i--)\
{ /* shift right */\
(List)->Data[_i] = (List)->Data[_i - 1];\
}\
(List)->Data[(Index)] = (E);\
}

#define ListFree(List) {\
free((List)->Data);\
}

/*======= string List =======*/
typedef struct string_list
{
    int Count;
    int ArraySize;
    string *Data;
    inline string& operator[](size_t Index) { return Data[Index]; }
    inline const string& operator[](size_t Index) const { return Data[Index]; }
} string_list;

string_list StringList()
{
    string_list Result;
    Result.Count = 0;
    Result.ArraySize = 20;
    Result.Data = (string *)malloc(20 * sizeof(string));
    return Result;
}

string_list StringList(int Size)
{
    string_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.Data = (string *)malloc(Size * sizeof(string));
    return Result;
}

int ListIndexOf(string_list *List, string E)
{
    int Result = -1;
    for(int i = 0; i < List->Count; i++)
    {
        if(List->Data[i] == E)
        {
            Result = i;
            break;
        }
    }
    return Result;
}

int ListRemoveAt(string_list *List, int Index)
{
    if(Index >= 0 && Index < List->Count)
    {
        for(int i = Index; i < List->Count; i++)
        {
            List->Data[i] = List->Data[i+1];
        }
        List->Count--;
        return 1;
    }
    return 0;
}
int ListRemove(string_list *List, string E)
{
    int Index = ListIndexOf(List, E);
    if(Index != -1)
    {
        ListRemoveAt(List, Index);
    }
    return Index;
}


#endif //FESTIVAL_STRING_H
