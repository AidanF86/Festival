/* date = July 11th 2024 11:14 pm */

#ifndef FESTIVAL_BUFFER_H
#define FESTIVAL_BUFFER_H

struct buffer
{
    b32 HasAssociatedFile;
    string FileName;
    string DirPath;
    const char *FileEncoding;
    
    int ActionIndex;
    action_list ActionStack;
    
    string_list Lines;
    
    
    
    int LineCount()
    {
        return Lines.Length;
    }
    int LineLength(int l)
    {
        if(l < 0 || l >= Lines.Length)
        {
            printerror("Line index out of bounds");
            return 0;
        }
        return Lines.Data[l].Length;
    }
    
    string LineAt(int l)
    {
        if(l < 0 || l >= Lines.Length)
        {
            printerror("Line index out of bounds: %d", l);
            return {0};
        }
        return Lines[l];
    }
    u32 CharAt(int l, int c)
    {
        if(c < 0 || c > LineLength(l))
        {
            printerror("Char index out of bounds: %d", c);
        }
        if(c == LineLength(l))
            return 0;
        return LineAt(l).Data[c];
    }
    u32 CharAt(buffer_pos Pos)
    {
        return CharAt(Pos.l, Pos.c);
    }
    
    
    void InsertLine(int l, string S)
    {
        if(l > Lines.Length - 1)
        {
            ListAdd(&Lines, S);
        }
        else
        {
            ListInsert(&Lines, l, S);
        }
    }
    
    void DeleteRange(buffer_pos Start, buffer_pos End)
    {
        Print("");
        
        // delete in-between lines
        for(int i = Start.l + 1; i < End.l; i++)
        {
            ListRemoveAt(&Lines, i);
            End.l--;
            i--;
        }
        
        // slice and join next line
        if(End.l > Start.l)
        {
            if(Lines[Start.l].Length > 0)
                Lines[Start.l].RemoveRange(Start.c, Lines[Start.l].Length);
            if(Lines[End.l].Length > 0)
                Lines[End.l].RemoveRange(0, End.c);
            Lines[Start.l].AppendString(Lines[End.l]);
            ListRemoveAt(&Lines, End.l);
        }
        else
        {
            if(Lines[Start.l].Length > 0)
                Lines[Start.l].RemoveRange(Start.c, End.c);
        }
    }
    void DeleteLine(int l)
    {
        // TODO: put functions in list struct
        if(l < 0 || l >= Lines.Length)
        {
            printerror("Line %d out of range", l);
            return;
        }
        ListRemoveAt(&Lines, l);
    }
};
DefineList(buffer, Buffer)


#endif //FESTIVAL_BUFFER_H
