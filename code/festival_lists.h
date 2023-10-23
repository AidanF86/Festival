/* date = October 5th 2023 3:42 pm */

#ifndef FESTIVAL_LISTS_H
#define FESTIVAL_LISTS_H



rect_list RectList()
{
    rect_list Result;
    Result.Count = 0;
    Result.ArraySize = 20;
    Result.IsAllocated = true;
    Result.Data = (rect *)malloc(20 * sizeof(rect));
    return Result;
}

rect_list RectList(int Size)
{
    rect_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.IsAllocated = true;
    Result.Data = (rect *)malloc(Size * sizeof(rect));
    return Result;
}

int ListRemoveAt(rect_list *List, int Index)
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

/*======= line_data List =======*/
line_data_list LineDataList()
{
    line_data_list Result;
    Result.Count = 0;
    Result.ArraySize = 20;
    Result.IsAllocated = true;
    Result.Data = (line_data *)malloc(20 * sizeof(line_data));
    return Result;
}

line_data_list LineDataList(int Size)
{
    line_data_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.IsAllocated = true;
    Result.Data = (line_data *)malloc(Size * sizeof(line_data));
    return Result;
}

int ListRemoveAt(line_data_list *List, int Index)
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





color_list ColorList()
{
    color_list Result;
    Result.Count = 0;
    Result.ArraySize = 20;
    Result.Data = (color *)malloc(20 * sizeof(color));
    return Result;
}

color_list ColorList(int Size)
{
    color_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.Data = (color *)malloc(Size * sizeof(color));
    return Result;
}

int ListRemoveAt(color_list *List, int Index)
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

#endif //FESTIVAL_LISTS_H
