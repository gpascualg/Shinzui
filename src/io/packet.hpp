/* Copyright 2016 Guillem Pascual */

#pragma once

#include <inttypes.h>
#include <algorithm>

#include "defs/common.hpp"

INCL_NOWARN
#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/intrusive_ptr.hpp>
#include <glm/glm.hpp>
INCL_WARN


union f2u
{
    float f;
    uint32_t u;
};

class Packet
{
    friend class boost::object_pool<Packet>;
    friend inline void intrusive_ptr_add_ref(Packet* x);
    friend inline void intrusive_ptr_release(Packet* x);

public:
    virtual ~Packet();

    static Packet* create()
    {
        auto packet = _pool.construct();
        return packet;
    }

    static Packet* create(uint16_t opcode)
    {
        auto packet = _pool.construct();
        *packet << opcode;
        *packet << uint16_t{ 0x0000 };
        return packet;
    }

    static Packet* copy(Packet* from, uint16_t size)
    {
        auto packet = _pool.construct();
        memcpy(packet->_buffer, from->_buffer, size);
        packet->_size = size;
        return packet;
    }

    template <typename T>
    Packet& operator<<(T v)
    {
        *reinterpret_cast<T*>(_buffer + _write) = v;
        _write += sizeof(T);
        return *this;
    }

    Packet& operator<<(float v)
    {
        *reinterpret_cast<uint32_t*>(_buffer + _write) = f2u{ v }.u;  // NOLINT(whitespace/braces)
        _write += sizeof(uint32_t);
        return *this;
    }

    Packet& operator<<(const glm::vec2& v)
    {
        *this << v.x;
        *this << v.y;
        return *this;
    }

    Packet& operator<<(const glm::vec3& v)
    {
        *this << v.x;
        *this << v.y;
        *this << v.z;
        return *this;
    }

    Packet& operator<<(Packet* packet)
    {
        uint16_t end = (uint16_t)std::max({ packet->totalRead(),   // NOLINT(whitespace/braces)
                                            packet->size(),
                                            packet->written() });  // NOLINT(whitespace/braces)

        for (uint16_t i = 4; i < end; ++i)
        {
            *this << packet->data()[i];
        }

        return *this;
    }

    Packet& operator<<(boost::intrusive_ptr<Packet> packet)
    {
        return *this << packet.get();
    }

    template <typename T>
    T peek(uint16_t offset)
    {
        return *reinterpret_cast<T*>(_buffer + offset);
    }

    template <typename T>
    T read()
    {
        T v = peek<T>(_read);
        _read += sizeof(T);
        return v;
    }

    inline void reset() { _read = _write = _size = 0; }

    inline uint16_t size() { return _size; }
    inline uint16_t totalRead() { return _read; }
    inline uint16_t written() { return _write; }

    inline uint8_t* data() { return _buffer; }
    inline boost::asio::mutable_buffers_1 sendBuffer()
    {
        *reinterpret_cast<uint16_t*>(_buffer + sizeof(uint16_t)) = _write - sizeof(uint16_t) * 2;
        return boost::asio::buffer(_buffer, _write);
    }
    inline boost::asio::mutable_buffers_1 recvBuffer(uint16_t len) { return boost::asio::buffer(_buffer + _size, len); }
    inline void addSize(uint16_t offset) { _size += offset; }

    // TODO(gpascualg): Is object pool threadsafe?
    inline void destroy() { _pool.destroy(this); }

private:
    Packet();

private:
    uint16_t _read;
    uint16_t _size;
    uint16_t _write;
    uint16_t _refs;
    uint8_t _buffer[1024];

    static boost::object_pool<Packet> _pool;
};

template <> float Packet::read();
template <> glm::vec2 Packet::read();
template <> glm::vec3 Packet::read();
