#include "map.hpp"
#include "cell.hpp"
#include "debug.hpp"
#include "cluster.hpp"
#include "operations.hpp"
#include "map_aware_entity.hpp"

#include <algorithm>
#include <iterator>
#include <boost/lockfree/queue.hpp>

Map::Map(int32_t x, int32_t y, uint32_t dx, uint32_t dy) :
    _x(x), _y(y),
    _dx(dx), _dy(dy)
{
    _cluster = new Cluster();
    _scheduledOperations = new QueueWithSize<MapOperation*>(2048);
}

Map::~Map()
{
    delete _cluster;
    delete _scheduledOperations;
}

void Map::runScheduledOperations()
{
    MapOperation* operation;
    while (_scheduledOperations->pop(operation))
    {
        Cell* cell = nullptr;

        switch (operation->type) {
            case MapOperationType::ADD_ENTITY_CREATE:
                cell = getOrCreate(std::move(operation->offset), true);
                if (cell)
                {
                    cell->_data.emplace(operation->entity->id(), operation->entity);
                    operation->entity->onAdded(cell);
                }
                break;

            case MapOperationType::REMOVE_ENTITY:
                cell = get(std::move(operation->offset));
                if (cell)
                {
                    cell->_data.erase(operation->entity->id());
                    operation->entity->onRemoved(cell);
                }
                break;

            case MapOperationType::DESTROY:
                // TODO: Not implemented yet
                break;

            default:
                // TODO Unkown operation error
                break;
        }
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
    _scheduledOperations->push(new MapOperation {
        MapOperationType::ADD_ENTITY_CREATE,
        offset,
        e
    });
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

Cell* Map::getOrCreate(int32_t q, int32_t r, bool siblings)
{
    return getOrCreate(Offset(q, r), siblings);
}

Cell* Map::getOrCreate(const Offset& offset, bool siblings)
{
    auto cell = get(offset);
    if (!cell)
    {
        auto result = _cells.emplace(offset.hash(), new Cell(this, std::move(offset)));
        cell = (*result.first).second;
    }

    if (siblings && !cell->_siblingsDone)
    {
        LOG(LOG_CELLS, "Creating siblings");

        auto siblings = createSiblings(cell);
        _cluster->add(cell, siblings);
        cell->_siblingsDone = true;
    }

    return cell;
}

std::vector<Cell*> Map::createSiblings(Cell* cell)
{
    const Offset& offset = cell->offset();
    int32_t q = offset.q();
    int32_t r = offset.r();

    return {
        getOrCreate(q + 0, r - 1, false),
        getOrCreate(q + 1, r - 1, false),
        getOrCreate(q + 1, r + 0, false),
        getOrCreate(q + 0, r + 1, false),
        getOrCreate(q - 1, r + 1, false),
        getOrCreate(q - 1, r + 0, false),
    };
}
