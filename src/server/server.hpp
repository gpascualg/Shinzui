#pragma once

#include "boost/asio/asio_forward.hpp"

#include <inttypes.h>
#include <cstddef>


class Client;

class Server
{
public:
    Server();
    virtual ~Server();

    void startAccept();
    virtual void handleAccept(Client* client, const boost::system::error_code& error);
    virtual void handleRead(Client* client, const boost::system::error_code* error, size_t size) = 0;

private:
    boost::asio::io_service* _service;
    boost::asio::SocketForward* _socket;
    boost::asio::AcceptorForward* _acceptor;
};
