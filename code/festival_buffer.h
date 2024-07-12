/* date = July 11th 2024 11:14 pm */

#ifndef FESTIVAL_BUFFER_H
#define FESTIVAL_BUFFER_H


struct buffer
{
    string FileName;
    string DirPath;
    char *FileEncoding;
    
    int ActionIndex;
    action_list ActionStack;
    
    string_list Lines;
};
DefineList(buffer, Buffer)


#endif //FESTIVAL_BUFFER_H
