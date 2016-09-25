/* Copyright 2016 Guillem Pascual */

#include "server.hpp"
#include "client.hpp"

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>


using tcp = boost::asio::ip::tcp;


Server::Server(uint16_t port, boost::object_pool<Client>* pool)
{
    _pool = pool ? pool : new boost::object_pool<Client>(2048);

    _service = new boost::asio::io_service;
    _acceptor = new tcp::acceptor(*_service, tcp::endpoint(tcp::v4(), port));
    _socket = new tcp::socket(*_service);
}

Server::~Server()
{
    delete _service;
    delete _acceptor;
    delete _socket;
}

void Server::updateIO()
{
    _service->run();
}

void Server::startAccept()
{
    Client* client = _pool->construct(_service, [this](auto client, const auto error, auto size)
    {
        this->handleRead(client, error, size);
    });  // NOLINT(whitespace/braces)

    _acceptor->async_accept(*client->socket(), [this, client](const auto error)
    {
        this->handleAccept(client, error);
    });  // NOLINT(whitespace/braces)
}

void Server::handleAccept(Client* client, const boost::system::error_code& error)
{
    startAccept();
};
