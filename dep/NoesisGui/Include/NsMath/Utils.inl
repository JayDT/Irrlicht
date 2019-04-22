////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <NsCore/Error.h>

#include <math.h>
#include <stdlib.h>


namespace Noesis
{

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsPow2(uint32_t x)
{
    return x == 0 ? false: ((x & (x - 1)) == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint32_t NearestPow2(uint32_t x)
{
    int32_t next = NextPow2(x);
    int32_t prev = PrevPow2(x);

    if (x - prev <= next - x)
    {
        return prev;
    }
    else
    {
        return next;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint32_t NextPow2(uint32_t x)
{
    /// Hacker's Delight (pg 48)
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);

    return x + 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint32_t PrevPow2(uint32_t x)
{
    /// Hacker's Delight (pg 47)
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);

    return x - (x >> 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsOne(float val, float epsilon)
{
    return fabsf(val - 1.0f) < epsilon;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsZero(float val, float epsilon)
{
    return fabsf(val) < epsilon;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool AreClose(float a, float b, float epsilon)
{
    return fabsf(a - b) < epsilon;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsNaN(float val)
{
    // Cannot use isnan() because with fast-math, GCC optimizes it out (seems to be fixed in GCC 6+)
    union { float f; uint32_t u; } ieee754 = { val };
    return ((ieee754.u & 0x7fffffff) > 0x7f800000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsNaN(double val)
{
    // Cannot use isnan() because with fast-math, GCC optimizes it out (seems to be fixed in GCC 6+)
    union { double f; uint64_t u; } ieee754 = { val };
    return ((ieee754.u & 0x7fffffffffffffff) > 0x7ff0000000000000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsInfinity(float val)
{
    // Cannot use isinf() because with fast-math, GCC optimizes it out (seems to be fixed in GCC 6+)
    union { float f; uint32_t u; } ieee754 = { val };
    return ((ieee754.u & 0x7fffffff) == 0x7f800000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsInfinity(double val)
{
    // Cannot use isinf() because with fast-math, GCC optimizes it out (seems to be fixed in GCC 6+)
    union { double f; uint64_t u; } ieee754 = { val };
    return ((ieee754.u & 0x7fffffffffffffff) == 0x7ff0000000000000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsPositiveInfinity(float val)
{
    // Cannot use isinf() because with fast-math, GCC optimizes it out (seems to be fixed in GCC 6+)
    union { float f; uint32_t u; } ieee754 = { val };
    return ieee754.u == 0x7f800000;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsPositiveInfinity(double val)
{
    // Cannot use isinf() because with fast-math, GCC optimizes it out (seems to be fixed in GCC 6+)
    union { double f; uint64_t u; } ieee754 = { val };
    return ieee754.u == 0x7ff0000000000000;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsNegativeInfinity(float val)
{
    // Cannot use isinf() because with fast-math, GCC optimizes it out (seems to be fixed in GCC 6+)
    union { float f; uint32_t u; } ieee754 = { val };
    return ieee754.u == 0xff800000;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline bool IsNegativeInfinity(double val)
{
    // Cannot use isinf() because with fast-math, GCC optimizes it out (seems to be fixed in GCC 6+)
    union { double f; uint64_t u; } ieee754 = { val };
    return ieee754.u == 0xfff0000000000000;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline int Trunc(float val)
{
    return static_cast<int>(val);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline int Round(float val)
{
    // In SSE4.1 _mm_round_ss could be used
    return static_cast<int>(val > 0.0f ? val + 0.5f : val - 0.5f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline float Floor(float val)
{
    // In SSE4.1 _mm_round_ss could be used
    return floorf(val);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline float Ceil(float val)
{
    // In SSE4.1 _mm_round_ss could be used
    return ceilf(val);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> const T& Max(const T& a, const T& b)
{
    return a < b ? b : a;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> const T& Min(const T& a, const T& b)
{
    return b < a ? b : a;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> T Clip(T val, T min, T max)
{
    NS_ASSERT(min <= max);
    return Min(Max(min, val), max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
float Lerp(float x, float y, float t)
{
    return x + t * (y - x);
}

}
