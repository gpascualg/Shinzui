/* Copyright 2016 Guillem Pascual */

#pragma once

#include "boost/asio/asio_forward.hpp"
#include "boost/pool/pool_forward.hpp"

#include <inttypes.h>
#include <cstddef>


class Client;

class Server
{
public:
    explicit Server(uint16_t port, boost::object_pool<Client>* pool = nullptr);
    virtual ~Server();

    void updateIO();

    void startAccept();
    virtual void handleAccept(Client* client, const boost::system::error_code& error);
    virtual void handleRead(Client* client, const boost::system::error_code* error, size_t size) = 0;

private:
    boost::asio::io_service* _service;
    boost::asio::SocketForward* _socket;
    boost::asio::AcceptorForward* _acceptor;

    boost::object_pool<Client>* _pool;
};
