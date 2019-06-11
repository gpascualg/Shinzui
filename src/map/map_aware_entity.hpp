/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>
#include <list>
#include <queue>
#include <vector>
#include <initializer_list>

#include "defs/common.hpp"
#include "debug/debug.hpp"
#include "executor/executor.hpp"
#include "time/traceable.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN


class BoundingBox;
class Cell;
class Client;
class MapAwareEntity;
class Transform;
class Packet;


// Using 16-queued jobs per-cicle&client should be more than enough
constexpr const uint16_t ExecutorQueueMax = 16;
class MapAwareEntity : public Executor<ExecutorQueueMax>
{
    friend class Map;

public:
    explicit MapAwareEntity(uint64_t id, Client* client = nullptr);
    virtual ~MapAwareEntity();

    inline Cell* cell();
    inline Client* client();
    inline BoundingBox* boundingBox();
    inline Transform& transform(TimePoint t);
    inline uint64_t id();

    void setupBoundingBox(std::initializer_list<glm::vec2>&& vertices);

    virtual void update(uint64_t elapsed);
    virtual std::vector<Cell*> onAdded(Cell* cell, Cell* old);
    virtual std::vector<Cell*> onRemoved(Cell* cell, Cell* to);

    glm::vec4 rect();
    glm::vec3 position();
    glm::vec2 position2D();
    glm::vec3 forward();

    virtual Packet* spawnPacket() = 0;
    virtual Packet* despawnPacket() = 0;

    inline bool isUpdater() { return _isUpdater; }

protected:
    inline void cell(Cell* cell) { _cell = cell; }

protected:
    Client* _client;
    uint64_t _id;
    Cell* _cell;
    BoundingBox* _boundingBox;
    Traceable<Transform> _transform;    

    bool _isUpdater;
};


Cell* MapAwareEntity::cell()
{
    return _cell;
}

Client* MapAwareEntity::client()
{
    return _client;
}

BoundingBox* MapAwareEntity::boundingBox()
{
    return _boundingBox;
}

Transform& MapAwareEntity::transform(TimePoint t)
{
    return _transform.at(t);
}

uint64_t MapAwareEntity::id()
{
    return _id;
}


class DummyUpdater : public MapAwareEntity
{
public:
    using MapAwareEntity::MapAwareEntity;

    Packet* spawnPacket() override { return nullptr; }
    Packet* despawnPacket() override { return nullptr; }

    void triggerUpdater() { _isUpdater = true; }
};
