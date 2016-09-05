#include <catch.hpp>
#include "entity.hpp"

#include <cell.hpp>
#include <cluster.hpp>
#include <map.hpp>


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
