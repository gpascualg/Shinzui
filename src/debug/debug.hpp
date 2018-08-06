/* Copyright 2016 Guillem Pascual */

#pragma once

#define LOG_NOTHING             0x0000000000000000
#define LOG_ALL                 0xFFFFFFFFFFFFFFFF
#define LOG_DEBUG               0x0000000000000001
#define LOG_CELLS               0x0000000000000002
#define LOG_CLUSTERS            0x0000000000000004
#define LOG_FATAL               0x0000000000000008
#define LOG_CLIENT_LIFECYCLE    0x0000000000000010
#define LOG_PACKET_LIFECYCLE    0x0000000000000020
#define LOG_PACKET_RECV         0x0000000000000040
#define LOG_PACKET_SEND         0x0000000000000080
#define LOG_SERVER_LOOP         0x0000000000000100
#define LOG_MOVEMENT_GENERATOR  0x0000000000000200
#define LOG_CELL_CHANGES        0x0000000000000400
#define LOG_FIRE_LOGIC          0x0000000000000800
#define LOG_FIRE_LOGIC_EXT      0x0000000000001000
#define LOG_QUADTREE            0x0000000000002000

#define LOG_LEVEL               LOG_CLIENT_LIFECYCLE | LOG_PACKET_LIFECYCLE | LOG_PACKET_RECV | LOG_PACKET_SEND | LOG_FIRE_LOGIC | LOG_FIRE_LOGIC_EXT  // NOLINT

#define STR(a)                  STR_(a)
#define STR_(a)                 #a

// Expand is a trick for MSVC __VA_ARGS__ to work :(
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

#define LOG_HELPER(lvl, fmt, ...) \
    EXPAND(printf("\x1B[01;44m[%.2X] %s:" STR(__LINE__) "(%s) \x1B[00m\x1B[0;34;49m\xee\x82\xb0\x1B[00m" fmt "\n%s", lvl, FILE_NAME, FUNCTION_NAME, __VA_ARGS__))  // NOLINT(whitespace/line_length)

#define LOG_ALWAYS(...)             EXPAND(LOG_HELPER(-1, __VA_ARGS__, ""))

#define FORCE_DEBUG

#if defined(FORCE_DEBUG) || ((!defined(NDEBUG) || defined(_DEBUG)) && BUILD_TESTS != ON)
    #define IF_LOG(lvl)         (lvl & (LOG_LEVEL))  // NOLINT
    #define LOG(lvl, ...)       ((lvl & (LOG_LEVEL)) && EXPAND(LOG_HELPER(lvl, __VA_ARGS__, "")))  // NOLINT
#else
    #define IF_LOG(lvl)         false
    #define LOG(lvl, ...)
#endif
