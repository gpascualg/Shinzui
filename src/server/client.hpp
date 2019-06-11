/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>
#include <array>
#include <cstddef>
#include <functional>
#include <type_traits>

#include "defs/common.hpp"

INCL_NOWARN
#include <boost/asio.hpp>
#include "defs/intrusive.hpp"
#include <boost/intrusive_ptr.hpp>
INCL_WARN


class Packet;
class MapAwareEntity;

class Client
{
public:
    enum class Status
    {
        INITIALIZED,
        IN_WORLD,
        CLOSED
    };

public:
    Client(boost::asio::io_service* io_service, uint64_t id);
    Client(const Client& client) = delete;
    virtual ~Client();

    uint64_t id();
    bool inMap();

    void scheduleRead(uint16_t bytesToRead, bool reset = false);
    void send(boost::intrusive_ptr<Packet> packet);

    inline Packet* packet() { return _packet; }
    inline uint8_t readPhase() { return _readTimes; }
    inline uint16_t lag() { return _lag; }
    inline boost::asio::ip::tcp::socket& socket() { return _socket; }

    inline Status status() { return _status; }
    inline void status(Status status) { _status = status; }

    inline MapAwareEntity* entity() { return _entity; }

    virtual void close();

private:
    Status _status;
    MapAwareEntity* _entity;
    boost::asio::ip::tcp::socket _socket;
    boost::asio::deadline_timer _timer;

    Packet* _packet;
    uint16_t _readTimes = 0;
    uint8_t _lag = 0;
};
