/* date = March 23rd 2024 2:30 pm */

#ifndef CCC_BASE_H
#define CCC_BASE_H

#define True 1
#define False 0

typedef float f32;
typedef double f64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int32_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef u32 b32;
typedef u64 b64;

typedef Color color;

#ifndef Assert
#define Assert(X) if(!X) { *0 = 0; }
#endif

#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 
#define IsAnyAltKeyDown (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) 
#define IsAnyControlKeyDown (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))

#define IsAlphaNumeric(c) (\
((c) >= 'a' && (c) <= 'z') || \
((c) >= 'A' && (c) <= 'Z') || \
((c) >= '0' && (c) <= '9')\
)
#define IsNonSpecial(c) (IsAlphaNumeric(c) || \
c == '_')


struct v2
{
    union
    {
        struct
        {
            int x, y;
        };
        struct
        {
            int w, h;
        };
    };
};
struct rect
{
    int x, y, w, h;
};

Rectangle
R(rect a)
{
    Rectangle Result = {(f32)a.x, (f32)a.y, (f32)a.w, (f32)a.h};
    return Result;
}



inline Vector2
V(v2 P)
{
    Vector2 Result = {(f32)P.x, (f32)P.y};
    return Result;
}

inline rect
Rect(int x, int y, int w, int h)
{
    rect Result;
    Result.x = x;
    Result.y = y;
    Result.w = w;
    Result.h = h;
    return Result;
}


struct buffer_pos {
    // line, col
    int l, c;
};


inline buffer_pos
BufferPos(int l, int c) {
    buffer_pos Result;
    Result.l = l;
    Result.c = c;
    return Result;
}





#endif //CCC_BASE_H
