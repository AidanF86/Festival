#if 1
color
RGBA(int r, int g, int b, int a) {
    color Result;
    Result.r = (u8)r;
    Result.g = (u8)g;
    Result.b = (u8)b;
    Result.a = (u8)a;
    return Result;
}

color
RGB(int r, int g, int b) {
    return RGBA(r, g, b, 255);
}


#endif