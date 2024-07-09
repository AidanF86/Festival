#include <dlfcn.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include "raylib.h"

#include "festival_platform.h"
#include "festival_linux.h"

#define SO_FILE_NAME "festival.so"

time_t
LinuxGetFileLastWriteTime(const char *FileName)
{
    struct stat Buffer;
    if(stat(FileName, &Buffer))
    {
        // Failure
        printf("\nfstat failed with file \"%s\"!\n\n", FileName);
        return 0;
    }
    return Buffer.st_mtime;
}

void
LinuxUnloadProgramCode(linux_program_code *ProgramCode)
{
    printf("Unloading program code... ");
    if(ProgramCode->Code)
    {
        dlclose(ProgramCode->Code);
        ProgramCode->Code = 0;
        print(AnsiColor_Green "Success" AnsiColor_Reset);
    }
    else
    {
        printerror("Program code was null");
    }
    ProgramCode->UpdateAndRender = 0;
    ProgramCode->IsValid = false;
}

linux_program_code
LinuxLoadProgramCode(const char *FileName)
{
    printf("Loading program code... ");
    linux_program_code Result;
    Result.IsValid = true;
    Result.Code = dlopen(FileName, RTLD_NOW);
    
    char *Error = dlerror();
    if(Error)
    {
        Result.IsValid = false;
        printerror("dlopen failed: %s", Error);
    }
    else 
    {
        Result.LastWriteTime = LinuxGetFileLastWriteTime(FileName);
        
        Result.UpdateAndRender = (program_update_and_render *)
            dlsym(Result.Code, "ProgramUpdateAndRender");
        
        char *Error = dlerror();
        if(Error)
        {
            Result.UpdateAndRender = 0;
            Result.IsValid = false;
            printerror("dlsym failed: %s", Error);
        }
        
        //dlclose(Result.Code);
    }
    
    print(AnsiColor_Green "Success" AnsiColor_Reset);
    return Result;
}

int main()
{
    //printf(AnsiColor_Green, "STARTING PROGRAM\n\n", AnsiColor_Reset);
    
    linux_program_code ProgramCode = LinuxLoadProgramCode(SO_FILE_NAME);
    if(!ProgramCode.IsValid)
    {
        print(AnsiColor_Red, "Invalid program code - Exiting", AnsiColor_Reset);
        return 1;
    }
    
    printf("Initializing memory... ");
    
    program_memory Memory;
    Memory.Initialized = false;
    Memory.Size = Kilobytes(30);
    Memory.Data = malloc(Memory.Size);
    Memory.WindowHeight = 450;
    Memory.WindowWidth = 800;
    Memory.IsRunning = true;
    
    print(AnsiColor_Green "Success" AnsiColor_Reset);
    
    SetTraceLogLevel(LOG_ERROR);
    
    InitWindow(Memory.WindowWidth, Memory.WindowHeight, "Festival");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    SetExitKey(0);
    
    while(Memory.IsRunning)
    {
#if 1
        time_t SOLastWriteTime = LinuxGetFileLastWriteTime(SO_FILE_NAME);
        if(SOLastWriteTime != ProgramCode.LastWriteTime)
        {
            LinuxUnloadProgramCode(&ProgramCode);
            sleep(1);
            ProgramCode = LinuxLoadProgramCode(SO_FILE_NAME);
            if(!ProgramCode.IsValid)
                printerror("Failed to reload program code");
        }
#else
        if(IsKeyPressed(KEY_U))
        {
            if(ProgramCode.Code)
            {
                LinuxUnloadProgramCode(&ProgramCode);
                print("Unloading program code");
            }
            else
            {
                print("Code is already unloaded");
            }
        }
        if(IsKeyPressed(KEY_L))
        {
            ProgramCode = LinuxLoadProgramCode(SO_FILE_NAME);
            print("Loading code");
        }
#endif
        
        Memory.WindowWidth = GetScreenWidth();
        Memory.WindowHeight = GetScreenHeight();
        
        if(ProgramCode.UpdateAndRender)
        {
            ProgramCode.UpdateAndRender(&Memory);
        }
        else
        {
            print("UpdateAndRender is Null");
        }
    }
    
    return 0;
}