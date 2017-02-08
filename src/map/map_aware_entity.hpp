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
    explicit MapAwareEntity(uint64_t id, Client* client = nullptr);
    virtual ~MapAwareEntity();

    inline Cell* cell() { return _cell; }
    inline Client* client() { return _client; }
    inline Position& position() { return _position; }
    inline uint64_t id() { return _id; }

    virtual void update(uint64_t elapsed) {}
    virtual void onAdded(Cell* cell);
    virtual void onRemoved(Cell* cell);

protected:
    Client* _client;
    Cell* _cell;
    uint64_t _id;

    Position _position;
};
