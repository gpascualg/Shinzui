#include <catch.hpp>
#include "entity.hpp"

#include <offset.hpp>
#include <stdlib.h>
#include <unordered_map>


SCENARIO("Offset can be created and manipulated", "[offset]") {
    GIVEN("All possible hex coordinates") {
        WHEN("offsets are built") {
            THEN("all hashes are unique") {
                std::unordered_map<uint64_t, uint8_t> hashes;

                // Not whole range
                for (int16_t y = -326; y < 327; ++y)
                {
                    for (int16_t x = -326; x < 327; ++x)
                    {
                        const Offset offset(x, y);
                        if (hashes.find(offset.hash()) != hashes.end())
                        {
                            INFO("Failed for " << offset.hash());
                            REQUIRE(false);
                        }

                        hashes[offset.hash()] = 0;
                    }
                }

                hashes.clear();
                REQUIRE(true);
            }
        }
    }

    GIVEN("Some random 2D coordinates") {
        int32_t x = (int32_t)(rand() - RAND_MAX / 2.0);
        int32_t y = (int32_t)(rand() - RAND_MAX / 2.0);

        WHEN("the offset is created") {
            const Offset offset = offsetOf(x, y);

            THEN("hex coordinates are coherent") {
                REQUIRE(offset.q() + offset.r() + offset.s() == 0);
            }

            THEN("hash is build from q and r") {
                REQUIRE((int32_t)(offset.hash() >> 32) == offset.q());
                REQUIRE((int32_t)(offset.hash() & 0xFFFFFFFF) == offset.r());
            }
        }
    }

    GIVEN("Two offsets at distance 1") {
        const Offset offset1(0, 0);
        const Offset offset2(0, 1);

        WHEN("the radius is computed") {
            THEN("the result is 1") {
                REQUIRE(offset1.distance(offset2) == 1);
            }
        }
    }
}
