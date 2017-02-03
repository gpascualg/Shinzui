#pragma once

#include <boost/pool/object_pool.hpp>

#include <inttypes.h>
#include <boost/asio.hpp>


class Packet
{
    friend class boost::object_pool<Packet>;

public:
    virtual ~Packet();
    
    static Packet* create()
    {
        auto packet = _pool.construct();
        return packet;
    }

    static Packet* copy(Packet* from, uint16_t size)
    {
        auto packet = _pool.construct();
        memcpy(packet->_buffer, from->_buffer, size);
        return packet;
    }

    template <typename T>
    Packet& operator<<(T v)
    {
        *reinterpret_cast<T*>(_buffer + _write) = v;
        _write += sizeof(T);
        return *this;
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
    inline uint16_t read() { return _read; }
    inline uint16_t written() { return _write; }

    inline uint8_t* data() { return _buffer; }
    inline boost::asio::mutable_buffers_1 sendBuffer() { return boost::asio::buffer(_buffer, _write); };
    inline boost::asio::mutable_buffers_1 recvBuffer(uint16_t len) { return boost::asio::buffer(_buffer + _size, len); };
    inline void addSize(uint16_t offset) { _size += offset; }

    inline void destroy() { _pool.destroy(this); }

private:
    Packet();

private:
    uint8_t _buffer[1024];
    uint16_t _size;
    uint16_t _read;
    uint16_t _write;

    static boost::object_pool<Packet> _pool;
};
