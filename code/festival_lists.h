/* date = October 5th 2023 3:42 pm */

#include <stdlib.h>

#ifndef CCC_BASE_H
#include "ccc_base.h"
#endif

#ifndef CCC_LIST_H
#define CCC_LIST_H


#define DefineList(snake_name, pascal_name) \
struct snake_name##_list\
{\
b32 IsAllocated;\
int Length;\
int ArraySize;\
snake_name *Data;\
inline snake_name& operator[](size_t Index) { return Data[Index]; }\
inline const snake_name& operator[](size_t Index) const { return Data[Index]; }\
\
snake_name ElementAt(int Index)\
{\
return Data[Index];\
}\
int Remove(int Index)\
{\
if(Index >= 0 && Index < Length)\
{\
for(int i = Index; i < Length; i++)\
{\
Data[i] = Data[i+1];\
}\
Length--;\
return 1;\
}\
return 0;\
}\
\
void DoubleSize()\
{\
ArraySize *= 2;\
Data = (snake_name*)realloc(Data, sizeof(snake_name) * ArraySize);\
}\
\
void Add(snake_name E)\
{\
if(Length + 1 > ArraySize) { DoubleSize(); }\
Data[Length] = E;\
Length++;\
}\
\
void Insert(int Index, snake_name E) {\
if(Length + 1 > ArraySize) { DoubleSize(); }\
for(int i = Length; i > Index; i--)\
{ /* shift right */\
Data[i] = Data[i-1];\
}\
Data[Index] = E;\
Length++;\
}\
\
void Free() {\
free(Data);\
Length = 0;\
}\
\
\
\
\
\
\
\
\
};\
\
snake_name##_list pascal_name##List(int Size)\
{\
snake_name##_list Result;\
Result.Length = 0;\
Result.ArraySize = Size;\
Result.IsAllocated = true;\
Result.Data = (snake_name *)TryMalloc(Size * sizeof(snake_name));\
return Result;\
}\
\
snake_name##_list pascal_name##List()\
{\
return pascal_name##List(20);\
}\
\
int ListRemoveAt(snake_name##_list *List, int Index)\
{\
if(Index >= 0 && Index < List->Length)\
{\
for(int i = Index; i < List->Length; i++)\
{\
List->Data[i] = List->Data[i+1];\
}\
List->Length--;\
return 1;\
}\
return 0;\
}\


#define ListDoubleSize(List) {\
(List)->ArraySize *= 2;\
(List)->Data = (typeof((List)->Data))TryRealloc((List)->Data, sizeof((List)->Data[0]) * (List)->ArraySize);\
}

#define ListAdd(List, E) {\
if((List)->Length + 1 > (List)->ArraySize) { ListDoubleSize(List); }\
(List)->Data[(List)->Length] = (E);\
(List)->Length++;\
}

#define ListInsert(List, Index, E) {\
if((List)->Length + 1 > (List)->ArraySize) { ListDoubleSize(List); }\
for(int _i = (List)->Length; _i > Index; _i--)\
{ /* shift right */\
(List)->Data[_i] = (List)->Data[_i - 1];\
}\
(List)->Data[(Index)] = (E);\
(List)->Length++;\
}

#define ListFree(List) {\
free((List)->Data);\
(List)->Length = 0;\
}




DefineList(int, Int)
DefineList(long, Long)
DefineList(bool, Bool)
DefineList(float, Float)
DefineList(double, Double)

DefineList(f32, F32)
DefineList(f64, F64)
DefineList(u8, U8)
DefineList(u16, U16)
DefineList(u32, U32)
DefineList(u64, U64)
DefineList(i8, I8)
DefineList(i16, I16)
DefineList(i32, I32)
DefineList(i64, I64)
DefineList(b32, B32)
DefineList(b64, B64)

DefineList(rect, Rect)
DefineList(v2, V2)
DefineList(color, Color)


#endif //CCC_LIST_H
