#pragma once

#include "defs/common.hpp"

#include <unordered_map>
#include <vector>


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

private:
    static Reactive* _instance;
    TimePoint _lastUpdate;
    float _cpuUsage;
    
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
