#pragma once

#include "mocks/entity.hpp"

#include <server/server.hpp>
#include <server/client.hpp>


class TestServer : public Server
{
public:
    using Server::Server;

    void handleAccept(Client* client, const boost::system::error_code& error) override
    {

    }

    void handleRead(Client* client, const boost::system::error_code& error, size_t size) override
    {

    }

    Client* newClient(boost::asio::io_service* service, uint64_t id) override
    {
        return new Client(service, id);
    }
    void destroyClient(Client* client) override
    {
        delete client;
    }

    MapAwareEntity* newMapAwareEntity(uint64_t id, Client* client) override
    {
        return new Entity(id, client);
    }
    void destroyMapAwareEntity(MapAwareEntity* entity) override
    {
        delete entity;
    }
};
