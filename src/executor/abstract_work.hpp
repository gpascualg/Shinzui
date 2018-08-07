#pragma once

#include <inttypes.h>
#include <functional>
#include <future>

#include "io/packet.hpp"


class Client;
class Server;
template <uint16_t MaxQueued> class Executor;
struct AbstractWork;
template <class T> struct FutureWork;

using ExecutorWork = std::function<AbstractWork*(AbstractWork*)>;


struct AbstractWork
{
public:
	virtual ~AbstractWork();

	template <uint16_t MaxQueued>
	bool call(Executor<MaxQueued>* who, AbstractWork** next)
	{
		try
    	{
			*next = _handler(this);
			return true;
		}
		catch (const ReadOutOfBounds& e)
		{
			triggerError();
		}

		return false;
	}

	void triggerError();
	virtual bool ready();

	inline Client* executor()
	{
		return _executor;
	}

protected:
	AbstractWork(ExecutorWork handler, Client* executor);

protected:
	ExecutorWork _handler;
	Client* _executor;
};

struct ClientWork : public AbstractWork
{
public:
	ClientWork(ExecutorWork handler, Client* executor, Packet* packet);
	virtual ~ClientWork();

	inline Packet* packet()
	{
		return _packet;
	}

protected:
	Packet* _packet;
};

template <typename T>
struct FutureWork : public AbstractWork
{
public:
	FutureWork(ExecutorWork handler, Client* executor, std::future<T>&& future) :
		AbstractWork(handler, executor)
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
