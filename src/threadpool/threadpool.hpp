/* Copyright 2016 Guillem Pascual */

#pragma once

#include "defs/common.hpp"
INCL_NOWARN
#include "boost/lockfree/lockfree_forward.hpp"
INCL_WARN

class Entity;
boost::lockfree::queue<Entity*>* _queue;
