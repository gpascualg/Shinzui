/* Copyright 2016 Guillem Pascual */

#pragma once

#include "boost/lockfree/lockfree_forward.hpp"

class Entity;
boost::lockfree::queue<Entity*>* _queue;
