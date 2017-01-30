/* Copyright 2016 Guillem Pascual */

#pragma once

#include "boost/asio/asio_forward.hpp"

#include <inttypes.h>
#include <array>
#include <cstddef>
#include <functional>

class Client
{
    using ReadFunction = std::function<void(Client*, const boost::system::error_code*, size_t)>;

public:
    Client(boost::asio::io_service* io_service, ReadFunction readFunction);
    Client(const Client& client) = delete;
    virtual ~Client();

    void scheduleRead(uint16_t bytesToRead, bool reset = false);

    inline char* data() { return _data; }
    inline uint8_t readPhase() { return _readTimes; }
    inline boost::asio::SocketForward* socket() { return _socket; }

private:
    boost::asio::SocketForward* _socket;
    ReadFunction _readFunction;

    char* _data = nullptr;
    uint16_t _totalRead = 0;
    uint8_t _readTimes = 0;
};
