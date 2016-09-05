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
            Entity e;
            map.addTo2D(0, 0, &e);

            THEN("the number of cells remains constant but scheduled operations increase") {
                REQUIRE(map.size() == 0);
                REQUIRE(map.scheduledSize() == 1);
            }

            THEN("entity is not flagged as added") {
                REQUIRE(!e.hasBeenAdded);
            }
        }

        WHEN("one entity is added to the map using hex coordinates") {
            Entity e;
            map.addTo(0, 0, &e);

            THEN("the number of cells remains constant but scheduled operations increase") {
                REQUIRE(map.size() == 0);
                REQUIRE(map.scheduledSize() == 1);
            }

            THEN("entity is not flagged as added") {
                REQUIRE(!e.hasBeenAdded);
            }
        }
    }

    GIVEN("A map with one add operation pending") {
        Map map(0, 0, 200, 50);
        Entity e;
        map.addTo2D(0, 0, &e);

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

            THEN("entity is flagged as added") {
                REQUIRE(e.hasBeenAdded);
            }
        }
    }

    GIVEN("A map with one cell") {
        Map map(0, 0, 200, 50);
        map.addTo2D(0, 0, new Entity());
        map.runScheduledOperations();

        REQUIRE(map.size() == 7);
        REQUIRE(map.scheduledSize() == 0);

        WHEN("the cluster is ran") {
            map.cluster()->update(0);

            THEN("cluster has size 1") {
                REQUIRE(map.cluster()->size() == 1);
            }
        }
    }

    GIVEN("A map with two cells and one pending which should merge") {
        Map map(0, 0, 200, 50);
        map.addTo(0, 0, new Entity());
        map.addTo(4, 0, new Entity());
        map.runScheduledOperations();

        // TODO: Make its own test
        map.cluster()->update(0);

        map.addTo(2, 0, new Entity());

        REQUIRE(map.size() == 14);
        REQUIRE(map.scheduledSize() == 1);
        REQUIRE(map.cluster()->size() == 2);

        WHEN("scheduled operations are ran") {
            map.runScheduledOperations();

            THEN("only one cluster should remain") {
                REQUIRE(map.cluster()->size() == 1);
            }
        }
    }
}
