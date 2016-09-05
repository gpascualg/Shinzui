#include <catch.hpp>
#include "entity.hpp"

#include <cell.hpp>
#include <cluster.hpp>
#include <map.hpp>


SCENARIO("Map cells can be fetched once created", "[map]") {
    GIVEN("A map with one cell and its siblings") {
        Map map(0, 0, 200, 50);
        map.addTo2D(0, 0, new Entity());
        map.runScheduledOperations();

        REQUIRE(map.size() == 7);
        REQUIRE(map.scheduledSize() == 0);

        WHEN("the main cell is required") {
            Cell* cell = map.get(0, 0);

            THEN("cell is not nullptr") {
                REQUIRE(cell != nullptr);
            }

            THEN("cell has all 6 siblings") {
                auto upper = cell->upperHalfSiblings(1);
                auto lower = cell->lowerHalfSiblings(1);

                for (auto i : {0, 1, 2})
                {
                    REQUIRE(upper[i] != nullptr);
                }

                for (auto i : {0, 1, 2})
                {
                    REQUIRE(lower[i] != nullptr);
                }
            }

            THEN("cell has a ring with radius 1") {
                auto ring = cell->ring(1);

                for (auto i : {0, 1, 2, 3, 4, 5, 6})
                {
                    REQUIRE(ring[i] != nullptr);
                }
            }

            THEN("cell has not a ring with radius 2") {
                auto ring = cell->ring(2);

                for (auto i : {0, 1, 2, 3, 4, 5, 6})
                {
                    REQUIRE(ring[i] == nullptr);
                }
            }
        }
    }
}
