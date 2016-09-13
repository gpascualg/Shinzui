/* Copyright 2016 Guillem Pascual */

#pragma once

// Boost forward declarations
namespace boost {
	namespace asio {
		class io_service;


		template <typename Protocol>
		class stream_socket_service;

		template <typename Protocol, typename StreamSocketService>
		class basic_stream_socket;


		template <typename Protocol>
		class socket_acceptor_service;

		template<typename Protocol, typename SocketAcceptorService>
		class basic_socket_acceptor;


		namespace ip {
			class tcp;
		}

        // boost::asio::ip::tcp::socket
        using SocketForward = boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::stream_socket_service<boost::asio::ip::tcp>>;
		// boost::asio::ip::tcp::acceptor
		using AcceptorForward = boost::asio::basic_socket_acceptor<boost::asio::ip::tcp, boost::asio::socket_acceptor_service<boost::asio::ip::tcp>>;
	}

	namespace system {
		class error_code;
	}
}
