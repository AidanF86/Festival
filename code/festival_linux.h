/* date = August 28th 2023 4:10 pm */

#ifndef FESTIVAL_LINUX_H
#define FESTIVAL_LINUX_H

struct linux_program_code
{
    void *Code;
    program_update_and_render *UpdateAndRender;
    
    time_t LastWriteTime;
    b32 IsValid;
};

#endif //FESTIVAL_LINUX_H
