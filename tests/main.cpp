#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <catch.hpp>

#include <cell.hpp>
#include <cluster.hpp>
#include <map.hpp>
#include <map_aware_entity.hpp>

static uint32_t gID = 0;
class Entity : public MapAwareEntity
{
public:
    uint32_t id() { return _id; }

    void onAdded(Cell* cell) {}
    void onRemoved(Cell* cell) {}

private:
    uint32_t _id = gID++;
};

SCENARIO("Map cells can be created and eliminated", "[map]") {
    GIVEN("An empty map") {
        Map map(0, 0, 200, 50);

        REQUIRE(map.size() == 0);
        REQUIRE(map.scheduledSize() == 0);

        WHEN("one entity is added to the map") {
            map.addTo2D(0, 0, new Entity());

            THEN("the number of cells remains constant but scheduled operations increase") {
                REQUIRE(map.size() == 0);
                REQUIRE(map.scheduledSize() == 1);
            }
        }
    }

    GIVEN("A map with one add operation pending") {
        Map map(0, 0, 200, 50);
        map.addTo2D(0, 0, new Entity());

        REQUIRE(map.size() == 0);
        REQUIRE(map.scheduledSize() == 1);

        WHEN("operations are ran") {
            map.runScheduledOperations();

            THEN("the numbers of cells increase and pending operations are 0") {
                REQUIRE(map.size() > 0);
                REQUIRE(map.scheduledSize() == 0);
            }

            THEN("7 cells are created, the original and 6 siblings") {
                REQUIRE(map.size() == 7);
            }
        }
    }
}
