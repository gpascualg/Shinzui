/* Copyright 2016 Guillem Pascual */

#include "server.hpp"
#include "debug.hpp"

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>


template <class TClient>
Server<TClient>::Server(uint16_t port, int poolSize) :
    _pool(poolSize),
    _service(),
    _acceptor(_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _socket(_service)
{
}

template <class TClient>
Server<TClient>::~Server()
{}

template <class TClient>
void Server<TClient>::updateIO()
{
    _service.run();
}

template <class TClient>
void Server<TClient>::startAccept()
{
    TClient* client = _pool.construct(&_service, [this](auto client, const auto error, auto size)
        {
            this->handleRead(static_cast<TClient*>(client), error, size);
        },  // NOLINT(whitespace/braces)
        [this](auto client)
        {
            this->_pool.destroy(static_cast<TClient*>(client));
        });  // NOLINT(whitespace/braces)

    _acceptor.async_accept(client->socket(), [this, client](const auto error)
        {
            this->handleAccept(static_cast<TClient*>(client), error);
        });  // NOLINT(whitespace/braces)
}

template <class TClient>
void Server<TClient>::handleAccept(TClient* client, const boost::system::error_code& error)
{
    LOG(LOG_DEBUG, "Client accepted");

    startAccept();
};
