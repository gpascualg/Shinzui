/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>


class Cell;
class Client;

class MapAwareEntity
{
public:
    MapAwareEntity(Client* client = nullptr) :
        _client(client)
    {}

    virtual ~MapAwareEntity()
    {}

    inline Client* client() { return _client; }
    virtual uint32_t id() = 0;

    virtual void update(uint64_t elapsed) = 0;
    virtual void onAdded(Cell* cell) = 0;
    virtual void onRemoved(Cell* cell) = 0;

protected:
    Client* _client;
};
