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

#if 1
u32 *
ConvertTextToUTF32(char *Data, char *Encoding)
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
    
    return (u32 *)Result;
}

#else
inline char
GetUTF8CodePointByteCount(char FirstByte)
{
    if(IsolateBitInByte(FirstByte, 0) == 0)
    {
        return 1;
    }
    else if(IsolateBitInByte(FirstByte, 0) == 1 &&
            IsolateBitInByte(FirstByte, 1) == 1 &&
            IsolateBitInByte(FirstByte, 2) == 0)
    {
        return 2;
    }
    else if(IsolateBitInByte(FirstByte, 0) == 1 &&
            IsolateBitInByte(FirstByte, 1) == 1 &&
            IsolateBitInByte(FirstByte, 2) == 1 &&
            IsolateBitInByte(FirstByte, 3) == 0)
    {
        return 3;
    }
    else if(IsolateBitInByte(FirstByte, 0) == 1 &&
            IsolateBitInByte(FirstByte, 1) == 0)
    {
        printerror("Trying to get UTF-8 codepoint byte count in a non-first byte");
        return 1;
    }
    return 4;
}

u32 *
ConvertUTF8ToUTF32(char *Data)
{
    // Allocate for largest scenario: every codepoint is 1 byte
    u32 *Result = (u32 *)TryMalloc(strlen(Data));
    
    int CodePointCount = 0;
    int Length = strlen(Data);
    for(int i = 0; i < Length; i++)
    {
        u32 CodePoint = 0;
        int CodePointByteCount = GetUTF8CodePointByteCount(Data[i]);
        if(CodePointByteCount == 1)
        {
            CodePoint = Data[i];
        }
        else if(CodePointByteCount == 2)
        {
            CodePoint |= Data[i] & 31;
            CodePoint << 6;
            CodePoint |= Data[i+1] & 63;
        }
        else if(CodePointByteCount == 3)
        {
            CodePoint |= Data[i] & 15;
            CodePoint << 6;
            CodePoint |= Data[i+1] & 63;
            CodePoint << 6;
            CodePoint |= Data[i+2] & 63;
        }
        else if(CodePointByteCount == 4)
        {
            CodePoint |= Data[i] & 7;
            CodePoint << 6;
            CodePoint |= Data[i+1] & 63;
            CodePoint << 6;
            CodePoint |= Data[i+2] & 63;
            CodePoint << 6;
            CodePoint |= Data[i+3] & 63;
        }
        Result[CodePointCount] = CodePoint;
        CodePointCount++;
        i += CodePointByteCount - 1;
    }
    
    // TODO: realloc to shrink Result accordingly
    
    return Result;
}

char *
ConvertUTF32ToUTF8(char *Data)
{
    return NULL;
}
#endif