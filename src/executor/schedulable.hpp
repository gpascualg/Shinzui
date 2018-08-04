#pragma once

#include <inttypes.h>

#include "defs/common.hpp"


template <uint16_t MaxQueued> class Executor;
template <uint16_t MaxQueued> using SchedulableTask = void(*)(Executor<MaxQueued>*);

template <uint16_t MaxQueued>
struct Schedulable
{
    bool operator<(const Schedulable<MaxQueued>& other) const
    {
        return when < other.when;
    }

    SchedulableTask<MaxQueued> task;
    TimePoint when;
};

template <uint16_t MaxQueued>
struct SchedulableComparator
{
    bool operator()(const Schedulable<MaxQueued>* a, const Schedulable<MaxQueued>* b)
    {
        return a->when < b->when;
    }
};
