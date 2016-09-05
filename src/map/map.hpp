#pragma once

#include "offset.hpp"
#include "boost/lockfree/lockfree_forward.hpp"
#include "boost/pool/pool_forward.hpp"

#include <inttypes.h>
#include <vector>
#include <unordered_map>


class Cell;
class CellAllocator;
class Cluster;
class Map;
class MapAwareEntity;
struct MapOperation;

#if BUILD_TESTS==ON
    #include <boost/lockfree/queue.hpp>
    #include <atomic>

    template <typename T>
    class QueueWithSize
    {
    public:
        QueueWithSize(uint32_t capacity):
            _queue(capacity),
            _counter{ 0 }
        {}

        bool push(T const & t)
        {
            if (_queue.push(t))
            {
                ++_counter;
                return true;
            }
            return false;
        }

        bool pop(T& t)
        {
            if (_queue.pop(t))
            {
                --_counter;
                return true;
            }
            return false;
        }

        inline uint32_t size() { return _counter; }

    private:
        boost::lockfree::queue<T> _queue;
        std::atomic<uint32_t> _counter;
    };
#else
    using QueueWithSize<T> = boost::lockfree::queue<T>;
#endif


class Map
{
public:
    Map(boost::object_pool<Cell>* cellAllocator = nullptr);
    Map(const Map& map) = delete;
    virtual ~Map();

    void runScheduledOperations();

    // Schedules an ADD (and maybe CREATE) operations
    void addTo2D(int32_t x, int32_t y, MapAwareEntity* e);
    void addTo(int32_t q, int32_t r, MapAwareEntity* e);
    void addTo(const Offset&& offset, MapAwareEntity* e);

    // NOT thread-safe
    // Gets a cell from the map
    Cell* get(int32_t q, int32_t r);
    Cell* get(const Offset& offset);

    // NOT thread-safe
    // Gets or creates a cell from the map
    Cell* getOrCreate(int32_t q, int32_t r, bool siblings = true);
    Cell* getOrCreate(const Offset& offset, bool siblings = true);

    // NOT thread-safe
    // Creates siblings for a cell
    std::vector<Cell*> createSiblings(Cell* cell);

    inline Cluster* cluster() { return _cluster; }
    inline uint32_t size() { return _cells.size(); }

#if BUILD_TESTS==ON
    inline uint32_t scheduledSize() { return _scheduledOperations->size(); }
#endif

private:
    boost::object_pool<Cell>* _cellAllocator;
    Cluster* _cluster;

    std::unordered_map<uint64_t /*hash*/, Cell*> _cells;
    QueueWithSize<MapOperation*>* _scheduledOperations;
};
