#pragma once

// Boost forward declarations
namespace boost { 
	namespace asio {
		template <typename Protocol>
		class stream_socket_service;

		template <typename Protocol, typename StreamSocketService>
		class basic_stream_socket;

		namespace ip {
			class tcp;
		}
	}
	
	namespace system {
		class error_code;
	}
}


class Client
{
public:
	void start();

	void handle_write(const boost::system::error_code* error, size_t bytes_transferred);

private:
	// boost::asio::ip::tcp::socket = ...
	boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::stream_socket_service<boost::asio::ip::tcp> >* _socket;
};
