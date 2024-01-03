/* date = January 1st 2024 7:42 pm */

#ifndef FESTIVAL_UNDO_H
#define FESTIVAL_UNDO_H

struct action
{
    b32 Delete;
    // NOTE: end can be before start, the only difference is that end is where the cursor is
    // inc, ex
    buffer_pos DeleteStart;
    buffer_pos DeleteEnd;
    
    b32 Add;
    buffer_pos AddPos;
    string_list AddContent;
};

DefineList(action, Action);


action
ActionForDeleteRange(buffer_pos Start, buffer_pos End)
{
    action Result;
    
    Result.Delete = true;
    Result.DeleteStart = Start;
    Result.DeleteEnd = End;
    Result.Add = false;
    
    return Result;
}

action
ActionForInsertContent(buffer_pos Pos, string_list Content)
{
    action Result;
    
    Result.Delete = false;
    Result.Add = true;
    Result.AddPos = Pos;
    Result.AddContent = CopyStringList(Content);
    
    return Result;
}


#endif //FESTIVAL_UNDO_H
