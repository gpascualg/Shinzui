/* Copyright 2016 Guillem Pascual */

#include "map/map.hpp"
#include "map/cell.hpp"
#include "debug/debug.hpp"
#include "map/map-cluster/cluster.hpp"
#include "map/map_operation.hpp"
#include "map/map_aware_entity.hpp"
#include "movement/motion_master.hpp"

#include <algorithm>
#include <iterator>
#include <list>
#include <utility>
#include <vector>

#include "defs/common.hpp"

INCL_NOWARN
#include <boost/lockfree/queue.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>
INCL_WARN


Map::Map(boost::object_pool<Cell>* cellAllocator)
{
    _cellAllocator = cellAllocator ? cellAllocator : new boost::object_pool<Cell>(2048);
    _cluster = new Cluster();
    _scheduledOperations = new boost::lockfree::queue<MapOperation*>(2048);
}

Map::~Map()
{
    delete _cellAllocator;
    delete _cluster;
    delete _scheduledOperations;
}

void Map::update(uint64_t elapsed)
{
    runScheduledOperations();
    cluster()->update(elapsed);
}

void Map::cleanup(uint64_t elapsed)
{
    cluster()->cleanup(elapsed);
    cluster()->runScheduledOperations();
}

void Map::runScheduledOperations()
{
    MapOperation* operation;
    while (_scheduledOperations->pop(operation))
    {
        Cell* cell = nullptr;
        auto entity = operation->entity;

        switch (operation->type)
        {
            case MapOperationType::ADD_ENTITY:
                cell = getOrCreate(std::move(operation->offset));
                if (cell)
                {
                    entity->cell(cell);
                    cell->_entities.emplace(entity->id(), entity);

                    if (entity->isUpdater())
                    {
                        cluster()->add(entity, createSiblings(cell));
                    }

                    entity->onAdded(cell, operation->param);
                }
                break;

            case MapOperationType::REMOVE_ENTITY:
                cell = get(std::move(operation->offset));
                if (cell)
                {
                    cell->_entities.erase(entity->id());

                    if (entity->isUpdater())
                    {
                        cluster()->remove(entity);
                    }

                    entity->onRemoved(cell, operation->param);
                }
                break;

            case MapOperationType::DESTROY:
                // TODO(gpascualg): Not implemented yet
                break;

            default:
                // TODO(gpascualg): Unkown operation error
                break;
        }

        delete operation;
    }
}

void Map::broadcastToSiblings(Cell* cell, boost::intrusive_ptr<Packet> packet)
{
    cell->broadcast(packet);

    auto cells = getSiblings(cell);
    broadcast(cells, packet);
}

void Map::broadcastExcluding(Cell* cell, Cell* exclude, boost::intrusive_ptr<Packet> packet)
{
    auto cells = getCellsExcluding(cell, exclude);
    broadcast(cells, packet);
}

std::vector<Cell*> Map::getCellsExcluding(Cell* cell, Cell* exclude)
{
    if (!exclude || !cell)
    {
        auto ref = cell ? cell : exclude;
        auto cells = createSiblings(ref);
        cells.push_back(ref);
        return cells;
    }

    auto offsetCell = cell->offset();
    auto offsetExclude = exclude->offset();
    auto directionQ = offsetCell.q() - offsetExclude.q();
    auto directionR = offsetCell.r() - offsetExclude.r();

    auto idx = directionIdxs(directionQ, directionR);
    auto i = idx == 0 ? MAX_DIR_IDX - 1 : idx - 1;
    auto j = (idx + 1) % MAX_DIR_IDX;

    auto cell1 = getOrCreate({ offsetCell.q() + directionQ,  // NOLINT(whitespace/braces)
        offsetCell.r() + directionR });  // NOLINT(whitespace/braces)

    auto cell2 = getOrCreate({ offsetCell.q() + directions[i].q,  // NOLINT(whitespace/braces)
        offsetCell.r() + directions[i].r });  // NOLINT(whitespace/braces)

    auto cell3 = getOrCreate({ offsetCell.q() + directions[j].q,  // NOLINT(whitespace/braces)
        offsetCell.r() + directions[j].r });  // NOLINT(whitespace/braces)

    return { cell1, cell2, cell3 };
}

void Map::onMove(MapAwareEntity* entity)
{
    auto& pos = entity->motionMaster()->position();
    auto offset = offsetOf(pos.x, pos.z);
    Cell* cell = getOrCreate(offset);

    if (cell != entity->cell())
    {
        removeFrom(entity->cell(), entity, cell);
        addTo(cell, entity, entity->cell());
    }
}

void Map::addTo(MapAwareEntity* e, Cell* old)
{
    addTo3D(e->motionMaster()->position(), e, old);
}

void Map::addTo3D(const glm::vec3& pos, MapAwareEntity* e, Cell* old)
{
    addTo(offsetOf(pos.x, pos.z), e, old);
}

void Map::addTo(int32_t q, int32_t r, MapAwareEntity* e, Cell* old)
{
    // TODO(gpascualg): Deprecate
    addTo(Offset(q, r), e, old);
}

void Map::addTo(const Offset&& offset, MapAwareEntity* e, Cell* old)
{
    _scheduledOperations->push(new MapOperation {  // NOLINT(whitespace/braces)
        MapOperationType::ADD_ENTITY,
        offset,
        e,
        old
    });  // NOLINT(whitespace/braces)
}

void Map::addTo(Cell* cell, MapAwareEntity* e, Cell* old)
{
    _scheduledOperations->push(new MapOperation{  // NOLINT(whitespace/braces)
        MapOperationType::ADD_ENTITY,
        cell->offset(),
        e,
        old
    });  // NOLINT(whitespace/braces)
}

void Map::removeFrom(MapAwareEntity* e, Cell* to)
{
    removeFrom3D(e->motionMaster()->position(), e, to);
}

void Map::removeFrom3D(const glm::vec3& pos, MapAwareEntity* e, Cell* to)
{
    removeFrom(offsetOf(pos.x, pos.z), e, to);
}

void Map::removeFrom(int32_t q, int32_t r, MapAwareEntity* e, Cell* to)
{
    removeFrom(Offset(q, r), e, to);
}

void Map::removeFrom(const Offset&& offset, MapAwareEntity* e, Cell* to)
{
    _scheduledOperations->push(new MapOperation{  // NOLINT(whitespace/braces)
        MapOperationType::REMOVE_ENTITY,
        offset,
        e,
        to
    });  // NOLINT(whitespace/braces)
}

void Map::removeFrom(Cell* cell, MapAwareEntity* e, Cell* to)
{
    _scheduledOperations->push(new MapOperation{  // NOLINT(whitespace/braces)
        MapOperationType::REMOVE_ENTITY,
        cell->offset(),
        e,
        to
    });  // NOLINT(whitespace/braces)
}

Cell* Map::get(int32_t q, int32_t r)
{
    return get(Offset(q, r));
}

Cell* Map::get(const Offset& offset)
{
    auto it = _cells.find(std::make_pair(offset.q(), offset.r()));
    if (it == _cells.end())
    {
        return nullptr;
    }

    auto cell = (*it).second;
    return cell;
}

Cell* Map::getOrCreate(int32_t q, int32_t r)
{
    return getOrCreate(Offset(q, r));
}

Cell* Map::getOrCreate(const Offset& offset)
{
    auto cell = get(offset);
    if (!cell)
    {
        // Allocate cell
        auto result = _cells.emplace(std::make_pair(offset.q(), offset.r()), _cellAllocator->construct(this, offset));
        cell = (*result.first).second;
    }

    return cell;
}

std::vector<Cell*> Map::createSiblings(Cell* cell)
{
    const Offset& offset = cell->offset();
    int32_t q = offset.q();
    int32_t r = offset.r();

    return {  // NOLINT(whitespace/braces)
        getOrCreate(q + 0, r - 1),
        getOrCreate(q + 1, r - 1),
        getOrCreate(q + 1, r + 0),
        getOrCreate(q + 0, r + 1),
        getOrCreate(q - 1, r + 1),
        getOrCreate(q - 1, r + 0),
    };
}

std::vector<Cell*> Map::getSiblings(Cell* cell)
{
    const Offset& offset = cell->offset();
    int32_t q = offset.q();
    int32_t r = offset.r();

    return{  // NOLINT(whitespace/braces)
        get(q + 0, r - 1),
        get(q + 1, r - 1),
        get(q + 1, r + 0),
        get(q + 0, r + 1),
        get(q - 1, r + 1),
        get(q - 1, r + 0),
    };
}
