/* Copyright 2016 Guillem Pascual */

#include "packet.hpp"


boost::object_pool<Packet> Packet::_pool(2048);

Packet::Packet() :
    _read(0),
    _size(0),
    _write(0)
{}

Packet::~Packet()
{
    reset();
}
