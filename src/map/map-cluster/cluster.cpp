/* Copyright 2016 Guillem Pascual */


#include "defs/atomic_autoincrement.hpp"
#include "defs/common.hpp"
#include "debug/debug.hpp"
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
{}

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

        for (int cid = 0; cid < _num_components; ++cid)
        {
            _pool.postWork<void>([this, cid, elapsed]()
            {
                for (auto pair : this->_vertices)
                {
                    if (this->_components[pair.second] == cid)
                    {
                        pair.first->update(elapsed);
                    }
                }
            });  // NOLINT (whitespace/braces)
        }

        _pool.waitAll();

        for (int cid = 0; cid < _num_components; ++cid)
        {
            _pool.postWork<void>([this, cid, elapsed]()
            {
                for (auto pair : this->_vertices)
                {
                    if (this->_components[pair.second] == cid)
                    {
                        pair.first->physics(elapsed);
                    }
                }
            });  // NOLINT (whitespace/braces)
        }

        _pool.waitAll();
    }
}


void Cluster::cleanup(uint64_t elapsed)
{
    if (!_vertices.empty())
    {
        for (int cid = 0; cid < _num_components; ++cid)
        {
            _pool.postWork<void>([this, cid, elapsed]()
            {
                for (auto pair : this->_vertices)
                {
                    if (this->_components[pair.second] == cid)
                    {
                        pair.first->cleanup(elapsed);
                    }
                }
            });  // NOLINT (whitespace/braces)
        }

        _pool.waitAll();

        // Reinitialize
        _graph = {};

        // Clear everything
        _vertices.clear();
        _mappings.clear();
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
        touch(entity->cell());

        for (auto nn : entity->cell()->inRadius(2))
        {
            if (nn && nn != entity->cell())
            {
                touch(nn);
                connect(entity->cell(), nn);
            }
        }
    }
}

void Cluster::touch(Cell* cell)
{
    if (_vertices.find(cell) == _vertices.end())
    {
        auto v = boost::add_vertex(_graph);
        _vertices[cell] = v;
        _mappings[v] = cell;
    }
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
