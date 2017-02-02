/* Copyright 2016 Guillem Pascual */

#pragma once

// Boost forward declarations
namespace boost {
    /*
    namespace posix_time {
        class ptime;
    }
    */

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

        /*
        template <typename Time>
        struct time_traits;

        template <typename TimeType, typename TimeTraits = boost::asio::time_traits<TimeType>>
        class deadline_timer_service;

        template <typename Time, typename TimeTraits, typename TimerService = deadline_timer_service<Time, TimeTraits>>
        class basic_deadline_timer;

        using deadline_timer = basic_deadline_timer<boost::posix_time::ptime>;
        */

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
