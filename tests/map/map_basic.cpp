#include <catch.hpp>
#include "entity.hpp"

#include <cell.hpp>
#include <cluster.hpp>
#include <map.hpp>


SCENARIO("Map cells can be created and eliminated", "[map]") {
    GIVEN("An empty map") {
        Map map;

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
        Map map;
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

            THEN("1 non-updating cell has been created") {
                REQUIRE(map.size() == 1);
            }

            THEN("entity is flagged as added") {
                REQUIRE(e.hasBeenAdded);
            }
        }
    }

    GIVEN("A map with one cell") {
        Map map;
        map.addTo2D(0, 0, new Entity());
        map.runScheduledOperations();

        REQUIRE(map.size() == 1);
        REQUIRE(map.scheduledSize() == 0);

        WHEN("the cluster is ran") {
            map.cluster()->update(0);

            THEN("cluster has size 0, non-updating entity!") {
                REQUIRE(map.cluster()->size() == 0);
            }
        }
    }

    GIVEN("A map with two cells and one pending") {
        Map map;
        map.addTo(0, 0, new Entity());
        map.addTo(4, 0, new Entity());
        map.runScheduledOperations();

        // TODO: Make its own test
        map.cluster()->update(0);
        map.cluster()->runScheduledOperations();

        map.addTo(2, 0, new Entity());

        REQUIRE(map.size() == 2);
        REQUIRE(map.scheduledSize() == 1);
        REQUIRE(map.cluster()->size() == 0);

        WHEN("scheduled operations are ran") {
            map.runScheduledOperations();
            map.cluster()->update(0);
            map.cluster()->runScheduledOperations();

            THEN("no clusters should remain") {
                REQUIRE(map.cluster()->size() == 0);
            }
        }
    }

    GIVEN("A map with two cells and one pending which should merge") {
        Map map;
        map.addTo(0, 0, new Entity((Client*)1));
        map.addTo(3, 0, new Entity((Client*)1));
        map.runScheduledOperations();

        // TODO: Make its own test
        map.cluster()->update(0);
        map.cluster()->runScheduledOperations();

        map.addTo(1, 0, new Entity((Client*)1));
        map.addTo(1, 0, new Entity());

        REQUIRE(map.size() == 14);
        REQUIRE(map.scheduledSize() == 2);
        REQUIRE(map.cluster()->size() == 2);

        WHEN("scheduled operations are ran") {
            map.runScheduledOperations();
            map.cluster()->update(0);
            map.cluster()->runScheduledOperations();

            THEN("only one cluster should remain") {
                REQUIRE(map.cluster()->size() == 1);
            }
        }
    }

    GIVEN("A map with entities") {
        Map map;
        auto e1 = new Entity((Client*)1);
        auto e2 = new Entity();
        e2->position().x = 3;
        map.addTo(e1);
        map.addTo(e2);
        map.runScheduledOperations();

        map.cluster()->update(0);
        map.cluster()->runScheduledOperations();
        
        REQUIRE(map.size() == 7);
        REQUIRE(map.scheduledSize() == 0);
        REQUIRE(map.cluster()->size() == 1);

        WHEN("one entity is removed") {
            map.removeFrom(e1);

            map.runScheduledOperations();
            map.cluster()->update(0);
            map.cluster()->runScheduledOperations();

            THEN("only one entity should remain") {
                REQUIRE(e1->cell() == nullptr);
                REQUIRE(e2->cell() != nullptr);
            }
        }

        WHEN("one entity is removed") {
            map.removeFrom(e1);
            map.removeFrom(e2);

            map.runScheduledOperations();
            map.cluster()->update(0);
            map.cluster()->runScheduledOperations();

            THEN("only one entity should remain") {
                REQUIRE(e1->cell() == nullptr);
                REQUIRE(e2->cell() == nullptr);
            }
        }
    }

    GIVEN("A map with entities") {
        Map map;
        auto e1 = new Entity((Client*)1);
        auto e2 = new Entity((Client*)1);
        e2->position().x = 50;

        map.addTo(e1);
        map.addTo(e2);
        map.runScheduledOperations();

        map.cluster()->update(0);
        map.cluster()->runScheduledOperations();

        REQUIRE(map.size() == 14);
        REQUIRE(map.scheduledSize() == 0);
        REQUIRE(map.cluster()->size() == 2);

        WHEN("one entity is moved") {
            e1->position().x = 50;
            map.onMove(e1);

            map.runScheduledOperations();
            map.cluster()->update(0);
            map.cluster()->runScheduledOperations();

            THEN("both entities should have the same cell") {
                REQUIRE(e1->cell() == e2->cell());
            }

            THEN("two clusters should remain") {
                REQUIRE(map.cluster()->size() == 2);
                // Cluster compaction would reduce it to 1
            }
        }
    }
}
