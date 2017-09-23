/* Copyright 2016 Guillem Pascual */

#pragma once

#define _USE_MATH_DEFINES

#include <inttypes.h>
#include <math.h>
#include <algorithm>
#include <unordered_map>
#include <utility>
#include <glm/glm.hpp>


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

// Windows does not define "cos" as constexpr yet... :(
constexpr float cellSize_x = 80.0f;
constexpr float cellSize_y = 80.0f;

struct Offset
{
public:
    constexpr Offset(int32_t q, int32_t r) :
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

    constexpr glm::vec2 center() const 
    { 
        return { cellSize_x * 3.0 / 2.0 * _q, cellSize_y * std::sqrt(3.0) * (_r + _q / 2.0) };
    }

private:
    const int32_t _q;
    const int32_t _r;
};

inline Offset offsetOf(float x, float y)
{
    float q = x * 2.0 / 3.0 / cellSize_x;
    float r = (-x / 3.0 + std::sqrt(3.0) / 3.0 * y) / cellSize_y;
    float s = -q - r;

    int q_int = static_cast<int>(round(q));
    int r_int = static_cast<int>(round(r));
    int s_int = static_cast<int>(round(s));

    float q_diff = abs(q_int - q);
    float r_diff = abs(r_int - r);
    float s_diff = abs(s_int - s);

    if (q_diff > r_diff && q_diff > s_diff)
    {
        q_int = -r_int - s_int;
    }
    else if (r_diff > s_diff)
    {
        r_int = -q_int - s_int;
    }
    else
    {
        // s_int = -q_int - r_int;
    }

    return Offset{ q_int, r_int };
}

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
