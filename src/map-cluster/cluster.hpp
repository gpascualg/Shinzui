#pragma once

#include <array>
#include <functional>
#include <map>
#include <vector>
#include <unordered_map>


class ClusterCenter;
class Cell;
class Map;


class Cluster
{
    friend class Map;

public:
    virtual ~Cluster();

    void add(Cell* node, std::vector<Cell*>& siblings);
    void update(uint64_t elapsed);

    inline uint32_t size() { return _uniqueClusters.size(); }

private:
    Cluster();

    uint16_t propagate(Cell* center, std::function<bool(Cell*)> fnc);
    std::vector<Cell*>& getRing(Cell* center, uint16_t radius, bool invalidate = false, bool recreate = true);

private:
    uint32_t _numClusters = 0;
    uint32_t _fetchCurrent = 0;

    std::vector<ClusterCenter*> _uniqueClusters;
    std::unordered_map<Cell*, std::unordered_map<uint16_t, std::vector<Cell*>>> _cache;
    std::vector<Cell*> _placeHolder;
};
