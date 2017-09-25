/* Copyright 2016 Guillem Pascual */

#pragma once

#include "defs/common.hpp"

#include <inttypes.h>
#include <list>
#include <queue>
#include <vector>
#include <initializer_list>
#include <glm/glm.hpp>


class BoundingBox;
class Cell;
class Client;
class MapAwareEntity;
class MotionMaster;
class Packet;

// TODO(gpascualg): Most probably, this should be moved somewhere else
using SchedulableTask = void(*)(MapAwareEntity*);
struct Schedulable
{
    bool operator<(const Schedulable& other) const
    {
        return when < other.when;
    }

    SchedulableTask task;
    TimePoint when;
};

struct Comp
{
    bool operator()(const Schedulable* a, const Schedulable* b)
    {
        return a->when < b->when;
    }
};

class MapAwareEntity
{
    friend class Map;

public:
    explicit MapAwareEntity(uint64_t id, Client* client = nullptr);
    virtual ~MapAwareEntity();

    inline Cell* cell() { return _cell; }
    inline Client* client() { return _client; }
    inline MotionMaster* motionMaster() { return _motionMaster; }
	inline BoundingBox* boundingBox() { return _boundingBox; }
    inline uint64_t id() { return _id; }

    void setupBoundingBox(std::initializer_list<glm::vec2>&& vertices);

    virtual void update(uint64_t elapsed);
    virtual std::vector<Cell*> onAdded(Cell* cell, Cell* old);
    virtual std::vector<Cell*> onRemoved(Cell* cell, Cell* to);

    void schedule(SchedulableTask&& task, TimePoint when);

    virtual Packet* spawnPacket() = 0;
    virtual Packet* despawnPacket() = 0;

    inline bool isUpdater() { return _isUpdater; }

protected:
    inline void cell(Cell* cell) { _cell = cell; }

protected:
    MotionMaster* _motionMaster;
	BoundingBox* _boundingBox;
    Client* _client;
    Cell* _cell;
    uint64_t _id;

    std::priority_queue<Schedulable*, std::vector<Schedulable*>, Comp> _scheduledTasks;
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
