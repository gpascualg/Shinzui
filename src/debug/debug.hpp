/* Copyright 2016 Guillem Pascual */

#pragma once

// For abort and printf
#include <stdlib.h>
#include <stdio.h>

// We shall use up to 4 bytes (uint32_t)
#define LOG_NOTHING             0x00000000
#define LOG_ALL                 0xFFFFFFFF

// Log levels, filters importance
#define LOG_LEVEL_DEBUG         0x00000001
#define LOG_LEVEL_INFO          0x00000002
#define LOG_LEVEL_WARNING       0x00000004
#define LOG_LEVEL_ERROR         0x00000008

// Log handlers
// #define LOG_FREE_SLOT        0x00000001
#define LOG_CELLS               0x00000002
#define LOG_CLUSTERS            0x00000004
#define LOG_FATAL               0x00000008
#define LOG_CLIENT_LIFECYCLE    0x00000010
#define LOG_PACKET_LIFECYCLE    0x00000020
#define LOG_PACKET_RECV         0x00000040
#define LOG_PACKET_SEND         0x00000080
#define LOG_SERVER_LOOP         0x00000100
#define LOG_MOVEMENT_GENERATOR  0x00000200
#define LOG_CELL_CHANGES        0x00000400
#define LOG_FIRE_LOGIC          0x00000800
#define LOG_FIRE_LOGIC_EXT      0x00001000
#define LOG_QUADTREE            0x00002000
#define LOG_SPAWNS              0x00004000
#define LEADING_ZEROS           4

// Compile time optimizable log
#define LOG_HANDLERS            LOG_ALL & ~LOG_SERVER_LOOP & ~LOG_QUADTREE & ~LOG_CELL_CHANGES
#define LOG_LEVEL               LOG_LEVEL_DEBUG | LOG_LEVEL_INFO | LOG_LEVEL_WARNING | LOG_LEVEL_ERROR

// No reactive debug and/or force
// #define FORCE_ASCII_DEBUG
// #define _DEBUG

/* =====================================================
   Inner functionality
   Do not modify unless absolutely mandatory
   =====================================================
*/


#define STR(a)                  STR_(a)
#define STR_(a)                 #a

// Expand is a trick for MSVC __VA_ARGS__ to work :(
// TODO(gpascualg): Revisit MSVC expand macro
#define EXPAND(x)               x

#ifndef FUNCTION_NAME
    #if defined(_MSC_VER)
        #define FUNCTION_NAME __FUNCTION__
        #define FILE_NAME __FILE__ + BASE_DIR_LEN
    #else
        #define FILE_NAME __FILE__ + BASE_DIR_LEN

        static inline void __dummy_func__()
        {
            #ifdef __PRETTY_FUNCTION__
                #define FUNCTION_NAME __PRETTY_FUNCTION__
            #elif __FUNCTION__
                #define FUNCTION_NAME __FUNCTION__
            #else
                #define FUNCTION_NAME __func__
            #endif
        }
    #endif
#endif

template <typename T>
bool nop(T value)
{
    (void)value;
    return true;
}

template <typename First, typename... Rest>
bool nop(First firstValue, Rest... rest)
{
    nop(firstValue);
    return nop(rest...);
}

#define LOG_FMT(pre, fmt, fg, bg)                pre "\x1B[" fg "m[%." STR(LEADING_ZEROS) "X] %s:" STR(__LINE__) "(%s) \x1B[00m\x1B[" bg "m\xee\x82\xb0\x1B[00m" fmt "\n%s"

#if defined(FORCE_ASCII_DEBUG)
    #define LOG_PRE                             ""
    #define LOG_FNC                             printf
    #define ASSERT_FNC                          printf
#else
    #include "debug/reactive.hpp"

    #define LOG_PRE                             "  "
    #define LOG_FNC                             Reactive::get()->print
    #define ASSERT_FNC                          Reactive::get()->print_now
#endif

#define EXPAND_HELPER(fnc, pre, lvl, fg, bg, fmt, ...) \
    EXPAND(fnc(LOG_FMT(pre, fmt, fg, bg), lvl, FILE_NAME, FUNCTION_NAME, __VA_ARGS__))  // NOLINT(whitespace/line_length)

#define LOG_ALWAYS(...)                         EXPAND(EXPAND_HELPER(LOG_FNC, LOG_PRE, -1, "01;44", __VA_ARGS__, ""))

#if !defined(NDEBUG) || defined(_DEBUG)
    #define _STATIC_IF_LOG(lvl, handler)        (((lvl) & (LOG_LEVEL)) && ((handler) & (LOG_HANDLERS)))  // NOLINT

    #if defined(FORCE_ASCII_DEBUG)
        #define _PP_IF_LOG(lvl, handler)        _STATIC_IF_LOG(lvl, handler)
    #else                    
        #define _PP_IF_LOG(lvl, handler)        _STATIC_IF_LOG(lvl, handler) && (Reactive::get()->LogLevel & lvl) && (Reactive::get()->LogHandlers & handler)
    #endif

    #define IF_LOG(lvl, handler)                if (_PP_IF_LOG(lvl, handler))

    #define _LOG(lvl, handler, fg, bg, ...)      \
        (( _PP_IF_LOG(lvl, handler) && EXPAND(EXPAND_HELPER(LOG_FNC, LOG_PRE, lvl, fg, bg, __VA_ARGS__, ""))) || \
         (!_PP_IF_LOG(lvl, handler) && EXPAND(EXPAND_HELPER(nop,     LOG_PRE, lvl, fg, bg, __VA_ARGS__, ""))))

    #define LOG(handler, ...)                   _LOG(LOG_LEVEL_INFO,    handler, "01;44", "0;34;49", __VA_ARGS__)
    #define LOG_DEBUG(handler, ...)             _LOG(LOG_LEVEL_DEBUG,   handler, "30;47", "0;37;49", __VA_ARGS__)
    #define LOG_INFO(handler, ...)              _LOG(LOG_LEVEL_INFO,    handler, "01;44", "0;34;49", __VA_ARGS__) 
    #define LOG_WARNING(handler, ...)           _LOG(LOG_LEVEL_WARNING, handler, "01;43", "0;33;49", __VA_ARGS__)
    #define LOG_ERROR(handler, ...)             _LOG(LOG_LEVEL_ERROR,   handler, "01;41", "0;31;49", __VA_ARGS__)
    
    #define LOG_ASSERT(expr, ...)               ((expr) ? (void)(0) : (EXPAND_HELPER(ASSERT_FNC, LOG_PRE, -1, "01;41", "0;31;49", __VA_ARGS__, ""), abort()))
#else
    #define _STATIC_IF_LOG(lvl, handler)        false
    #define _PP_IF_LOG(lvl, handler)            false
    #define IF_LOG(lvl, handler)                if (false)

    #define LOG(handler, ...)                   ((void)(0))
    #define LOG_DEBUG(handler, ...)             ((void)(0))
    #define LOG_INFO(handler, ...)              ((void)(0))
    #define LOG_WARNING(handler, ...)           ((void)(0))
    #define LOG_ERROR(handler, ...)             ((void)(0))

    #define LOG_ASSERT(expr, ...)               ((void)(0))
#endif
