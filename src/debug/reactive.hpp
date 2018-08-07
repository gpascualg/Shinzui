#pragma once

#include "defs/common.hpp"

#include <unordered_map>

namespace rxterm
{
    class VirtualTerminal;
}


class Reactive
{
public:
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

    inline void onClusterUpdate(uint16_t numClusters, uint16_t numCells)
    {
        _numClusters = numClusters;
        _numCells = numCells;
    }

private:
    Reactive();

private:
    static Reactive* _instance;
    rxterm::VirtualTerminal* _vt;
    
    uint16_t _numAlivePackets;
    std::unordered_map<uint16_t, uint16_t> _packetCount;
    
    uint16_t _pendingAcceptClient;
    uint16_t _pendingCloseClient;
    uint16_t _numClients;
    
    uint16_t _numClusters;
    uint16_t _numCells;
};
