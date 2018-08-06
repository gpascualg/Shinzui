#include <inttypes.h>
#include <functional>
#include <future>

#include "executor/abstract_work.hpp"
#include "server/server.hpp"


AbstractWork::AbstractWork(ExecutorWork handler, Client* executor) :
    _handler(handler),
    _executor(executor)
{}

AbstractWork::~AbstractWork()
{}

void AbstractWork::triggerError()
{
    Server::get()->onWorkError(this);
}

bool AbstractWork::ready()
{
    return true;
}


ClientWork::ClientWork(ExecutorWork handler, Client* executor, Packet* packet) :
    AbstractWork(handler, executor),
    _packet(packet)
{}

ClientWork::~ClientWork()
{
    _packet->destroy();
}
