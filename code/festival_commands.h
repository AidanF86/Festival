/* date = December 22nd 2023 0:17 am */

#ifndef FESTIVAL_COMMANDS_H
#define FESTIVAL_COMMANDS_H

//typedef int (*command_function)(program_state*, view*);
#if 0
struct command
{
    command_function Function;
    const char *Name;
};
#endif

command
Command(command_function Function, const char *Name)
{
    command Result;
    Result.Function = Function;
    Result.Name = Name;
    return Result;
}


#define DefineCommand(name) int name##Command(program_state *ProgramState, view *View)


DefineCommand(TestCommand)
{
    Print("Hello!!! This is the test command!");
    return 0;
}

DefineCommand(SwitchFontType);
DefineCommand(EditFile);
DefineCommand(RandomSelectAndDeleteTest);

command Commands[3] =
{
    Command(SwitchFontTypeCommand, "Switch Font Type"),
    Command(EditFileCommand, "Open File"),
    Command(RandomSelectAndDeleteTestCommand, "Test Random Select and Delete"),
};


#endif //FESTIVAL_COMMANDS_H
