#include "debug/reactive.hpp"
#include "debug/cpu.hpp"
#include "debug/debug.hpp"
#include "server/client.hpp"
#include "server/server.hpp"
#include "map/map.hpp"
#include "map/cell.hpp"
#include "map/map_aware_entity.hpp"
#include "map/map-cluster/cluster.hpp"

INCL_NOWARN
#include <rxterm/terminal.hpp>
#include <rxterm/style.hpp>
#include <rxterm/image.hpp>
#include <rxterm/reflow.hpp>
#include <rxterm/components/text.hpp>
#include <rxterm/components/stacklayout.hpp>
#include <rxterm/components/flowlayout.hpp>
#include <rxterm/components/progress.hpp>
#include <rxterm/components/maxwidth.hpp>
INCL_WARN

#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <memory>

using namespace rxterm;


Reactive* Reactive::_instance = nullptr;
constexpr const float alpha = 0.1f;
// Otherwise there are double-includes
static VirtualTerminal _vt; 

Reactive::Reactive():
    LogLevel(LOG_LEVEL),
    LogHandlers(LOG_HANDLERS),
    _cpuUsage(0)
{
    initCPUDebugger();
    _lastUpdate = Server::get()->now();
}

Reactive::~Reactive()
{}

auto renderToTerm = [](auto const& vt, unsigned const w, Component const& c) {
  // TODO: get actual terminal width
  return vt.flip(c.render(w).toString());
};

void Reactive::onPacketCreated() 
{ 
    ++_numAlivePackets; 
}

void Reactive::onPacketWritten(uint16_t opcode) 
{
    IF_LOG(LOG_LEVEL_DEBUG, LOG_PACKET_LIFECYCLE)
    {
        ++_packetCount[opcode];
    }
}

void Reactive::onPacketDestroyed(uint16_t opcode) 
{
    --_numAlivePackets; 
    
    IF_LOG(LOG_LEVEL_DEBUG, LOG_PACKET_LIFECYCLE)
    {
        if (opcode)
        {
            --_packetCount[opcode];
        }
    }
}

void Reactive::update(TimeBase heartBeat, TimeBase diff, TimeBase prevSleep)
{
    if (std::chrono::duration_cast<TimeBase>(Server::get()->now() - _lastUpdate) < std::chrono::duration_cast<TimeBase>(std::chrono::seconds(1)))
    {
        return;
    }
    
    _lastUpdate = Server::get()->now();
    update_impl(heartBeat, diff, prevSleep);
}

void Reactive::update_impl(TimeBase heartBeat, TimeBase diff, TimeBase prevSleep)
{
#if !defined(FORCE_ASCII_DEBUG)
    using namespace std::chrono_literals;
    using namespace std::string_literals;

    auto diffNum = std::to_string(std::min(diff.count(), TimeScale));
    auto diffStr = std::string(4 - diffNum.length(), ' ').append(diffNum);

    auto sleepNum = std::to_string(prevSleep.count());
    auto sleepStr = std::string(4 - sleepNum.length(), ' ').append(sleepNum);
    
    float progress = (heartBeat - prevSleep).count() / float(heartBeat.count());
    if (diff > heartBeat)
    {
        progress = std::min(1.0f, (diff - prevSleep).count() / float(heartBeat.count()));
    }

    _cpuUsage = (alpha * getCurrentCPUUsage()) + (1.0 - alpha) * _cpuUsage;
    std::string ramUsage = std::to_string(getCurrentRAMUsage() / 1024) + "MB";

    auto component =
        StackLayout<>{
            FlowLayout<>{
                Text(Style::Default(), "AuraServer debug information [CPU "),
                Text(Style{Color::None, FontColor::Red}, int(_cpuUsage)),
                Text(Style::Default(), ", RAM "),
                Text(Style{Color::None, FontColor::Red}, ramUsage),
                Text(Style::Default(), "]")
            },
            
            FlowLayout<>{
                Text(Style::Default(), "Diff: "),
                Text(Style::Default(), diffStr),
                Text(Style::Default(), "  "),
                MaxWidth(heartBeat.count(), Progress(progress, Pixel{' ', {Color::Cyan}}, Pixel{' ', {Color::Red}})),
                Text(Style::Default(), "  "),
                Text(Style::Default(), "Sleep: "),
                Text(Style::Default(), sleepStr)
            },

            Text(Style::Default(), "Messages:")
        };

    _messagesLock.lock();
    for (auto msg : _messages)
    {
        component.children.emplace_back(Text(Style::Default(), msg));
    }
    _messagesLock.unlock();

    component.children.emplace_back(
        StackLayout<>{
            Text(Style::Default(), " "),
            FlowLayout<>{
                Text(Style::Default(), "Alive packets: "),
                Text(Style::Default(), _numAlivePackets)
            }
        });

    IF_LOG(LOG_LEVEL_DEBUG, LOG_PACKET_LIFECYCLE)
    {
        for (auto pair : _packetCount)
        {
            if (pair.second > 0)
            {
                std::stringstream opcode;
                opcode << std::setfill('0') << std::setw(4) << std::hex << pair.first;

                component.children.emplace_back(
                    FlowLayout<>{
                        Text(Style::Default(), "    ["),
                        Text(Style::Default(), opcode.str()),
                        Text(Style::Default(), "]: "),
                        Text(Style::Default(), pair.second)
                    });
            }
        }
    }

    component.children.emplace_back(
            FlowLayout<>{
                Text(Style::Default(), "Clients (accepting/playing/closing): "),
                Text(Style::Default(), _pendingAcceptClient),
                Text(Style::Default(), "/"),
                Text(Style::Default(), _numClients),
                Text(Style::Default(), "/"),
                Text(Style::Default(), _pendingCloseClient)
            });

    IF_LOG(LOG_LEVEL_DEBUG, LOG_CLIENT_LIFECYCLE)
    {
        Server::get()->iterateClients([&component](Client* client) {
            if (client->entity() && client->entity()->cell())
            {
                component.children.emplace_back(
                    FlowLayout<>{
                        Text(Style::Default(), "    ["),
                        Text(Style::Default(), client->entity()->cell()->offset().q()),
                        Text(Style::Default(), ","),
                        Text(Style::Default(), client->entity()->cell()->offset().r()),
                        Text(Style::Default(), "]: "),
                        Text(Style::Default(), client->id())
                    });
            }
        });
    }

    component.children.emplace_back(
            FlowLayout<>{
                Text(Style::Default(), "Map (clusters/cells/stall [candidates]): "),
                Text(Style::Default(), _numClusters),
                Text(Style::Default(), "/"),
                Text(Style::Default(), _numCells),
                Text(Style::Default(), "/"),
                Text(Style::Default(), _numStall),
                Text(Style::Default(), " ["),
                Text(Style::Default(), _numStallCandidates),
                Text(Style::Default(), "]")
            });

    IF_LOG(LOG_LEVEL_DEBUG, LOG_CLUSTERS)
    {
        uint8_t count = 0;
        for (const auto& cell : Server::get()->map()->cluster()->cells())
        {
            if (++count > 10)
            {
                break;
            }

            // Do not display non-stall&non-cooldown cells
            if (!cell->stall.isRegistered)
            {
                continue;
            }

            component.children.emplace_back(
                FlowLayout<>{
                    Text(Style::Default(), "    ["),
                    Text(Style::Default(), cell->offset().q()),
                    Text(Style::Default(), ","),
                    Text(Style::Default(), cell->offset().r()),
                    Text(Style::Default(), "]: "),
                    Text(Style::Default(), std::chrono::duration_cast<std::chrono::seconds>(TimeBase(cell->stall.remaining)).count()),
                    Text(Style::Default(), " ("),
                    Text(Style::Default(), cell->stall.isOnCooldown),
                    Text(Style::Default(), ")")
                });
        }
    }

    _vt = renderToTerm(_vt, 200, component);
#endif
}
