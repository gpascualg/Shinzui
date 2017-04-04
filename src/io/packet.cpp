/* Copyright 2016 Guillem Pascual */

#include "io/packet.hpp"
#include "debug/debug.hpp"


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


template <>
float Packet::read()
{
    float f;
    uint32_t u = read<uint32_t>();
    memcpy(&f, &u, sizeof(float));
    return f;
}

template <>
glm::vec2 Packet::read()
{
    return{ read<float>(), read<float>() };
}

template <>
glm::vec3 Packet::read()
{
    return{ read<float>(), read<float>(), read<float>() };
}
