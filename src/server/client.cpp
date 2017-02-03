/* Copyright 2016 Guillem Pascual */

#include "client.hpp"
#include "debug.hpp"
#include "server.hpp"
#include "packet.hpp"

#include <boost/bind.hpp>


using tcp = boost::asio::ip::tcp;

Client::Client(boost::asio::io_service* io_service, ReadFunction readFunction,
    CloseFunction closeFunction) :
    _readFunction(readFunction),
    _closeFunction(closeFunction),
    _socket(*io_service),
    _timer(*io_service)
{
    _packet = Packet::create();

    LOG(LOG_DEBUG, "New client %p", this);
}

Client::~Client()
{
    _packet->destroy();
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
                LOG(LOG_DEBUG, "Closed: %" PRId64, time(NULL));
                close();
            }
            else if (!error)
            {
                ++_readTimes;
                _packet->addSize(size);
                this->_readFunction(this, error, size);
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
                LOG(LOG_DEBUG, "Timeout: %" PRId64, time(NULL));
                close();
            }
        }
    );  // NOLINT(whitespace/parens)
}

void Client::close()
{
    LOG(LOG_DEBUG, "Closed client %" PRId64, id());

    _timer.cancel();
    _socket.cancel();
    _socket.close();
    _closeFunction(this);
}
