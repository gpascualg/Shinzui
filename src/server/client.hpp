/* Copyright 2016 Guillem Pascual */

#pragma once

#include <boost/asio.hpp>

#include <inttypes.h>
#include <array>
#include <cstddef>
#include <functional>
#include <type_traits>


class Client
{
    using ReadFunction = std::function<void(Client*, const boost::system::error_code&, size_t)>;
    using CloseFunction = std::function<void(Client*)>;

public:
    Client(boost::asio::io_service* io_service, ReadFunction readFunction, CloseFunction closeFunction);
    Client(const Client& client) = delete;
    virtual ~Client();

    inline virtual uint64_t id() = 0;

    void scheduleRead(uint16_t bytesToRead, bool reset = false);

    inline char* data() { return _data; }
    inline uint8_t readPhase() { return _readTimes; }
    inline uint32_t totalRead() { return _totalRead; }
    inline boost::asio::ip::tcp::socket& socket() { return _socket; }
    
    virtual void close();

private:
    boost::asio::ip::tcp::socket _socket;
    boost::asio::deadline_timer _timer;

    ReadFunction _readFunction;
    CloseFunction _closeFunction;

    char* _data = nullptr;
    uint32_t _totalRead = 0;
    uint8_t _readTimes = 0;
};
