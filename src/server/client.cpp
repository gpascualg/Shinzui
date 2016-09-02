#include "client.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

using tcp = boost::asio::ip::tcp;

Client::Client(boost::asio::io_service* io_service, ReadFunction readFunction) :
	_readFunction(readFunction)
{
	_socket = new tcp::socket(*io_service);
	_data = new char[1024];
}

Client::~Client()
{
	delete[] _data;
}

void Client::scheduleRead(uint16_t bytesToRead, bool reset)
{
	if (reset)
	{
		_readTimes = 0;
		_totalRead = 0;
	}

	boost::asio::async_read(*_socket, boost::asio::buffer(_data + _totalRead, bytesToRead),
		[this](const boost::system::error_code& error, size_t size) {
			if (!error)
			{
				++_readTimes;
				_totalRead += size;
			}

			this->_readFunction(this, &error, size);
		}
	);
}
