/* Copyright 2016 Guillem Pascual */

#include "io/packet.hpp"
#include "debug/debug.hpp"
#include "debug/reactive.hpp"


Packet::Packet() :
    _read(0),
    _size(0),
    _write(0),
    _refs(0)
{
    Reactive::get()->onPacketCreated();
}

Packet::~Packet()
{
    if (bufferLen() >= 2)
    {
        uint16_t opcode = peek<uint16_t>(0);
        LOG(LOG_PACKET_LIFECYCLE, "Packet destroyed %.4X", opcode);
        Reactive::get()->onPacketDestroyed(opcode);
    }
    else
    {
        LOG(LOG_PACKET_LIFECYCLE, "Packet destroyed [EMPTY]");
        Reactive::get()->onPacketDestroyed(0x0);
    }
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

template <>
std::string Packet::read()
{
    uint16_t length = read<uint16_t>();
    char* str = static_cast<char*>(read(length));
    return std::string(str, (std::size_t)length);
}
