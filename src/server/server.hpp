/* Copyright 2016 Guillem Pascual */

#pragma once

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/lockfree/queue.hpp>

#include <inttypes.h>
#include <cstddef>


class Client;

class Server
{
public:
    explicit Server(uint16_t port, int poolSize = 2048);
    virtual ~Server();

    static Server* get() { return _instance; }

    void updateIO();
    void runScheduledOperations();

    void startAccept();
    virtual void handleAccept(Client* client, const boost::system::error_code& error);
    virtual void handleRead(Client* client, const boost::system::error_code& error, size_t size) = 0;
    virtual void handleClose(Client* client);

private:
    static Server* _instance;

    boost::asio::io_service _service;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::acceptor _acceptor;

    boost::lockfree::queue<Client*, boost::lockfree::capacity<1024>> _closeList;
    boost::object_pool<Client> _pool;
};
