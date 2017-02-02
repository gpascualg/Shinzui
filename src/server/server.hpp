/* Copyright 2016 Guillem Pascual */

#pragma once

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>

#include <inttypes.h>
#include <cstddef>


template <class TClient>
class Server
{
public:
    explicit Server(uint16_t port, int poolSize = 2048);
    virtual ~Server();

    void updateIO();

    void startAccept();
    virtual void handleAccept(TClient* client, const boost::system::error_code& error);
    virtual void handleRead(TClient* client, const boost::system::error_code& error, size_t size) = 0;

private:
    boost::asio::io_service _service;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::acceptor _acceptor;

    boost::object_pool<TClient> _pool;
};

// Implementation details
#include "server_i.hpp"
