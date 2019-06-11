/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>
#include <list>
#include <vector>

#include "debug/debug.hpp"
#include "map/map_aware_entity.hpp"
#include "physics/methods.hpp"
#include "physics/sat_collisions.hpp"

#include "defs/common.hpp"

INCL_NOWARN
#include <glm/glm.hpp>
INCL_WARN


template <int MaxEntities, int MaxDepth>
class QuadTree
{
public:
    void insert(MapAwareEntity* entity);
    void retrieve(std::list<MapAwareEntity*>& entities, glm::vec4 rect);  // NOLINT(runtime/references)
    void trace(std::list<MapAwareEntity*>& entities, glm::vec2 start, glm::vec2 end);  // NOLINT(runtime/references)
    void clear();

protected:
    QuadTree(int depth, glm::vec4 bounds);

    std::vector<QuadTree<MaxEntities, MaxDepth>*> _nodes;
    std::list<MapAwareEntity*> _entities;

    int getIndex(glm::vec4 rect);
    bool intersects(glm::vec2 start, glm::vec2 end);
    void split();

private:
    const int _depth;
    const glm::vec4 _bounds;  // (x0, y0, x1, y1) ~ (x, y, z, w)
    std::vector<glm::vec2> _vertices;
};

template <int MaxEntities, int MaxDepth>
class RadialQuadTree : public QuadTree<MaxEntities, MaxDepth>
{
public:
    RadialQuadTree(glm::vec2 center, float radius);
    bool contains(glm::vec2 pos);

private:
    const glm::vec2 _center;
    const float _radiusSqr;
};


template <int MaxEntities, int MaxDepth>
QuadTree<MaxEntities, MaxDepth>::QuadTree(int depth, glm::vec4 bounds) :
    _depth(depth),
    _bounds(bounds)
{
    auto width = _bounds.z - _bounds.x;
    auto height = _bounds.w - _bounds.y;

    _vertices.push_back({ bounds.x, bounds.y });                   // NOLINT(whitespace/braces)
    _vertices.push_back({ bounds.x, bounds.y + height });          // NOLINT(whitespace/braces)
    _vertices.push_back({ bounds.x + width, bounds.y + height });  // NOLINT(whitespace/braces)
    _vertices.push_back({ bounds.x + width, bounds.y });           // NOLINT(whitespace/braces)
}

template <int MaxEntities, int MaxDepth>
void QuadTree<MaxEntities, MaxDepth>::insert(MapAwareEntity* entity)
{
    if (!_nodes.empty())
    {
        int index = getIndex(entity->rect());
        if (index != -1)
        {
            _nodes[index]->insert(entity);
            return;
        }
    }

    _entities.push_back(entity);

    if (_entities.size() > MaxEntities && _depth < MaxDepth)
    {
        if (_nodes.empty())
        {
            split();
        }

        for (auto it = _entities.begin(); it != _entities.end();)
        {
            auto entity = *it;

            int index = getIndex(entity->rect());
            if (index != -1)
            {
                _nodes[index]->insert(entity);
                it = _entities.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

template <int MaxEntities, int MaxDepth>
void QuadTree<MaxEntities, MaxDepth>::retrieve(std::list<MapAwareEntity*>& entities, glm::vec4 rect)  // NOLINT
{
    LOG(LOG_QUADTREE, "Retrieve call from (%f, %f) to (%f, %f)", rect.x, rect.y, rect.z, rect.w);

    if (!intersects({ rect.x, rect.y }, { rect.z, rect.y }) &&  // (x0, y0) -> (x1, y0)
        !intersects({ rect.x, rect.y }, { rect.z, rect.w }) &&  // (x0, y0) -> (x1, y1)
        !intersects({ rect.z, rect.y }, { rect.z, rect.w }) &&  // (x1, y0) -> (x1, y1)
        !intersects({ rect.x, rect.y }, { rect.z, rect.y }))    // (x0, y0) -> (x0, y1)
    {
        return;
    }

    for (auto*& node : _nodes)
    {
        node->retrieve(entities, rect);
    }

    entities.insert(entities.end(), _entities.begin(), _entities.end());
}

template <int MaxEntities, int MaxDepth>
void QuadTree<MaxEntities, MaxDepth>::trace(std::list<MapAwareEntity*>& entities, glm::vec2 start, glm::vec2 end)  // NOLINT
{
    LOG(LOG_QUADTREE, "Trace call from (%f, %f) to (%f, %f)", start.x, start.y, end.x, end.y);

    if (!intersects(start, end))
    {
        LOG(LOG_QUADTREE, "Does not intersect with (%f, %f, %f, %f)", _bounds.x, _bounds.y, _bounds.z, _bounds.w);
        return;
    }

    for (auto*& node : _nodes)
    {
        node->trace(entities, start, end);
    }

    entities.insert(entities.end(), _entities.begin(), _entities.end());
}

template <int MaxEntities, int MaxDepth>
int QuadTree<MaxEntities, MaxDepth>::getIndex(glm::vec4 rect)
{
    float verticalMidpoint = _bounds.x + ((_bounds.z - _bounds.x) / 2);
    float horizontalMidpoint = _bounds.y + ((_bounds.w - _bounds.y) / 2);

    // Object can completely fit within the top quadrants
    bool topQuadrant = (rect.y < horizontalMidpoint && rect.y + (rect.w - rect.y) < horizontalMidpoint);
    // Object can completely fit within the bottom quadrants
    bool bottomQuadrant = (rect.y > horizontalMidpoint);

    // Object can completely fit within the left quadrants
    if (rect.x < verticalMidpoint && rect.x + (rect.z - rect.x) < verticalMidpoint)
    {
        if (topQuadrant) return 1;
        if (bottomQuadrant) return 2;
    }
    // Object can completely fit within the right quadrants
    else if (rect.x > verticalMidpoint)
    {
        if (topQuadrant) return 0;
        if (bottomQuadrant) return 3;
    }

    return -1;
}

template <int MaxEntities, int MaxDepth>
bool QuadTree<MaxEntities, MaxDepth>::intersects(glm::vec2 start, glm::vec2 end)
{
    // Check inclusion
    if (start.x > _bounds.x && start.y > _bounds.y && end.x < _bounds.z && end.y < _bounds.w)
    {
        return true;
    }

    // Check segments
    auto width = _bounds.z - _bounds.x;
    auto height = _bounds.w - _bounds.y;

    if (::intersects({ _bounds.x, _bounds.y }, { _bounds.x, _bounds.y + height }, start, end))  // NOLINT
    {
        return true;
    }

    if (::intersects({ _bounds.x, _bounds.y + height }, { _bounds.x + width, _bounds.y + height }, start, end))  // NOLINT
    {
        return true;
    }

    if (::intersects({ _bounds.x + width, _bounds.y + height }, { _bounds.x + width, _bounds.y }, start, end))  // NOLINT
    {
        return true;
    }

    if (::intersects({ _bounds.x + width, _bounds.y }, { _bounds.x, _bounds.y }, start, end))  // NOLINT
    {
        return true;
    }

    return false;
}

template <int MaxEntities, int MaxDepth>
void QuadTree<MaxEntities, MaxDepth>::split()
{
    int subWidth = static_cast<int>((_bounds.z - _bounds.x) / 2);
    int subHeight = static_cast<int>((_bounds.w - _bounds.y) / 2);
    int x = static_cast<int>(_bounds.x);
    int y = static_cast<int>(_bounds.y);

    _nodes.reserve(4);
    _nodes.emplace_back(new QuadTree<MaxEntities, MaxDepth>(_depth + 1, { x + subWidth, y, subWidth, subHeight }));              // NOLINT
    _nodes.emplace_back(new QuadTree<MaxEntities, MaxDepth>(_depth + 1, { x, y, subWidth, subHeight }));                         // NOLINT
    _nodes.emplace_back(new QuadTree<MaxEntities, MaxDepth>(_depth + 1, { x, y + subHeight, subWidth, subHeight }));             // NOLINT
    _nodes.emplace_back(new QuadTree<MaxEntities, MaxDepth>(_depth + 1, { x + subWidth, y + subHeight, subWidth, subHeight }));  // NOLINT
}

template <int MaxEntities, int MaxDepth>
void QuadTree<MaxEntities, MaxDepth>::clear()
{
    _entities.clear();

    for (auto*& node : _nodes)
    {
        node->clear();
        // delete node;
    }

    // _nodes.clear();
}

template <int MaxEntities, int MaxDepth>
RadialQuadTree<MaxEntities, MaxDepth>::RadialQuadTree(glm::vec2 center, float radius) :
    QuadTree<MaxEntities, MaxDepth>(0, { center.x - radius, center.y - radius, center.x + radius, center.y + radius }),  // NOLINT
    _center(center),
    _radiusSqr(std::pow(radius, 2))
{}

template <int MaxEntities, int MaxDepth>
bool RadialQuadTree<MaxEntities, MaxDepth>::contains(glm::vec2 pos)
{
    auto distSqr = std::pow(pos.x - _center.x, 2) + std::pow(pos.y - _center.y, 2);
    return distSqr <= _radiusSqr;
}
