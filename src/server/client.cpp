/* Copyright 2016 Guillem Pascual */

#include "client.hpp"
#include "debug.hpp"
#include "server.hpp"

#include <boost/bind.hpp>


using tcp = boost::asio::ip::tcp;

Client::Client(boost::asio::io_service* io_service, ReadFunction readFunction,
    CloseFunction closeFunction) :
    _readFunction(readFunction),
    _closeFunction(closeFunction),
    _socket(*io_service),
    _timer(*io_service, boost::posix_time::seconds(2))
{
    _data = new char[1024];
}

Client::~Client()
{
    delete[] _data;
}

void Client::scheduleRead(uint16_t bytesToRead, bool reset)
{
    // Cancel timeout
    _timer.cancel();

    // Read reset?
    if (reset)
    {
        _readTimes = 0;
        _totalRead = 0;
    }

    // Start read
    boost::asio::async_read(_socket, boost::asio::buffer(_data + _totalRead, bytesToRead),
        [this](const boost::system::error_code& error, size_t size)
        {
            if (!error)
            {
                ++_readTimes;
                _totalRead += size;
            }

            this->_readFunction(this, error, size);
        }
    );  // NOLINT(whitespace/parens)

    // Setup timeout!
    _timer.async_wait([this] (const boost::system::error_code& error)
        {
            if (!error)
            {
                close();
            }
        });  // NOLINT(whitespace/braces)
}

void Client::close()
{
    LOG(LOG_DEBUG, "Closed client %d", id());

    _timer.cancel();
    _socket.close();
    _closeFunction(this);
}
