/* date = October 5th 2023 3:42 pm */

#ifndef FESTIVAL_LISTS_H
#define FESTIVAL_LISTS_H

#define DefineList(snake_name, pascal_name) \
struct snake_name##_list\
{\
b32 IsAllocated;\
int Count;\
int ArraySize;\
snake_name *Data;\
inline snake_name& operator[](size_t Index) { return Data[Index]; }\
inline const snake_name& operator[](size_t Index) const { return Data[Index]; }\
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
}

#define ListFree(List) {\
free((List)->Data);\
(List)->Count = 0;\
}

#endif //FESTIVAL_LISTS_H
