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
    v2 Result = {(int)(V.x * Scalar), (int)(V.y * Scalar)};
    return Result;
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
    v2 Result = {(int)(V.x / Scalar), (int)(V.y / Scalar)};
    return Result;
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
    rect Result = {R.x + V.x, R.y + V.y, R.w, R.h};
    return Result;
}
inline rect
operator-(rect R, v2 V)
{
    rect Result = {R.x - V.x, R.y - V.y, R.w, R.h};
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

inline v2
V2(int x, int y)
{
    v2 Result = {x,y};
    return Result;
}
inline v2
V2(rect R)
{
    return V2(R.x, R.y);
}
inline v2
V2(Vector2 V)
{
    return V2(V.x, V.y);
}

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







/*
#define Interpolate(a, target, speed) \
( ((speed) >= 1) ? (target) :\
( (a) + ((target) - (a))*(speed) == (a) ? (target) :\
(a) + ((target) - (a))*(speed) ))
*/

#define InterpolateSpeedScalar 50

inline rect Interpolate(rect a, rect b, f32 speed)
{
    speed *= InterpolateSpeedScalar*GetFrameTime();
    if(speed >= 1)
        return b;
    rect Result = a + (b-a)*speed;
    if(Result == b)
        return Result;
    
    
    //f32 newX = (f32)a.x + ((f32)(b.x-a.x))*speed;
    //f32 newY = (f32)a.y + ((f32)(b.y-a.y))*speed;
    
    if(Result.x == a.x && a.x != b.x)
    {
        if(Result.x < b.x)
            Result.x++;
        else
            Result.x--;
    }
    if(Result.y == a.y && a.y != b.y)
    {
        if(Result.y < b.y)
            Result.y++;
        else
            Result.y--;
    }
    
    if(Result.w == a.w && a.w != b.w)
    {
        if(Result.w < b.w)
            Result.w++;
        else
            Result.w--;
    }
    if(Result.h == a.h && a.h != b.h)
    {
        if(Result.h < b.h)
            Result.h++;
        else
            Result.h--;
    }
    
    return Result;
}

inline int Interpolate(int a, int b, f32 speed)
{
    speed *= InterpolateSpeedScalar*GetFrameTime();
    if(speed >= 1)
        return b;
    int Result = a + (b-a)*speed;
    if(Result == a && a != b)
    {
        if(Result < b)
            Result++;
        else
            Result--;
    }
    return Result;
}

#define Clamp(Value, Min, Max) { if(Value < Min){Value = Min;} if(Value > Max){Value = Max;} }


u32
RandomU32InRange(u32 Min, u32 Max)
{
    // TODO: is this inclusive???
    f64 RandF = (f64)(rand()) / (f64)RAND_MAX;
    return (Max-Min)*RandF + Min;
}


#endif //FESTIVAL_MATH_H
