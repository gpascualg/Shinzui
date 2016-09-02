#pragma once

#if SIZEOF_VOID_P == 8
    #define FMT_PTR "%llX"
#else
    #define FMT_PTR "%lX"
#endif
