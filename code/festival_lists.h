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
int Count;\
int ArraySize;\
snake_name *Data;\
inline snake_name& operator[](size_t Index) { return Data[Index]; }\
inline const snake_name& operator[](size_t Index) const { return Data[Index]; }\
\
int Remove(int Index)\
{\
if(Index >= 0 && Index < Count)\
{\
for(int i = Index; i < Count; i++)\
{\
Data[i] = Data[i+1];\
}\
Count--;\
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
if(Count + 1 > ArraySize) { DoubleSize(); }\
Data[Count] = E;\
Count++;\
}\
\
void Insert(int Index, snake_name E) {\
if(Count + 1 > ArraySize) { DoubleSize(); }\
for(int i = Count; i > Index; i--)\
{ /* shift right */\
Data[i] = Data[i-1];\
}\
Data[Index] = E;\
Count++;\
}\
\
void Free() {\
free(Data);\
Count = 0;\
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
Result.Count = 0;\
Result.ArraySize = Size;\
Result.IsAllocated = true;\
Result.Data = (snake_name *)malloc(Size * sizeof(snake_name));\
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
if(Index >= 0 && Index < List->Count)\
{\
for(int i = Index; i < List->Count; i++)\
{\
List->Data[i] = List->Data[i+1];\
}\
List->Count--;\
return 1;\
}\
return 0;\
}\


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
(List)->Count++;\
}

#define ListFree(List) {\
free((List)->Data);\
(List)->Count = 0;\
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
