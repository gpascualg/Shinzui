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

#define LOG_LEVEL               LOG_ALL & ~LOG_SERVER_LOOP

#define STR(a) STR_(a)
#define STR_(a) #a

// Expand is a trick for MSVC __VA_ARGS__ to work :(
#define EXPAND(x)               x

#if defined(_MSC_VER)
    #define __func__ __FUNCTION__
    #define __FILENAME__ __FILE__ + BASE_DIR_LEN
#else
    #undef __func__
    #define __func__ STR(__FUNCTION__)
    #define __FILENAME__ __FILE__
#endif

#define LOG_HELPER(lvl, fmt, ...) \
    EXPAND(printf("[%.2X] %s:" STR(__LINE__) "(" __func__ ") > " fmt "\n%s", lvl, __FILENAME__, __VA_ARGS__))

#define LOG_ALWAYS(...)             EXPAND(LOG_HELPER(-1, __VA_ARGS__, ""))

#define FORCE_DEBUG

#if defined(FORCE_DEBUG) || ((!defined(NDEBUG) || defined(_DEBUG)) && BUILD_TESTS != ON)
    #define IF_LOG(lvl)         (lvl & LOG_LEVEL)  // NOLINT
    #define LOG(lvl, ...)       ((lvl & LOG_LEVEL) && EXPAND(LOG_HELPER(lvl, __VA_ARGS__, "")))  // NOLINT
#else
    #define IF_LOG(lvl)         false
    #define LOG(lvl, ...)
#endif
