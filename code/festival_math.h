/* date = August 27th 2023 11:46 am */

#ifndef FESTIVAL_MATH_H
#define FESTIVAL_MATH_H

#include "math.h"

inline v2
operator+(v2 A, v2 B)
{
    return {A.x + B.x, A.y + B.y};
}
inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A+B;
    return A;
}
inline v2
operator-(v2 A, v2 B)
{
    return {A.x - B.x, A.y - B.y};
}
inline v2 &
operator-=(v2 &A, v2 B)
{
    A = A-B;
    return A;
}
inline v2
operator*(v2 V, f32 Scalar)
{
    return {V.x * Scalar, V.y * Scalar};
}
inline v2 &
operator*=(v2 &A, f32 B)
{
    A = A*B;
    return A;
}
inline v2
operator/(v2 V, f32 Scalar)
{
    return {V.x / Scalar, V.y / Scalar};
}
inline v2 &
operator/=(v2 &A, f32 B)
{
    A = A/B;
    return A;
}
inline bool
operator<(v2 A, v2 B)
{
    return A.x < B.x && A.y < B.y;
}
inline bool
operator>(v2 A, v2 B)
{
    return A.x > B.x && A.y > B.y;
}

inline bool
operator==(rect a, rect b)
{
    bool Result = (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h);
    return Result;
}
inline rect
operator+(rect R, v2 V)
{
    rect Result = {R.x + (int)V.x, R.y + (int)V.y, R.w, R.h};
    return Result;
}
inline rect
operator-(rect R, v2 V)
{
    rect Result = {R.x - (int)V.x, R.y - (int)V.y, R.w, R.h};
    return Result;
}
inline rect &
operator+=(rect &R, v2 V)
{
    R = R + V;
    return R;
}
inline rect
operator*(rect R, f32 Scalar)
{
    rect Result = R;
    Result.x *= Scalar;
    Result.y *= Scalar;
    Result.w *= Scalar;
    Result.h *= Scalar;
    return Result;
}
inline rect
operator+(rect A, rect B)
{
    rect Result = {A.x+B.x, A.y+B.y, A.w+B.w, A.h+B.h};
    return Result;
}
inline rect
operator-(rect A, rect B)
{
    rect Result = {A.x-B.x, A.y-B.y, A.w-B.w, A.h-B.h};
    return Result;
}
inline rect &
operator+=(rect &R, rect B)
{
    R = R + B;
    return R;
}
#if 0
inline rect
InterpolateRect(rect R, rect Target, f32 Speed)
{
    rect Result = R;
    if(Speed >= 1)
        return Target;
    
    Result += (Target - R)*Speed;
    
    return Result;
}
#endif
#define Interpolate(a, target, speed) ( ((speed) >= 1) ? (target) : ( (a) + ((target) - (a))*(speed) == (a) ? (target) : (a) + ((target) - (a))*(speed) ))

inline v3
V2ToV3(v2 V)
{
    v3 Result;
    Result.x = V.x;
    Result.y = V.y;
    Result.z = 0;
    return Result;
}

inline v2
V3ToV2(v3 V)
{
    v2 Result;
    Result.x = V.x;
    Result.y = V.y;
    return Result;
}

v2 V2(f32 x, f32 y)
{
    v2 Result = {x,y};
    return Result;
}

v3 V3(f32 x, f32 y, f32 z)
{
    v3 Result = {x,y,z};
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

#define Clamp(Value, Min, Max) { if(Value < Min){Value = Min;} if(Value > Max){Value = Max;} }

struct buffer_pos {
    // line, col
    int l, c;
};

inline b32
operator==(buffer_pos a, buffer_pos b)
{
    b32 Result = a.l == b.l && a.c == b.c;
    return Result;
}
inline b32
operator!=(buffer_pos a, buffer_pos b)
{
    b32 Result = !(a==b);
    return Result;
}
inline buffer_pos
operator+(buffer_pos a, buffer_pos b)
{
    buffer_pos Result = {a.l + b.l, a.c + b.c};
    return Result;
}
inline buffer_pos &
operator+=(buffer_pos &a, buffer_pos b)
{
    a = a + b;
    return a;
}
inline buffer_pos
operator-(buffer_pos a, buffer_pos b)
{
    buffer_pos Result = {a.l - b.l, a.c - b.c};
    return Result;
}
inline buffer_pos &
operator-=(buffer_pos &a, buffer_pos b)
{
    a = a - b;
    return a;
}

inline buffer_pos
BufferPos(int l, int c) {
    buffer_pos Result;
    Result.l = l;
    Result.c = c;
    return Result;
}

#endif //FESTIVAL_MATH_H
