/* Copyright 2016 Guillem Pascual */

#pragma once

#if SIZEOF_VOID_P == 8
    #define FMT_PTR "%lX"
#else
    #define FMT_PTR "%X"
#endif

#include <chrono>

using TimePoint = std::chrono::high_resolution_clock::time_point;

#if !defined(INCL_NOWARN) && !defined(INCL_WARN)
    #ifdef _MSC_VER
        #define INCL_NOWARN __pragma(warning(push, 0))
        #define INCL_WARN __pragma(warning(pop))
    #else
        #define INCL_NOWARN \
            _Pragma(diagnostic push) \
            _Pragma(diagnostic ignored "-Wpedantic") \
            _Pragma(diagnostic ignored "-Wunused-variable") \
            _Pragma(diagnostic ignored "-Wunused-result") \
            _Pragma(diagnostic ignored "-Wstrict-aliasing") \
            _Pragma(diagnostic ignored "-Wunreachable-code") \
            _Pragma(diagnostic ignored "-Wuninitialized")
        #define INCL_WARN _Pragma(diagnostic pop)
    #endif
#endif
