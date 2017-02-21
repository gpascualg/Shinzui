/* Copyright 2016 Guillem Pascual */

#include "packet.hpp"
#include "debug.hpp"


boost::object_pool<Packet> Packet::_pool(2048);

Packet::Packet() :
    _read(0),
    _size(0),
    _write(0),
    _refs(0)
{}

Packet::~Packet()
{
    LOG(LOG_PACKET_LIFECYCLE, "Packet destroyed %.4X", *(uint16_t*)_buffer);
}
