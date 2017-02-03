#include "packet.hpp"


boost::object_pool<Packet> Packet::_pool(2048);

Packet::Packet() :
    _read(0),
    _size(0),
    _write(0)
{}
