/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>
#include <list>
#include <vector>
#include <glm/glm.hpp>


class Cell;
class Client;
class MotionMaster;
class Packet;


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
    virtual std::vector<Cell*> onAdded(Cell* cell, Cell* old);
    virtual std::vector<Cell*> onRemoved(Cell* cell, Cell* to);

    virtual Packet* spawnPacket() = 0;
    virtual Packet* despawnPacket() = 0;

    inline bool isUpdater() { return _isUpdater; }

protected:
    inline void cell(Cell* cell) { _cell = cell; }

protected:
    MotionMaster* _motionMaster;
    Client* _client;
    Cell* _cell;
    uint64_t _id;

    bool _isUpdater;
};


class DummyUpdater : public MapAwareEntity
{
public:
    using MapAwareEntity::MapAwareEntity;

    Packet* spawnPacket() override { return nullptr; }
    Packet* despawnPacket() override { return nullptr; }

    void triggerUpdater() { _isUpdater = true; }
};
