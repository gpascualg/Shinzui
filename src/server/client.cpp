/* Copyright 2016 Guillem Pascual */

#include "server/client.hpp"
#include "debug/debug.hpp"
#include "server/server.hpp"
#include "io/packet.hpp"
#include "map/cell.hpp"
#include "map/map.hpp"
#include "map/map_aware_entity.hpp"

#include "defs/common.hpp"

INCL_NOWARN
#include <boost/bind.hpp>
INCL_WARN


using tcp = boost::asio::ip::tcp;

Client::Client(boost::asio::io_service* io_service, uint64_t id) :
    _status(Status::INITIALIZED),
    _socket(*io_service),
    _timer(*io_service)
{
    _packet = Packet::create();
    _entity = Server::get()->newMapAwareEntity(id, this);

    LOG(LOG_CLIENT_LIFECYCLE, "New client %" PRId64, id);
}

Client::~Client()
{
    LOG(LOG_CLIENT_LIFECYCLE, "Deleted client %" PRId64, id());

    _packet->destroy();
    Server::get()->destroyMapAwareEntity(_entity);
}

uint64_t Client::id()
{
    return _entity->id();
}

bool Client::inMap()
{
    return _entity->cell() != nullptr;
}

void Client::scheduleRead(uint16_t bytesToRead, bool reset)
{
    // Cancel timeout
    _timer.cancel();

    // Read reset?
    if (reset)
    {
        _readTimes = 0;
        _packet->reset();
    }

    // Start read
    boost::asio::async_read(_socket, _packet->recvBuffer(bytesToRead),
        [this](const boost::system::error_code& error, size_t size)
        {
            if (error == boost::asio::error::eof)
            {
                LOG(LOG_CLIENT_LIFECYCLE, "Closed: %" PRId64, time(NULL));
                close();
            }
            else if (!error)
            {
                ++_readTimes;
                _packet->addSize((uint16_t)size);
                Server::get()->handleRead(this, error, size);
            }
            // Note: boost::asio::error::operation_aborted when cancel()
        }
    );  // NOLINT(whitespace/parens)

    // Setup timeout!
    _timer.expires_from_now(boost::posix_time::seconds(30));
    _timer.async_wait([this] (const boost::system::error_code& error)
        {
            if (!error)
            {
                LOG(LOG_CLIENT_LIFECYCLE, "Timeout: %" PRId64, time(NULL));
                close();
            }
        }
    );  // NOLINT(whitespace/parens)
}

void Client::send(boost::intrusive_ptr<Packet> packet)
{
    uint16_t opcode = packet->peek<uint16_t>(0);
    LOG(LOG_PACKET_SEND, "Sending %04X", opcode);

    boost::asio::async_write(socket(), packet->sendBuffer(),
        [packet](const boost::system::error_code & error, std::size_t size)
        {
            LOG(LOG_PACKET_SEND, "\tPacket %04X sent!", packet->peek<uint16_t>(0));
        }
    );  // NOLINT(whitespace/parens)
}

void Client::close()
{
    if (_status != Status::CLOSED)
    {
        LOG(LOG_CLIENT_LIFECYCLE, "Closed client %" PRId64, id());

        // Cancel all IO
        _timer.cancel();
        _socket.close();

        // Remove from map
        auto cell = entity()->cell();
        if (cell)
        {
            cell->map()->removeFrom(cell, entity(), nullptr);
        }

        // Flag me
        _status = Status::CLOSED;

        // Recycle client
        Server::get()->handleClose(this);
    }
}
