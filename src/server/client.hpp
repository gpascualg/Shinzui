/* Copyright 2016 Guillem Pascual */

#pragma once

#include <boost/asio.hpp>

#include <inttypes.h>
#include <array>
#include <cstddef>
#include <functional>
#include <type_traits>


class Packet;

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

    inline Packet* packet() { return _packet; }
    inline uint8_t readPhase() { return _readTimes; }
    inline boost::asio::ip::tcp::socket& socket() { return _socket; }

    virtual void close();

private:
    boost::asio::ip::tcp::socket _socket;
    boost::asio::deadline_timer _timer;

    ReadFunction _readFunction;
    CloseFunction _closeFunction;

    Packet* _packet;
    uint8_t _readTimes = 0;
};
