#include "server.hpp"
#include "client.hpp"

#include <boost/asio.hpp>


using tcp = boost::asio::ip::tcp;


Server::Server(uint16_t port)
{
    _service = new boost::asio::io_service;
    _acceptor = new tcp::acceptor(*_service, tcp::endpoint(tcp::v4(), port));
    _socket = new tcp::socket(*_service);
}

Server::~Server()
{
    delete _service;
    delete _acceptor;
    delete _socket;
}

void Server::updateIO()
{
    _service->run();
}

void Server::startAccept()
{
    Client* client = new Client(_service, [this](auto client, const auto error, auto size) {
        this->handleRead(client, error, size);
    });

    _acceptor->async_accept(*client->socket(), [this, client](const auto error) {
        this->handleAccept(client, error);
    });
}

void Server::handleAccept(Client* client, const boost::system::error_code& error)
{
    startAccept();
};
