/* date = October 5th 2023 3:42 pm */

#ifndef FESTIVAL_LISTS_H
#define FESTIVAL_LISTS_H


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


/*======= view List =======*/
view_list ViewList()
{
    view_list Result;
    Result.Count = 0;
    Result.ArraySize = 20;
    Result.Data = (view *)malloc(20 * sizeof(view));
    return Result;
}

view_list ViewList(int Size)
{
    view_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.Data = (view *)malloc(Size * sizeof(view));
    return Result;
}

int ListRemoveAt(view_list *List, int Index)
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



lister_entry_list ListerEntryList(int Size)
{
    lister_entry_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.Data = (lister_entry *)malloc(Size * sizeof(lister_entry));
    return Result;
}

int ListRemoveAt(lister_entry_list *List, int Index)
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




int_list IntList(int Size)
{
    int_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.Data = (int *)malloc(Size * sizeof(int));
    return Result;
}

int ListIndexOf(int_list *List, int E)
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

int ListRemoveAt(int_list *List, int Index)
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
int ListRemove(int_list *List, int E)
{
    int Index = ListIndexOf(List, E);
    if(Index != -1)
    {
        ListRemoveAt(List, Index);
    }
    return Index;
}


#endif //FESTIVAL_LISTS_H
