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
    _client(client),
    _id(id),
    _cell(nullptr),
    _boundingBox(nullptr),
    Executor<ExecutorQueueMax>()
{
    _motionMaster = new MotionMaster(this);
    _isUpdater = client != nullptr;
}

MapAwareEntity::~MapAwareEntity()
{
    delete _motionMaster;
}

void MapAwareEntity::update(uint64_t elapsed)
{
    Executor<ExecutorQueueMax>::runScheduled();

    // Update motion
    _motionMaster->update(elapsed);
}

void MapAwareEntity::setupBoundingBox(std::initializer_list<glm::vec2>&& vertices)
{
    // TODO(gpascualg): Logging friendly assert
    assert(_boundingBox == nullptr);
    _boundingBox = new RectBoundingBox(_motionMaster, std::move(vertices));
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
    Server::get()->map()->broadcast(oldCells, packet, [this](Cell* cell)
        {
            LOG(LOG_CELL_CHANGES, "RequestType::DESPAWN (%d, %d)", cell->offset().q(), cell->offset().r());
            if (client())
            {
                cell->request(this, RequestType::DESPAWN);
            }
        }
    );  // NOLINT(whitespace/parens)

    return oldCells;
}
