#include "client.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

void Client::start()
{
	std::array<char, 1024> data;
	boost::asio::async_write(*_socket, boost::asio::buffer(data), [this](const boost::system::error_code& error, size_t size) { 
		this->handle_write(&error, size);
	});
}

void Client::handle_write(const boost::system::error_code* error, size_t bytes_transferred)
{
	
}