
DefineCommand(SwitchFontType)
{
    OpenSwitchFontTypeLister(ProgramState, View);
    return 0;
}

DefineCommand(EditFile)
{
    OpenEditFileLister(ProgramState, View, "./");
    return 0;
}
DefineCommand(RandomSelectAndDeleteTest)
{
    for(int i = 0; i < 20; i++)
    {
        buffer *Buffer = ProgramState->Views[ProgramState->SelectedViewIndex].Buffer;
        
        u32 StartL = RandomU32InRange(0, Buffer->Lines.Count);
        u32 EndL = RandomU32InRange(0, Buffer->Lines.Count);
        buffer_pos Start = BufferPos(StartL, RandomU32InRange(0, Buffer->Lines[StartL].Length));
        buffer_pos End = BufferPos(EndL, RandomU32InRange(0, Buffer->Lines[EndL].Length));
        
        DoAndAddAction(Buffer, ActionForDeleteRange(Buffer, Start, End));
    }
    return 0;
}

