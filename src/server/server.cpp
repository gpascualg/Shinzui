/* Copyright 2016 Guillem Pascual */

#include "server.hpp"
#include "debug.hpp"
#include "client.hpp"
#include "atomic_autoincrement.hpp"

#include <list>

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>


Server* Server::_instance = nullptr;

Server::Server(uint16_t port, int poolSize) :
    _pool(poolSize),
    _service(),
    _acceptor(_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _socket(_service)
{
    assert(!_instance);
    _instance = this;
}

Server::~Server()
{}

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
                _pool.destroy(client);
            }
        }
    );

    for (Client* client : pending)
    {
        _closeList.push(client);
    }
}

void Server::startAccept()
{
    Client* client = _pool.construct(&_service, AtomicAutoIncrement<0>::get());

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
