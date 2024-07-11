char *
GetTextEncodingType(char *Data)
{
    DetectObj *DetectObject;
    DetectObject = detect_obj_init();
    
    if(DetectObject == NULL)
    {
        printerror("Failed to allocate libchardet DetectObj");
        return NULL;
    }
    
    switch(detect_r(Data, strlen(Data), &DetectObject))
    {
        case CHARDET_OUT_OF_MEMORY : {
            printerror("On handle processing, occured out of memory");
            detect_obj_free (&DetectObject);
            return NULL;
        } break;
        case CHARDET_NULL_OBJECT : {
            printerror("2st argument of chardet() is must memory allocation with detect_obj_init API");
            return NULL;
        } break;
    }
    
    //printf ("encoding: %s, confidence: %f\n", DetectObject->encoding, DetectObject->confidence);
    
    int ResultLength = strlen(DetectObject->encoding) + 1;
    char *Result = (char *)TryMalloc(ResultLength * sizeof(char));
    Result[ResultLength - 1] = 0;
    strcpy(Result, DetectObject->encoding);
    
    detect_obj_free(&DetectObject);
    
    return Result;
}

u32 *
ConvertTextToUTF32(char *Data, const char *Encoding, u64 *FinalCharCount)
{
    printf("Converting text from %s to UTF-32...");
    iconv_t cd = iconv_open("UTF32", Encoding);
    if(cd == (iconv_t) -1)
    {
        if(errno == EINVAL)
            printerror("Conversion from %s to UTF-32 not available", Encoding);
        else
            printerror("unknown iconv_open issue");
        return NULL;
    }
    
    int DataLength = strlen(Data);
    int OutputBufferSize = (DataLength + 10) * sizeof(u32);
    char *Result = (char *)TryMalloc(OutputBufferSize);
    
    size_t InBytesLeft = DataLength;
    size_t OutBytesLeft = OutputBufferSize;
    
    for(int i = 0; i < OutputBufferSize; i++)
    {
        Result[i] = 0;
    }
    
    char *OutBuffer = Result;
    
    while(InBytesLeft > 0)
    {
        size_t nconv = iconv(cd, &Data, &InBytesLeft, &OutBuffer, &OutBytesLeft);
        
        if(nconv == (size_t) -1)
        {
            if(errno == EINVAL)
            {
                printerror("iconv EINVAL");
            }
            else
            {
                printerror("unknown iconv issue");
            }
        }
    }
    
    iconv_close(cd);
    
    
    size_t FinalSize = OutputBufferSize - OutBytesLeft;
    
    *FinalCharCount = (u64)(FinalSize / 4);
    u32 *FinalResult = (u32 *)TryRealloc(Result, FinalSize);
    
    print(AnsiColor_Green "Success" AnsiColor_Reset);
    
    return FinalResult;
}

char *
ConvertUTF32StringToEncoding(string Str, const char *Encoding, u64 *FinalCharCount)
{
    // TODO: does this edit the string?
    printf("Converting text from UTF-32 to %s...", Encoding);
    iconv_t cd = iconv_open(Encoding, "UTF32");
    if(cd == (iconv_t) -1)
    {
        if(errno == EINVAL)
            printerror("Conversion from UTF-32 to %s not available", Encoding);
        else
            printerror("unknown iconv_open issue");
        return NULL;
    }
    
    string StrCopy = CopyString(Str);
    StrCopy.AppendChar(0);
    char *InPointer = (char *)(StrCopy.Data);
    int InLength = StrCopy.Length;
    int OutputBufferSize = (InLength + 10) * sizeof(u32);
    // Worst-case: each codepoint takes up 4 chars
    char *Result = (char *)TryMalloc(OutputBufferSize);
    
    size_t InBytesLeft = InLength * sizeof(u32);
    size_t OutBytesLeft = OutputBufferSize;
    
    for(int i = 0; i < OutputBufferSize; i++)
    {
        Result[i] = 0;
    }
    
    char *OutBuffer = Result;
    
    while(InBytesLeft > 0)
    {
        size_t nconv = iconv(cd, &InPointer, &InBytesLeft, &OutBuffer, &OutBytesLeft);
        
        if(nconv == (size_t) -1)
        {
            if(errno == EINVAL)
            {
                printerror("iconv EINVAL");
                memmove(StrCopy.Data, InPointer, StrCopy.Length * sizeof(u32));
            }
            else
            {
                printerror("unknown iconv issue");
            }
        }
    }
    
    iconv_close(cd);
    
    size_t FinalSize = OutputBufferSize - OutBytesLeft;
    
    *FinalCharCount = (u64)(FinalSize);
    char *FinalResult = (char *)TryRealloc(Result, FinalSize);
    
    print(AnsiColor_Green "Success" AnsiColor_Reset);
    
    return FinalResult;
}
