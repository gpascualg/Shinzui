/* Copyright 2016 Guillem Pascual */

#pragma once

#include "defs/common.hpp"
#include "executor/executor.hpp"

INCL_NOWARN
#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/lockfree/queue.hpp>
INCL_WARN

#include <inttypes.h>
#include <cstddef>
#include <unordered_map>


class Client;
class Packet;
class Map;
class MapAwareEntity;

// TODO(gpascualg): Is 8192 too much?
class Server : public Executor<8192>
{
private:
    enum class OperationType
    {
        ACCEPT,
        CLOSE
    };

    struct Operation
    {
        OperationType type;
        Client* client;
        const boost::system::error_code& error;
    };

public:
    explicit Server(uint16_t port);
    virtual ~Server();

    void checkInstance()
    {
#ifndef SHINZUI_TESTS
        // HACK(gpascualg): ifndef not working, remove comment
        // assert(!_instance);
#endif
        _instance = this;
    }

    static Server* get() { return _instance; }

    void updateIO();
    void runScheduledOperations();

    inline Map* map() { return _map; }
    inline TimePoint now() { return _now; }
    void update();

    void startAccept();
    virtual void handleAccept(Client* client, const boost::system::error_code& error);
    virtual void handleRead(Client* client, const boost::system::error_code& error, size_t size) = 0;
    virtual void handleClose(Client* client);

    virtual Client* newClient(boost::asio::io_service* service, uint64_t id);
    virtual void destroyClient(Client* client);

    virtual MapAwareEntity* newMapAwareEntity(uint64_t id, Client* client) = 0;
    virtual void destroyMapAwareEntity(MapAwareEntity* entity) = 0;

private:
    boost::asio::io_service _service;
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _socket;

    Map* _map;

    // Timing related attributes
    TimePoint _lastUpdate;
    TimePoint _now;
    TimeBase _prevSleepTime;

    boost::lockfree::queue<Operation*, boost::lockfree::capacity<1024>> _operations;

    static Server* _instance;
};
