/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>


class Cell;
class Client;

struct Position
{
    float x;
    float y;
};

class MapAwareEntity
{
public:
    explicit MapAwareEntity(Client* client = nullptr) :
        _client(client),
        _cell(nullptr),
        _position{0, 0}
    {}

    virtual ~MapAwareEntity()
    {}

    inline Cell* cell() { return _cell; }
    inline Client* client() { return _client; }
    inline Position& position() { return _position; }
    virtual uint32_t id() = 0;

    virtual void update(uint64_t elapsed) {}
    virtual void onAdded(Cell* cell);
    virtual void onRemoved(Cell* cell);

protected:
    Client* _client;
    Cell* _cell;

    Position _position;
};
