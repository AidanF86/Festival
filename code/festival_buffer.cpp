inline buffer_pos
LargerBufferPos(buffer_pos A, buffer_pos B)
{
    if(A.l == B.l) return A.c > B.c ? A : B;
    return A.l > B.l ? A : B;
}

inline buffer_pos
SmallerBufferPos(buffer_pos A, buffer_pos B)
{
    if(A.l == B.l) return A.c < B.c ? A : B;
    return A.l < B.l ? A : B;
}
