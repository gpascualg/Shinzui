#include "executor/executor.hpp"
#include "server/server.hpp"

#include <vector>


template <uint16_t MaxQueued>
void Executor<MaxQueued>::executeJobs()
{
    // First do all async tasks
    std::vector<AbstractWork*> reschedule;
    
    _jobs.consume_all([this, &reschedule](auto job)
        {
            if (!job->ready())
            {
                reschedule.emplace_back(job);
                return;
            }

            job->call(this);
            delete job;
        }
    );

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
