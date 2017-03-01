#include <catch.hpp>
#include "entity.hpp"

#include <cell.hpp>
#include <cluster.hpp>
#include <map.hpp>
#include <motion_master.hpp>
#include <server.hpp>


SCENARIO("Map cells can be created and eliminated", "[map]") {
    GIVEN("An empty map") {
        TestServer server(12345);
        Map& map = *server.map();

        REQUIRE(map.size() == 0);
        // REQUIRE(map.scheduledSize() == 0);

        WHEN("one entity is added to the map") {
            Entity e(0);
            map.addTo2D({ 0, 0 }, &e, nullptr);

            THEN("the number of cells remains constant but scheduled operations increase") {
                REQUIRE(map.size() == 0);
                // REQUIRE(map.scheduledSize() == 1);
            }

            THEN("entity is not flagged as added") {
                REQUIRE(!e.hasBeenAdded);
            }
        }

        WHEN("one entity is added to the map using hex coordinates") {
            Entity e(0);
            map.addTo(0, 0, &e, nullptr);

            THEN("the number of cells remains constant but scheduled operations increase") {
                REQUIRE(map.size() == 0);
                // REQUIRE(map.scheduledSize() == 1);
            }

            THEN("entity is not flagged as added") {
                REQUIRE(!e.hasBeenAdded);
            }
        }
    }

    GIVEN("A map with one add operation pending") {
        TestServer server(12345);
        Map& map = *server.map();

        Entity e(0);
        map.addTo2D({ 0, 0 }, &e, nullptr);

        REQUIRE(map.size() == 0);
        // REQUIRE(map.scheduledSize() == 1);

        WHEN("operations are ran") {
            map.runScheduledOperations();

            THEN("the numbers of cells increase and pending operations are 0") {
                REQUIRE(map.size() > 0);
                // REQUIRE(map.scheduledSize() == 0);
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
        TestServer server(12345);
        Map& map = *server.map();

        map.addTo2D({ 0, 0 }, new Entity(0), nullptr);
        map.runScheduledOperations();

        REQUIRE(map.size() == 1);
        // REQUIRE(map.scheduledSize() == 0);

        WHEN("the cluster is ran") {
            map.cluster()->update(0);

            THEN("cluster has size 0, non-updating entity!") {
                REQUIRE(map.cluster()->size() == 0);
            }
        }
    }

    GIVEN("A map with two cells and one pending") {
        TestServer server(12345);
        Map& map = *server.map();

        map.addTo(0, 0, new Entity(0), nullptr);
        map.addTo(4, 0, new Entity(1), nullptr);
        map.runScheduledOperations();

        // TODO: Make its own test
        map.cluster()->update(0);
        map.cluster()->runScheduledOperations();

        map.addTo(2, 0, new Entity(2), nullptr);

        REQUIRE(map.size() == 2);
        // REQUIRE(map.scheduledSize() == 1);
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
        TestServer server(12345);
        Map& map = *server.map();

        map.addTo(0, 0, new Entity(0, (Client*)1), nullptr);
        map.addTo(3, 0, new Entity(1, (Client*)1), nullptr);
        map.runScheduledOperations();

        // TODO: Make its own test
        map.cluster()->update(0);
        map.cluster()->runScheduledOperations();

        map.addTo(1, 0, new Entity(0, (Client*)1), nullptr);
        map.addTo(1, 0, new Entity(1), nullptr);

        REQUIRE(map.size() == 14);
        // REQUIRE(map.scheduledSize() == 2);
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
        TestServer server(12345);
        Map& map = *server.map();

        auto e1 = new Entity(0, (Client*)1);
        auto e2 = new Entity(1);
        e2->motionMaster()->teleport({ 3,0 });
        map.addTo(e1, nullptr);
        map.addTo(e2, nullptr);
        map.runScheduledOperations();

        map.cluster()->update(0);
        map.cluster()->runScheduledOperations();

        REQUIRE(map.size() == 7);
        // REQUIRE(map.scheduledSize() == 0);
        REQUIRE(map.cluster()->size() == 1);

        WHEN("one entity is removed") {
            map.removeFrom(e1, nullptr);

            map.runScheduledOperations();
            map.cluster()->update(0);
            map.cluster()->runScheduledOperations();

            THEN("only one entity should remain") {
                REQUIRE(e1->cell() == nullptr);
                REQUIRE(e2->cell() != nullptr);
            }
        }

        WHEN("one entity is removed") {
            map.removeFrom(e1, nullptr);
            map.removeFrom(e2, nullptr);

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
        TestServer server(12345);
        Map& map = *server.map();

        auto e1 = new Entity(0, (Client*)1);
        auto e2 = new Entity(1, (Client*)1);
        e2->motionMaster()->teleport({ 50, 0 });

        map.addTo(e1, nullptr);
        map.addTo(e2, nullptr);
        map.runScheduledOperations();

        map.cluster()->update(0);
        map.cluster()->runScheduledOperations();

        REQUIRE(map.size() == 14);
        // REQUIRE(map.scheduledSize() == 0);
        REQUIRE(map.cluster()->size() == 2);

        WHEN("one entity is moved") {
            e1->motionMaster()->teleport({ 50, 0 });
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
