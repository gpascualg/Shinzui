/* Copyright 2016 Guillem Pascual */

// FIXME: This is a placeholder to have some .cpp files around :/

#include "defs/common.hpp"
INCL_NOWARN
#include <boost/lockfree/queue.hpp>
#include <boost/asio.hpp>
INCL_WARN

volatile int _ = 0;

boost::asio::io_service io_service;
