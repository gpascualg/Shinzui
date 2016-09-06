#pragma once

#include <inttypes.h>
#include <algorithm>
#include <array>
#include <vector>

#include "common.hpp"
#include "debug.hpp"


class Cluster;
class Cell;

struct ClusterCenter
{
    Cell* center;
    bool mergeSignaled = false;
};
