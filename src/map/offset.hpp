/* Copyright 2016 Guillem Pascual */

#pragma once

#define _USE_MATH_DEFINES

#include <inttypes.h>
#include <math.h>
#include <algorithm>

#ifdef _MSC_VER
union HashConverter
{
    struct
    {
        uint32_t high;
        uint32_t low;
    }
    b32;
    uint64_t b64;
};
#endif

struct Offset
{
public:
    constexpr Offset(int32_t q, int32_t r):
        _q(q), _r(r)
    {}

    inline constexpr int32_t q() const { return _q; }
    inline constexpr int32_t r() const { return _r; }
    inline constexpr int32_t s() const { return - _q - _r; }

    constexpr uint64_t hash() const
    {
#ifndef _MSC_VER
        union HashConverter
        {
            struct
            {
                uint32_t high;
                uint32_t low;
            }
            b32;
            uint64_t b64;
        };
#endif

        return (HashConverter { { (uint32_t)r(), (uint32_t)q() } }).b64;  // NOLINT(whitespace/braces)
    }

    /*constexpr*/ int distance(const Offset& offset) const
    {
        int32_t dq = std::abs(q() - offset.q());
        int32_t dr = std::abs(r() - offset.r());
        int32_t ds = std::abs(s() - offset.s());
        return std::max(std::max(dq, dr), ds);
    }

private:
    const int32_t _q;
    const int32_t _r;
};

// Windows does not define "cos" as constexpr yet... :(
constexpr uint8_t cellSize = 9;

#if defined(_WIN32) || defined(__clang__)
    const double slopeX = cos(15 * M_PI / 180.0) * cellSize;

    inline const Offset offsetOf(int32_t x, int32_t y)
    {
        return Offset(x / slopeX, y / cellSize);
    }
#else
    constexpr double slopeX = cos(15 * M_PI / 180.0) * cellSize;

    constexpr Offset offsetOf(int32_t x, int32_t y)
    {
        return Offset(x / slopeX, y / cellSize);
    }
#endif

// Directions
struct Direction
{
    int32_t q;
    int32_t r;
};

static Direction directions[] =
{
    {+1, -1}, {+1, +0}, {+0, +1},
    {-1, +1}, {-1, +0}, {+0, -1}
};
