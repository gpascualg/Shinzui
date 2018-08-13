/* Copyright 2016 Guillem Pascual */


#include "defs/atomic_autoincrement.hpp"
#include "defs/common.hpp"
#include "debug/debug.hpp"
#include "debug/reactive.hpp"
#include "map/cell.hpp"
#include "map/map-cluster/cluster.hpp"
#include "map/map-cluster/cluster_center.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"
#include "map/offset.hpp"
#include "movement/motion_master.hpp"
#include "physics/rect_bounding_box.hpp"
#include "server/server.hpp"

#include <algorithm>
#include <set>
#include <vector>


Cluster::Cluster():
    _num_components(0)
{
    _currentCells = &_verticesBuffer_1;
    _oldCells = &_verticesBuffer_2;
}

Cluster::~Cluster()
{
    _pool.joinAll();
}

void Cluster::update(uint64_t elapsed)
{
    if (!_vertices.empty())
    {
        _components.resize(_vertices.size());
        _num_components = boost::connected_components(_graph, &_components[0]);

        // Order by cluster
        for (auto const& pair : this->_vertices)
        {
            _cellsByCluster[_components[pair.second]].push_back(pair.first);
        }

        for (uint16_t cid = 0; cid < _num_components; ++cid)
        {
            _pool.postWork<void>([this, elapsed, cells = _cellsByCluster[cid]]()
            {
                for (auto cell : cells)
                {
                    cell->update(elapsed);
                }
            });  // NOLINT (whitespace/braces)
        }

        _pool.waitAll();

        for (uint16_t cid = 0; cid < _num_components; ++cid)
        {
            _pool.postWork<void>([this, elapsed, cells = _cellsByCluster[cid]]()
            {
                for (auto cell : cells)
                {
                    cell->physics(elapsed);
                }
            });  // NOLINT (whitespace/braces)
        }

        _pool.waitAll();
    }

    Reactive::get()->onClusterUpdate(_num_components, _vertices.size() - _numStall, _numStall, _numStallCandidates);
}

void Cluster::cleanup(uint64_t elapsed)
{
    if (!_vertices.empty())
    {
        for (uint16_t cid = 0; cid < _num_components; ++cid)
        {
            _pool.postWork<void>([this, elapsed, cells = _cellsByCluster[cid]]()
            {
                for (auto cell : cells)
                {
                    cell->cleanup(elapsed);
                    
                    // Calculate stall condition, if needed
                    if (cell->stall.isRegistered)
                    {
                        if (elapsed < cell->stall.remaining)
                        {
                            cell->stall.remaining -= elapsed;
                        }
                        else
                        {
                            cell->stall.isOnCooldown = true;
                            cell->stall.isRegistered = false;
                        }
                    }
                }
            });  // NOLINT (whitespace/braces)
        }

        _pool.waitAll();

        // Reinitialize
        _graph = {};

        // Clean graph structures
        _vertices.clear();

        for (uint16_t cid = 0; cid < _num_components; ++cid)
        {
            _cellsByCluster[cid].clear();
        }

        _num_components = 0;
    }

    // Switch buffers and clean (always, to avoid keeping old cells)
    auto temp = _oldCells;
    _oldCells = _currentCells;
    _currentCells = temp;
    
    _currentCells->clear();
}

uint16_t Cluster::processStallCells(uint64_t elapsed)
{
    _numStallCandidates = _stallCells.size();
    uint16_t addCount = 0;
    
    // Insert old-now-missing cells
    for (auto it = _stallCells.begin(); it != _stallCells.end();)
    {
        auto cell = *it;

        // If its on cooldown it means that it has already exhausted
        // stall time, it can be freed up. Otherwise, try to add it to
        // the update queue (it might have already been added as a
        // nons-stall cell)
        if (!cell->stall.isOnCooldown)
        {
            if (touchWithNeighbours(cell, true))
            {
                ++addCount;
            }

            ++it;
        }
        else
        {
            // Memory can be freed up
            Server::get()->map()->destroyCell(cell);
            it = _stallCells.erase(it);
        }
    }

    return addCount;
}

void Cluster::runScheduledOperations(uint64_t elapsed)
{
    _scheduledOperations.consume_all([this](ClusterOperation op)
    {
        switch (op.type)
        {
            case ClusterOperationType::KEEP:
                this->_keepers.push_back(op.entity);
                break;

            case ClusterOperationType::UNKEEP:
                this->_keepers.erase(std::find(this->_keepers.begin(), this->_keepers.end(), op.entity));
                break;
        }
    });  // NOLINT (whitespace/braces)

    // Flag cells that must be updated (because they have keepers)
    for (auto entity : _keepers)
    {
        touchWithNeighbours(entity->cell());
    }

    // New stall cells
    // TODO(gpascualg): More efficient ways to do so?? As of now, it is O(n^2)
    for (const auto& candidate : *_oldCells)
    {
        if (_currentCells->find(candidate) != _currentCells->cend())
        {
            continue;
        }

        // If is is not really inserted yet
        auto insertion = _stallCells.insert(candidate);
        if (insertion.second)
        {
            candidate->stall.isRegistered = true;
            candidate->stall.isOnCooldown = false;
            candidate->stall.remaining = TimeBase(std::chrono::minutes(2)).count();
        }
    }

    // Process stalled cells, either new or old
    _numStall = processStallCells(elapsed);
}

void Cluster::checkStall(Cell* from, Cell* to)
{
    if (!from || !to->stall.isRegistered || to->stall.isOnCooldown)
    {
        return;
    }

    if (!from->stall.isRegistered && !from->stall.isOnCooldown)
    {
        // It got unstalled, remove statuses
        to->stall.isRegistered = false;
        to->stall.isOnCooldown = false;

        // It should be found, if it is registered, it is on vector
        _stallCells.erase(std::find(_stallCells.cbegin(), _stallCells.cend(), to));
    }
}

void Cluster::onCellCreated(Cell* cell)
{
    touchWithNeighbours(cell);
}

bool Cluster::touchWithNeighbours(Cell* cell, bool isStall)
{
    // If we can't touch the center, and it was stall, do not continue
    // that means it has already been processed as part of a keeper operation
    bool firstInserted = touch(cell, isStall);
    if (!firstInserted && isStall)
    {
        return false;
    }

    for (auto nn : cell->inRadius(1))
    {
        if (nn && nn != cell)
        {
            if (touch(nn, isStall))
            {
                connect(cell, nn);
            }
        }
    }

    return true;
}

bool Cluster::touch(Cell* cell, bool isStall)
{
    // It should not be non-stall and non-cooldown
    LOG_ASSERT(isStall || !cell->stall.isOnCooldown, "A cell is expected to either not be stall or not be in cooldown");
    
    // Only tru if the resulting vertice must be connected
    // false otherwise (which is the case of stall cells on cooldown)
    bool result = false;

    // Do not add if it is on cooldown (ie. to be deleted)    
    if (!cell->stall.isOnCooldown)
    {
        result = true;
        auto insertion = _currentCells->insert(cell);
        if (insertion.second)
        {
            auto v = boost::add_vertex(_graph);
            _vertices[cell] = v;
        }
    }

    // Whatever the result, check for stallness
    if (!isStall && (cell->stall.isRegistered || cell->stall.isOnCooldown))
    {
        cell->stall.isRegistered = false;
        cell->stall.isOnCooldown = false;
        _stallCells.erase(std::find(_stallCells.cbegin(), _stallCells.cend(), cell));
    }

    return result;
}

void Cluster::connect(Cell* a, Cell* b)
{
    LOG_ASSERT(_vertices.find(a) != _vertices.end(), "Trying to connect an untouched cell");
    LOG_ASSERT(_vertices.find(b) != _vertices.end(), "Trying to connect an untouched cell");

    boost::add_edge(_vertices[a], _vertices[b], _graph);
}

void Cluster::add(MapAwareEntity* entity, std::vector<Cell*> const& siblings)
{
    if (entity->isUpdater())
    {
        _scheduledOperations.push({  // NOLINT (whitespace/braces)
            ClusterOperationType::KEEP,
            entity
        });  // NOLINT (whitespace/braces)
    }
}

void Cluster::remove(MapAwareEntity* entity)
{
    if (entity->isUpdater())
    {
        _scheduledOperations.push({  // NOLINT (whitespace/braces)
            ClusterOperationType::UNKEEP,
            entity
        });  // NOLINT (whitespace/braces)
    }
}

