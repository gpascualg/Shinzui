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
    _verticesCells = &_verticesBuffer_1;
}

Cluster::~Cluster()
{
    _pool.joinAll();
}

void Cluster::update(uint64_t elapsed)
{
    // Compute diff between buffers
    _numStall = processStallCells(elapsed);

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

uint16_t Cluster::processStallCells(uint64_t elapsed)
{
    const std::unordered_set<Cell*>& candidates = (_verticesCells == &_verticesBuffer_1) ? _verticesBuffer_2 : _verticesBuffer_1;
    _numStallCandidates = candidates.size();
    uint16_t addCount = 0;

    // Insert old-now-missing cells
    for (auto const& cell : candidates)
    {
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
        
        if (!cell->stall.isOnCooldown)
        {
            if (touchWithNeighbours(cell, true))
            {
                ++addCount;
            }
        }
    }

    return addCount;
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
                }
            });  // NOLINT (whitespace/braces)
        }

        _pool.waitAll();

        // Reinitialize
        _graph = {};

        // Switch buffers and clean
        _verticesCells = (_verticesCells == &_verticesBuffer_1) ? &_verticesBuffer_2 : &_verticesBuffer_1;
        _verticesCells->clear();
        _vertices.clear();

        for (uint16_t cid = 0; cid < _num_components; ++cid)
        {
            _cellsByCluster[cid].clear();
        }

        _num_components = 0;
    }
}

void Cluster::runScheduledOperations()
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

    for (auto entity : _keepers)
    {
        touchWithNeighbours(entity->cell());
    }
}

void Cluster::checkStall(Cell* from, Cell* to)
{
    if (!from)
    {
        return;
    }

    if (!from->stall.isRegistered && !from->stall.isOnCooldown)
    {
        to->stall.isRegistered = false;
        to->stall.isOnCooldown = false;
    }
}

void Cluster::onCellCreated(Cell* cell)
{
    touchWithNeighbours(cell);
}

bool Cluster::touchWithNeighbours(Cell* cell, bool isStall)
{
    // If we can't touch the center, and it was stall, do not continue
    // that means it has already been processed as part of a
    // keeper operation
    bool firstInserted = touch(cell, isStall);
    if (!firstInserted && isStall)
    {
        return false;
    }

    for (auto nn : cell->inRadius(1))
    {
        if (nn && nn != cell)
        {
            touch(nn, isStall);
            connect(cell, nn);
        }
    }

    return true;
}

bool Cluster::touch(Cell* cell, bool isStall)
{
    auto insertion = _verticesCells->insert(cell);
    if (insertion.second)
    {
        auto v = boost::add_vertex(_graph);
        _vertices[cell] = v;

        if (isStall && !cell->stall.isRegistered && !cell->stall.isOnCooldown)
        {
            cell->stall.isRegistered = true;
            cell->stall.remaining = TimeBase(std::chrono::minutes(2)).count();
        }
        else if (!isStall)
        {
            cell->stall.isRegistered = false;
            cell->stall.isOnCooldown = false;
        }
    }

    return insertion.second;
}

void Cluster::connect(Cell* a, Cell* b)
{
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
