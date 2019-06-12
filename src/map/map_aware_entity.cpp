/* Copyright 2016 Guillem Pascual */

#include "map/cell.hpp"
#include "server/client.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"
#include "movement/motion_master.hpp"
#include "physics/rect_bounding_box.hpp"
#include "server/server.hpp"

#include <list>
#include <utility>
#include <vector>


MapAwareEntity::MapAwareEntity(uint64_t id, Client* client) :
    Executor<ExecutorQueueMax>(),
    _client(client),
    _id(id),
    _cell(nullptr),
    _boundingBox(nullptr)
{
    _isUpdater = client != nullptr;
}

MapAwareEntity::~MapAwareEntity()
{}

void MapAwareEntity::update(uint64_t elapsed)
{
    Executor<ExecutorQueueMax>::executeJobs();

    // Update motion
    // TODO(gpascualg): UPDATE MOVEMENT!
    // _motionMaster->update(elapsed);
}

void MapAwareEntity::setupBoundingBox(std::initializer_list<glm::vec2>&& vertices)
{
    // TODO(gpascualg): Logging friendly assert
    LOG_ALWAYS("SETUP BBOX FOR %" PRId64, id());
    assert(_boundingBox == nullptr);
    _boundingBox = new RectBoundingBox(std::move(vertices));
}

std::vector<Cell*> MapAwareEntity::onAdded(Cell* cell, Cell* old)
{
    _cell = cell;

    Packet* packet = spawnPacket();

    LOG(LOG_CELL_CHANGES, "MapAwareEntity::onAdded (%" PRId64 ")", id());

    // Broadcast all packets
    auto newCells = cell->map()->getCellsExcluding(cell, old);
    Server::get()->map()->broadcast(newCells, packet, [this](Cell* cell)
        {
            LOG(LOG_CELL_CHANGES, "RequestType::SPAWN (%d, %d)", cell->offset().q(), cell->offset().r());
            if (client())
            {
                cell->request(this, RequestType::SPAWN);
            }
        }
    );  // NOLINT(whitespace/parens)

    return newCells;
}

std::vector<Cell*> MapAwareEntity::onRemoved(Cell* cell, Cell* to)
{
    _cell = nullptr;

    Packet* packet = despawnPacket();

    LOG(LOG_CELL_CHANGES, "MapAwareEntity::onRemoved (%" PRId64 ")", id());

    // Broadcast all packets
    auto oldCells = cell->map()->getCellsExcluding(cell, to);

    // Do not request despawn paquets from old cells if we are disconnecting
    // or we are not a client
    if (to && client())
    {
        Server::get()->map()->broadcast(oldCells, packet, [this](Cell* cell)
            {
                LOG(LOG_CELL_CHANGES, "RequestType::DESPAWN (%d, %d)", cell->offset().q(), cell->offset().r());
                cell->request(this, RequestType::DESPAWN);
            }
        );  // NOLINT(whitespace/parens)
    }
    else
    {
        Server::get()->map()->broadcast(oldCells, packet);
    }

    return oldCells;
}

TimePoint MapAwareEntity::lag()
{
    TimePoint now = Server::get()->now();
    if (_client)
    {
        now -= TimeBase(_client->lag());
    }
    return now;
}

FixedTransform& MapAwareEntity::transform()
{
    return _transform.at(lag(), _boundingBox);
}

void MapAwareEntity::teleport(glm::vec3 position, glm::vec3 forward)
{
    _transform.teleport(lag(), position, forward);
}

void MapAwareEntity::move(float speed)
{
    _transform.move(lag(), speed);
}

void MapAwareEntity::rotate(float angle)
{
    _transform.rotate(lag(), angle);
}

void MapAwareEntity::stop()
{
    _transform.stop(lag());
}
