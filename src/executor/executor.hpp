#pragma once

#include <inttypes.h>
#include <queue>
#include <boost/lockfree/queue.hpp>

#include "executor/abstract_work.hpp"
#include "executor/schedulable.hpp"


template <uint16_t MaxQueued> class Executor;


template <uint16_t MaxQueued>
class Executor
{
public:
    Executor();
    virtual ~Executor();

    void executeJobs();
    void schedule(AbstractWork* job);
    void schedule(SchedulableTask<MaxQueued>&& task, TimePoint when);

private:
    // Can be used for async jobs
    boost::lockfree::queue<AbstractWork*, boost::lockfree::capacity<MaxQueued>> _jobs;

    // Purely sequential and time-stamped jobs
    std::priority_queue<Schedulable<MaxQueued>*, std::vector<Schedulable<MaxQueued>*>, SchedulableComparator<MaxQueued>> _scheduledTasks;
};
