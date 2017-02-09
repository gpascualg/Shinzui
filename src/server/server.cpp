/* Copyright 2016 Guillem Pascual */

#include "server.hpp"
#include "atomic_autoincrement.hpp"
#include "client.hpp"
#include "debug.hpp"
#include "map.hpp"
#include "map_aware_entity.hpp"

#include <list>

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>


Server* Server::_instance = nullptr;

Server::Server(uint16_t port) :
    _service(),
    _acceptor(_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _socket(_service)
{
    assert(!_instance);
    _instance = this;
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
    std::list<Client*> pending;

    _closeList.consume_all([this, &pending](Client* client)
        {
            // Still in the map
            if (client->inMap())
            {
                pending.push_back(client);
            }
            else
            {
                destroyClient(client);
            }
        }
    );  // NOLINT(whitespace/parens)

    for (Client* client : pending)
    {
        _closeList.push(client);
    }
}

void Server::startAccept()
{
    Client* client = newClient(&_service, AtomicAutoIncrement<0>::get());

    _acceptor.async_accept(client->socket(), [this, client](const auto error)
        {
            this->handleAccept(client, error);
        }
    );  // NOLINT(whitespace/parens)
}

void Server::handleAccept(Client* client, const boost::system::error_code& error)
{
    LOG(LOG_CLIENT_LIFECYCLE, "Client accepted");

    startAccept();
};

void Server::handleClose(Client* client)
{
    _closeList.push(client);
}

Client* Server::newClient(boost::asio::io_service* service, uint64_t id)
{
    return new Client(service, id);
}

void Server::destroyClient(Client* client)
{
    delete client;
}

MapAwareEntity* Server::newMapAwareEntity(uint64_t id, Client* client)
{
    return new MapAwareEntity(id, client);
}

void Server::destroyMapAwareEntity(MapAwareEntity* entity)
{
    delete entity;
}
