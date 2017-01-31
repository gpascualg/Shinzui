/* Copyright 2016 Guillem Pascual */

#include "map.hpp"
#include "cell.hpp"
#include "debug.hpp"
#include "cluster.hpp"
#include "map_operation.hpp"
#include "map_aware_entity.hpp"

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>
#include <boost/lockfree/queue.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>


Map::Map(boost::object_pool<Cell>* cellAllocator)
{
    _cellAllocator = cellAllocator ? cellAllocator : new boost::object_pool<Cell>(2048);
    _cluster = new Cluster();
    _scheduledOperations = new QueueWithSize<MapOperation*>(2048);
}

Map::~Map()
{
    delete _cellAllocator;
    delete _cluster;
    delete _scheduledOperations;
}

void Map::runScheduledOperations()
{
    MapOperation* operation;
    while (_scheduledOperations->pop(operation))
    {
        Cell* cell = nullptr;

        switch (operation->type)
        {
            case MapOperationType::ADD_ENTITY_CREATE:
                cell = getOrCreate(std::move(operation->offset));
                if (cell)
                {
                    operation->entity->onAdded(cell);

                    if (operation->entity->client())
                    {
                        cell->_playerData.emplace(operation->entity->id(), operation->entity);
                        cluster()->add(operation->entity, createSiblings(cell));
                    }
                    else
                    {
                        cell->_data.emplace(operation->entity->id(), operation->entity);
                    }
                }
                break;

            case MapOperationType::REMOVE_ENTITY:
                cell = get(std::move(operation->offset));
                if (cell)
                {
                    auto& data = cell->_data;
                    if (operation->entity->client())
                    {
                        cell->_playerData.erase(operation->entity->id());
                    }
                    else
                    {
                        cell->_data.erase(operation->entity->id());
                    }

                    operation->entity->onRemoved(cell);
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

void Map::addTo2D(int32_t x, int32_t y, MapAwareEntity* e)
{
    addTo(offsetOf(x, y), e);
}

void Map::addTo(int32_t q, int32_t r, MapAwareEntity* e)
{
    addTo(Offset(q, r), e);
}

void Map::addTo(const Offset&& offset, MapAwareEntity* e)
{
    _scheduledOperations->push(new MapOperation {  // NOLINT(whitespace/braces)
        MapOperationType::ADD_ENTITY_CREATE,
        offset,
        e
    });  // NOLINT(whitespace/braces)
}

Cell* Map::get(int32_t q, int32_t r)
{
    return get(Offset(q, r));
}

Cell* Map::get(const Offset& offset)
{
    auto it = _cells.find(offset.hash());

    if (it == _cells.end())
    {
        return nullptr;
    }

    return (*it).second;
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
        auto result = _cells.emplace(offset.hash(), _cellAllocator->construct(this, offset));
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
