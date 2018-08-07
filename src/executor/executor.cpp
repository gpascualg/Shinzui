#include "executor/executor.hpp"
#include "server/server.hpp"

#include <vector>


template <uint16_t MaxQueued>
Executor<MaxQueued>::Executor()
{}

template <uint16_t MaxQueued>
Executor<MaxQueued>::~Executor()
{
    // Free async jobs memory
    AbstractWork* job = nullptr;
    while (_jobs.pop(job)) 
    {
        delete job;
    }

    // Free sync jobs memory
    while (!_scheduledTasks.empty())
    {
        auto st = _scheduledTasks.top();
        _scheduledTasks.pop();
        delete st;
    }
}

template <uint16_t MaxQueued>
void Executor<MaxQueued>::executeJobs()
{
    // First do all async tasks
    std::vector<AbstractWork*> reschedule;
    bool success = true;

    AbstractWork* job = nullptr;
    while (success && _jobs.pop(job)) 
    {
        // FutureWork(s) might now be ready yet
        if (!job->ready())
        {
            reschedule.emplace_back(job);
            continue;
        }

        // Execute next and get next task, if any
        // If success is false, next iteration won't go through
        AbstractWork* next = nullptr;
        success = job->call(this, &next);
        if (success && next)
        {
            reschedule.emplace_back(next);
        }

        // Free memory
        delete job;
    }

    // No need to continue if the client has been errored out
    if (!success)
    {
        return;
    }

    for (AbstractWork* job : reschedule)
    {
        _jobs.push(job);
    }

    // Now do timestamped jobs
    // Run all scheduled tasks if any
    while (!_scheduledTasks.empty() && _scheduledTasks.top()->when < Server::get()->now())
    {
        auto st = _scheduledTasks.top();

        // Execute task and pop
        (*st->task)(this);
        _scheduledTasks.pop();

        // Free memory
        delete st;
    }
}

template <uint16_t MaxQueued>
void Executor<MaxQueued>::schedule(AbstractWork* job)
{
    _jobs.push(job);
}

template <uint16_t MaxQueued>
void Executor<MaxQueued>::schedule(SchedulableTask<MaxQueued>&& task, TimePoint when)
{
    _scheduledTasks.emplace(new Schedulable<MaxQueued>{ task, when });  // NOLINT(whitespace/braces)
}


// TODO: Is there another way around?
template class Executor<16>;
template class Executor<32>;
template class Executor<64>;
template class Executor<128>;
template class Executor<256>;
template class Executor<512>;
template class Executor<1024>;
template class Executor<2048>;
template class Executor<4096>;
template class Executor<8192>;
