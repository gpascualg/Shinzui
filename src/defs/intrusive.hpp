/* Copyright 2016 Guillem Pascual */

#pragma once

#include "io/packet.hpp"


inline void intrusive_ptr_add_ref(Packet* x)
{
    ++x->_refs;
}

inline void intrusive_ptr_release(Packet* x)
{
    if (--x->_refs == 0)
    {
        x->destroy();
    }
}
