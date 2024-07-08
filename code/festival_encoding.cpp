file_encoding
GetTextEncodingType(char *Data)
{
    DetectObj *DetectObject;
    DetectObject = detect_obj_init();
    
    if(DetectObject == NULL)
    {
        printerror("Failed to allocate libchardet DetectObj");
        return FileEncoding_Error;
    }
    
    switch(detect_r(Data, strlen(Data), &DetectObject))
    {
        case CHARDET_OUT_OF_MEMORY : {
            printerror("On handle processing, occured out of memory");
            detect_obj_free (&DetectObject);
            return FileEncoding_Error;
        } break;
        case CHARDET_NULL_OBJECT : {
            printerror("2st argument of chardet() is must memory allocation with detect_obj_init API");
            return FileEncoding_Error;
        } break;
    }
    
    printf ("encoding: %s, confidence: %f\n", DetectObject->encoding, DetectObject->confidence);
    
    detect_obj_free(&DetectObject);
    
    // Default: UTF-8
    return FileEncoding_UTF8;
}

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