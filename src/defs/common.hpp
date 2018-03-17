/* Copyright 2016 Guillem Pascual */

#pragma once

#if SIZEOF_VOID_P == 8
    #define FMT_PTR "%lX"
#else
    #define FMT_PTR "%X"
#endif

#if !defined(INCL_NOWARN) && !defined(INCL_WARN)
    #ifdef _MSC_VER
        #define INCL_NOWARN __pragma(warning(push, 0))
        #define INCL_WARN __pragma(warning(pop))
    #else
        #define INCL_NOWARN \
            _Pragma("GCC diagnostic push") \
            _Pragma("GCC diagnostic ignored \"-Wpedantic\"") \
            _Pragma("GCC diagnostic ignored \"-Wunused-variable\"") \
            _Pragma("GCC diagnostic ignored \"-Wunused-result\"") \
            _Pragma("GCC diagnostic ignored \"-Wstrict-aliasing\"") \
            _Pragma("GCC diagnostic ignored \"-Wunreachable-code\"") \
            _Pragma("GCC diagnostic ignored \"-Wuninitialized\"")
        #define INCL_WARN _Pragma("GCC diagnostic pop")
    #endif
#endif

#include <chrono>  // NOLINT(build/c++11)
using TimePoint = std::chrono::high_resolution_clock::time_point;
