#pragma once

#include "defs/common.hpp"

#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>


namespace rxterm
{
    class VirtualTerminal;
}

class Client;

class Reactive
{
public:
    ~Reactive();
    
    static Reactive* get()
    {
        if (!_instance)
        {
            _instance = new Reactive();
        }

        return _instance;
    }

    void update(TimeBase heartBeat, TimeBase diff, TimeBase prevSleep);

    template <typename... T>
    bool print(const char* fmt, T... params)
    {
        size_t sz = snprintf(NULL, 0, fmt, params...);
        char* buf = (char *)malloc(sz + 1);
        if (buf)
        {
            std::lock_guard<std::mutex> lock(_messagesLock);

            snprintf(buf, sz+1, fmt, params...);
            _messages.emplace_back(std::string{buf, sz});
            free(buf);

            if (_messages.size() > 15)
            {
                _messages.erase(_messages.begin());
            }
        }

        return buf != nullptr;
    }
    
    template <typename... T>
    bool print_now(const char* fmt, T... params)
    {
        if (print(fmt, params...))
        {
            update_impl(TimeBase(0), TimeBase(0), TimeBase(0));
            return true;
        }

        return false;
    }

    void onPacketCreated();
    void onPacketWritten(uint16_t opcode);
    void onPacketDestroyed(uint16_t opcode);

    inline void onClientCreated() { ++_pendingAcceptClient; }
    inline void onClientAccepted() { ++_numClients; --_pendingAcceptClient; }
    inline void onClientClosed() { ++_pendingCloseClient; }
    inline void onClientDestroyed() { --_pendingCloseClient; --_numClients;}

    inline void onClusterUpdate(uint16_t numClusters, uint16_t numCells, uint16_t numStall, uint16_t numStallCandidates)
    {
        _numClusters = numClusters;
        _numCells = numCells;
        _numStall = numStall;
        _numStallCandidates = numStallCandidates;
    }

    std::vector<Client*> clients;

private:
    Reactive();
    void update_impl(TimeBase heartBeat, TimeBase diff, TimeBase prevSleep);

public:
    uint32_t LogLevel;
    uint32_t LogHandlers;

private:
    static Reactive* _instance;
    TimePoint _lastUpdate;
    float _cpuUsage;

    std::mutex _messagesLock;
    std::vector<std::string> _messages;
    
    uint16_t _numAlivePackets;
    std::unordered_map<uint16_t, uint16_t> _packetCount;
    
    uint16_t _pendingAcceptClient;
    uint16_t _pendingCloseClient;
    uint16_t _numClients;
    
    uint16_t _numClusters;
    uint16_t _numCells;
    uint16_t _numStall;
    uint16_t _numStallCandidates;
};
