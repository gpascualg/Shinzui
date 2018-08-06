/* Copyright 2016 Guillem Pascual */

#include "server/server.hpp"
#include "defs/atomic_autoincrement.hpp"
#include "server/client.hpp"
#include "debug/debug.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"

#include <list>

#include "defs/common.hpp"

INCL_NOWARN
#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>
INCL_WARN


Server* Server::_instance = nullptr;
const TimeBase WORLD_HEART_BEAT = TimeBase(50);


Server::Server(uint16_t port) :
    _service(),
    _acceptor(_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _socket(_service)
{
    checkInstance();
    _map = new Map();
}

Server::~Server()
{
    delete _map;
}

void Server::updateIO()
{
    _service.run();
}

void Server::runScheduledOperations()
{
    std::list<Operation*> pending;

    _operations.consume_all([this, &pending](Operation* op)
        {
            switch (op->type)
            {
                case OperationType::ACCEPT:
                    this->handleAccept(op->client, op->error);
                    delete op;
                    break;

                case OperationType::CLOSE:
                    // Still in the map
                    if (op->client->inMap())
                    {
                        pending.push_back(op);
                    }
                    else
                    {
                        destroyClient(op->client);
                        delete op;
                    }
                    break;
            }
        }
    );  // NOLINT(whitespace/parens)

    // Reschedule pending operations
    for (auto op : pending)
    {
        _operations.push(op);
    }
}

void Server::update()
{ 
    // Update timebase
    _now = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<TimeBase>(_now - _lastUpdate);
    _lastUpdate = _now;
    
    // First update map
    map()->update(diff.count());
    
    // Run scheduled tasks, which might include packets
    Executor<8192>::executeJobs();

    // Last, server wide operations (ie. accept/close clients)
    runScheduledOperations();

    // Now cleanup map
    map()->cleanup(diff.count());

    // Wait for a constant update time
    if (diff <= WORLD_HEART_BEAT + _prevSleepTime)
    {
        _prevSleepTime = WORLD_HEART_BEAT + _prevSleepTime - diff;
        std::this_thread::sleep_for(_prevSleepTime);
    }
    else
    {
        _prevSleepTime = _prevSleepTime.zero();
    }

    LOG(LOG_SERVER_LOOP, "DIFF: %" PRId64 " - SLEEP: %" PRId64, diff.count(), _prevSleepTime.count());
}

void Server::startAccept()
{
    Client* client = newClient(&_service, AtomicAutoIncrement<0>::get());

    _acceptor.async_accept(client->socket(), [this, client](const auto error)
        {
            _operations.push(new Operation{ OperationType::ACCEPT, client, error });  // NOLINT (whitespace/braces)

            this->startAccept();
        }
    );  // NOLINT(whitespace/parens)
}

void Server::handleAccept(Client* client, const boost::system::error_code& error)
{
    LOG(LOG_CLIENT_LIFECYCLE, "Client accepted");
}

void Server::handleClose(Client* client)
{
    _operations.push(new Operation{ OperationType::CLOSE, client, boost::system::error_code() });  // NOLINT (whitespace/braces)
}

void Server::onWorkError(AbstractWork* work)
{
    work->executor()->close();
}

Client* Server::newClient(boost::asio::io_service* service, uint64_t id)
{
    return new Client(service, id);
}

void Server::destroyClient(Client* client)
{
    delete client;
}
