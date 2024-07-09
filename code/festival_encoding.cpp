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
    
    printf ("encoding: %s, confidence: %f\n", DetectObject->encoding, DetectObject->confidence);
    
    int ResultLength = strlen(DetectObject->encoding) + 1;
    char *Result = (char *)TryMalloc(ResultLength * sizeof(char));
    Result[ResultLength - 1] = 0;
    strcpy(Result, DetectObject->encoding);
    
    detect_obj_free(&DetectObject);
    
    return Result;
}

u32 *
ConvertTextToUTF32(char *Data, char *Encoding, u64 *FinalCharCount)
{
    print("Converting text from %s to UTF-32");
    print(Data);
    iconv_t cd = iconv_open("UTF32", Encoding);
    if(cd == (iconv_t) -1)
    {
        if(errno == EINVAL)
            printerror("Conversion from %s to UTF32 not available", Encoding);
        else
            printerror("iconv_open");
        return NULL;
    }
    
    int DataLength = strlen(Data);
    int OutputBufferSize = DataLength * sizeof(u32);
    char *Result = (char *)TryMalloc(OutputBufferSize);
    
    size_t InBytesLeft = DataLength;
    size_t OutBytesLeft = OutputBufferSize;
    
    print("InBytes: %ld", InBytesLeft);
    print("OutBytes: %ld", OutBytesLeft);
    
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
                printerror("iconv unknown issue");
            }
        }
    }
    
    iconv_close(cd);
    
    size_t FinalSize = OutputBufferSize - OutBytesLeft;
    *FinalCharCount = (u64)(FinalSize / 4);
    //print("%ld vs %ld", OutputBufferSize, FinalSize);
    // TODO: we're assuming this size aligns with 4 bytes, but we should probably check
    u32 *FinalResult = (u32 *)TryRealloc(Result, FinalSize);
    
    return FinalResult;
}
