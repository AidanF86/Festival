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

inline rect
operator+(rect R, v2 V)
{
    rect Result = {R.x + V.x, R.y + V.y, R.width, R.height};
    return Result;
}
inline rect
operator-(rect R, v2 V)
{
    rect Result = {R.x - V.x, R.y - V.y, R.width, R.height};
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
    Result.width *= Scalar;
    Result.height *= Scalar;
    return Result;
}

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
Rect(f32 x, f32 y, f32 w, f32 h)
{
    rect Result;
    Result.x = x;
    Result.y = y;
    Result.width = w;
    Result.height = h;
    return Result;
}

#define Clamp(Value, Min, Max) { if(Value < Min){Value = Min;} if(Value > Max){Value = Max;} }

#endif //FESTIVAL_MATH_H
