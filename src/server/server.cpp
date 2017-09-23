/* Copyright 2016 Guillem Pascual */

#include "server/server.hpp"
#include "defs/atomic_autoincrement.hpp"
#include "server/client.hpp"
#include "debug/debug.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"

#include <list>

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>


Server* Server::_instance = nullptr;

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

void Server::startAccept()
{
    Client* client = newClient(&_service, AtomicAutoIncrement<0>::get());

    _acceptor.async_accept(client->socket(), [this, client](const auto error)
        {
            _operations.push(new Operation{ OperationType::ACCEPT, client, error });

            startAccept();
        }
    );  // NOLINT(whitespace/parens)
}

void Server::handleAccept(Client* client, const boost::system::error_code& error)
{
    LOG(LOG_CLIENT_LIFECYCLE, "Client accepted");
};

void Server::handleClose(Client* client)
{
    _operations.push(new Operation{ OperationType::CLOSE, client, boost::system::error_code() });
}

Client* Server::newClient(boost::asio::io_service* service, uint64_t id)
{
    return new Client(service, id);
}

void Server::destroyClient(Client* client)
{
    delete client;
}
