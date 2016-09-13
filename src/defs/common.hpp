/* Copyright 2016 Guillem Pascual */

#pragma once

#if SIZEOF_VOID_P == 8
    #define FMT_PTR "%lX"
#else
    #define FMT_PTR "%X"
#endif
