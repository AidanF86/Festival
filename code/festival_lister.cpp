
void
CloseLister(program_state *ProgramState, view *View)
{
    lister *Lister = &View->Lister;
    ProgramState->InputMode = InputMode_Nav;
    
    View->ListerIsOpen = false;
    ListFree(&Lister->Rects);
    ListFree(&Lister->Entries);
    ListFree(&Lister->MatchingEntries);
    FreeString(Lister->Input);
    FreeString(Lister->InputLabel);
}


void
OpenEditFileLister(program_state *ProgramState, view *View, const char *Directory)
{
    View->Lister = Lister(ListerType_String);
    lister *Lister = &View->Lister;
    
    Lister->InputLabel = String("Open file: ");
    Lister->Input = String(Directory);
    AbsolutizePath(&Lister->Input);
    Lister->Purpose = ListerPurpose_EditFile;
    
    FilePathList FilesInDir = LoadDirectoryFiles(Directory);
    for(int i = 0; i < FilesInDir.count; i++)
    {
        b32 IsDirectory = false;
        
        lister_entry NewEntry;
        NewEntry.String = String(FilesInDir.paths[i]);
        AbsolutizePath(&NewEntry.String);
        if(DirectoryExists(TempRawString(NewEntry.String)))
        {
            IsDirectory = true;
            NewEntry.String.AppendChar('/');
        }
        
        string Name = TempString(FilesInDir.paths[i]);
        CleanUpPath(&Name);
        NewEntry.Name = GetFileName(Name);
        if(IsDirectory)
            NewEntry.Name.AppendChar('/');
        
        
        ListAdd(&Lister->Entries, NewEntry);
        ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
        ListAdd(&Lister->MatchingEntries, i);
    }
    UnloadDirectoryFiles(FilesInDir);
    
    View->ListerIsOpen = true;
    Lister->ShouldExecute = false;
}

// TODO: policy: all strings used in lister are uniquely allocated and freed upon closure

void
OpenSwitchBufferLister(program_state *ProgramState, view *View)
{
    View->Lister = Lister(ListerType_BufferPointer);
    lister *Lister = &View->Lister;
    
    Lister->InputLabel = String("Switch to Buffer: ");
    Lister->Input = String("");
    Lister->Purpose = ListerPurpose_SwitchBuffer;
    
    for(int i = 0; i < ProgramState->Buffers.Length; i++)
    {
        lister_entry NewEntry;
        NewEntry.Buffer = &ProgramState->Buffers[i];
        NewEntry.Name = CopyString(ProgramState->Buffers[i].DirPath);
        ListAdd(&Lister->Entries, NewEntry);
        ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
        ListAdd(&Lister->MatchingEntries, i);
    }
    
    View->ListerIsOpen = true;
    Lister->ShouldExecute = false;
}

void
OpenCommandLister(program_state *ProgramState, view *View)
{
    View->Lister = Lister(ListerType_Command);
    lister *Lister = &View->Lister;
    
    Lister->InputLabel = String("Run Command: ");
    Lister->Input = String("");
    Lister->Purpose = ListerPurpose_RunCommand;
    
    for(int i = 0; i < sizeof(Commands) / sizeof(command); i++)
    {
        lister_entry NewEntry;
        NewEntry.Command = Commands[i];
        NewEntry.Name = String(Commands[i].Name);
        ListAdd(&Lister->Entries, NewEntry);
        ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
        ListAdd(&Lister->MatchingEntries, i);
    }
    
    View->ListerIsOpen = true;
    Lister->ShouldExecute = false;
}

void
OpenSwitchFontTypeLister(program_state *ProgramState, view *View)
{
    View->Lister = Lister(ListerType_FontType);
    lister *Lister = &View->Lister;
    
    Lister->InputLabel = String("Open Font File: ");
    //Lister->Input = String("./data/fonts/");
    Lister->Input = String("");
    //AbsolutizePath(&Lister->Input);
    Lister->Purpose = ListerPurpose_SwitchFontType;
    
    FilePathList FilesInDir = LoadDirectoryFiles("./data/fonts/");
    for(int i = 0; i < FilesInDir.count; i++)
    {
        b32 IsDirectory = false;
        
        lister_entry NewEntry;
        NewEntry.String = String(FilesInDir.paths[i]);
        AbsolutizePath(&NewEntry.String);
        if(DirectoryExists(TempRawString(NewEntry.String)))
        {
            IsDirectory = true;
            NewEntry.String.AppendChar('/');
        }
        
        string Path = String(FilesInDir.paths[i]);
        AbsolutizePath(&Path);
        NewEntry.Name = GetFileName(Path);
        Path.Free();
        
        ListAdd(&Lister->Entries, NewEntry);
        ListAdd(&Lister->Rects, Rect(0, 0, 0, 0));
        ListAdd(&Lister->MatchingEntries, i);
    }
    UnloadDirectoryFiles(FilesInDir);
    
    View->ListerIsOpen = true;
    Lister->ShouldExecute = false;
    
}


void
ExecLister(program_state *ProgramState, view *View)
{
    Print("Exec lister!");
    lister *Lister = &View->Lister;
    switch(Lister->Purpose)
    {
        case ListerPurpose_EditFile: {
            Print("Purpose: Edit file");
            if(Lister->Type != ListerType_String) {
                Print("Lister is for Edit File but type isn't String!");
                break;
            }
            
            if(Lister->MatchingEntries.Length == 0)
            {
                // Create buffer for non-existant file
                buffer NewBuffer;
                NewBuffer.FileName = CopyString(Lister->Input);
                NewBuffer.DirPath = CopyString(Lister->Input);
                NewBuffer.Lines = StringList();
                ListAdd(&NewBuffer.Lines, String(""));
                ListAdd(&ProgramState->Buffers, NewBuffer);
                View->Buffer = &ProgramState->Buffers[ProgramState->Buffers.Length - 1];
            }
            else
            {
                // If selection is a directory, we'll open a new lister for opening files in that
                // directory, otherwise open the file
                
                string Selection = CopyString(Lister->Entries[Lister->MatchingEntries[Lister->SelectedIndex]].String);
                
                if(DirectoryExists(TempRawString(Selection)))
                {
                    Print("Switching to directory");
                    // We've selected a directory, so make a new lister for opening files in it
                    
                    Lister->ShouldExecute = false;
                    CloseLister(ProgramState, View);
                    
                    OpenEditFileLister(ProgramState, View, TempRawString(Selection));
                    
                    return;
                }
                else
                {
                    // Open file
                    char *RawFileName = TempRawString(Selection);
                    buffer *ExistingBuffer = GetBufferForPath(ProgramState, Selection);
                    
                    if(ExistingBuffer)
                    {
                        View->Buffer = ExistingBuffer;
                    }
                    else
                    {
                        ListAdd(&ProgramState->Buffers, LoadFileToBuffer(RawFileName));
                        View->Buffer = &ProgramState->Buffers[ProgramState->Buffers.Length - 1];
                    }
                }
            }
        } break;
        case ListerPurpose_SwitchBuffer: {
            Print("Purpose: Switch buffer");
            if(Lister->Type != ListerType_BufferPointer) {
                Print("Lister is for Switch buffer but type isn't Buffer Pointer!");
                break;
            }
            
            View->Buffer = Lister->Entries[Lister->MatchingEntries[Lister->SelectedIndex]].Buffer;
            
        } break;
        case ListerPurpose_SwitchFontType: {
            if(Lister->Type != ListerType_FontType) {
                Print("Lister is for Switch font type but type isn't FontType!");
                break;
            }
            
            SetNewFont(&ProgramState->Font, Lister->Entries[Lister->MatchingEntries[Lister->SelectedIndex]].String);
        } break;
        case ListerPurpose_RunCommand: {
            if(Lister->Type != ListerType_Command) {
                Print("Lister is for Switch font type but type isn't FontType!");
                break;
            }
            Lister->ShouldExecute = false;
            command CommandToExec = Lister->Entries[Lister->MatchingEntries[Lister->SelectedIndex]].Command;
            CloseLister(ProgramState, View);
            
            CommandToExec.Function(ProgramState, View);
            
            return;
            
        } break;
        default: {
            Print("Unknown lister purpose!");
        }
    }
    
    Lister->ShouldExecute = false;
    CloseLister(ProgramState, View);
}
