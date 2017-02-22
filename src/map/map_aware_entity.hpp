/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>
#include <list>
#include <glm/glm.hpp>


class Cell;
class Client;
class MotionMaster;


class MapAwareEntity
{
    friend class Map;

public:
    explicit MapAwareEntity(uint64_t id, Client* client = nullptr);
    virtual ~MapAwareEntity();

    inline Cell* cell() { return _cell; }
    inline Client* client() { return _client; }
    inline MotionMaster* motionMaster() { return _motionMaster; }
    inline uint64_t id() { return _id; }

    virtual void update(uint64_t elapsed);
    virtual std::list<Cell*> onAdded(Cell* cell, Cell* old);
    virtual std::list<Cell*> onRemoved(Cell* cell, Cell* to);

protected:
    inline void cell(Cell* cell) { _cell = cell; }

protected:
    MotionMaster* _motionMaster;
    Client* _client;
    Cell* _cell;
    uint64_t _id;
};
