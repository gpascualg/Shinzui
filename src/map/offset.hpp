/* Copyright 2016 Guillem Pascual */

#pragma once

#define _USE_MATH_DEFINES

#include <inttypes.h>
#include <math.h>
#include <algorithm>
#include <unordered_map>
#include <utility>


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

#undef max
#undef min

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

    inline const Offset offsetOf(float x, float y)
    {
        return Offset((int32_t)(x / slopeX), (int32_t)(y / cellSize));
    }

    inline const Offset offsetOf(int32_t x, int32_t y)
    {
        return Offset((int32_t)(x / slopeX), (int32_t)(y / cellSize));
    }
#else
    constexpr double slopeX = cos(15 * M_PI / 180.0) * cellSize;

    inline const Offset offsetOf(float x, float y)
    {
        return Offset((int32_t)(x / slopeX), (int32_t)(y / cellSize));
    }

    constexpr Offset offsetOf(int32_t x, int32_t y)
    {
        return Offset((int32_t)(x / slopeX), (int32_t)(y / cellSize));
    }
#endif

// Directions
struct Direction
{
    int32_t q;
    int32_t r;
};

// Index to direction
static Direction directions[] =
{
    {+1, -1}, {+1, +0}, {+0, +1},
    {-1, +1}, {-1, +0}, {+0, -1}
};

struct pair_hash
{
    uint64_t operator () (const std::pair<int32_t, int32_t> &p) const
    {
        return Offset(p.first, p.second).hash();
    }
};

// Direction to index
static constexpr int32_t MAX_DIR_IDX = 6;
static inline int32_t directionIdxs(int32_t q, int32_t r)
{
    if (q == +1 && r == -1) return 0;
    if (q == +1 && r == +0) return 1;
    if (q == +0 && r == +1) return 2;
    if (q == -1 && r == +1) return 3;
    if (q == -1 && r == +0) return 4;
    if (q == +0 && r == -1) return 5;

    return -1;
}
