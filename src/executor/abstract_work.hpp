#pragma once

#include <inttypes.h>
#include <functional>
#include <future>


class Server;
template <uint16_t MaxQueued> class Executor;
template <uint16_t MaxQueued> struct AbstractWork;
template <typename T, uint16_t MaxQueued> struct FutureWork;

template <uint16_t MaxQueued> using ExecutorWork = std::function<bool(Server*, Executor<MaxQueued>*, AbstractWork<MaxQueued>*)>;


template <uint16_t MaxQueued>
struct AbstractWork
{
public:
	virtual ~AbstractWork()
	{}

	bool call(Server* server, Executor<MaxQueued>* executor)
	{
		return _handler(server, executor, this);
	}

    virtual bool ready()
    {
        return true;
    }

protected:
	AbstractWork(ExecutorWork<MaxQueued> handler) :
		_handler(handler)
	{}

protected:
	ExecutorWork<MaxQueued> _handler;
};

template <uint16_t MaxQueued>
struct ClientWork : public AbstractWork<MaxQueued>
{
public:
	ClientWork(ExecutorWork<MaxQueued> handler) :
		AbstractWork<MaxQueued>(handler)
	{}

	virtual ~ClientWork()
	{}
};

template <typename T, uint16_t MaxQueued>
struct FutureWork : public AbstractWork<MaxQueued>
{
public:
	FutureWork(ExecutorWork<MaxQueued> handler, std::future<T>&& future) :
		AbstractWork<MaxQueued>(handler)
	{
        _future = std::move(future);
    }

	virtual ~FutureWork()
	{}

	inline T get() { return _future.get(); }
    bool ready() override
    {
        return _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

protected:
	std::future<T> _future;
};
