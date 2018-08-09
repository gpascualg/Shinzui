#include "debug/reactive.hpp"
#include "debug/cpu.hpp"
#include "debug/debug.hpp"
#include "server/client.hpp"
#include "server/server.hpp"
#include "map/map.hpp"
#include "map/cell.hpp"
#include "map/map_aware_entity.hpp"
#include "map/map-cluster/cluster.hpp"

#include <rxterm/terminal.hpp>
#include <rxterm/style.hpp>
#include <rxterm/image.hpp>
#include <rxterm/reflow.hpp>
#include <rxterm/components/text.hpp>
#include <rxterm/components/stacklayout.hpp>
#include <rxterm/components/flowlayout.hpp>
#include <rxterm/components/progress.hpp>
#include <rxterm/components/maxwidth.hpp>

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
    ++_packetCount[opcode];
}

void Reactive::onPacketDestroyed(uint16_t opcode) 
{
    --_numAlivePackets; 
    if (opcode)
    {
        --_packetCount[opcode];
    }
}

void Reactive::update(TimeBase heartBeat, TimeBase diff, TimeBase prevSleep)
{
    if (std::chrono::duration_cast<TimeBase>(Server::get()->now() - _lastUpdate) < std::chrono::duration_cast<TimeBase>(std::chrono::seconds(1)))
    {
        return;
    }
    
    _lastUpdate = Server::get()->now();

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

    auto component =
        StackLayout<>{
            FlowLayout<>{
                Text(Style::Default(), "AuraServer debug information [CPU "),
                Text(Style{Color::None, FontColor::Red}, int(_cpuUsage)),
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

            FlowLayout<>{
                Text(Style::Default(), "Alive packets: "),
                Text(Style::Default(), _numAlivePackets)
            }
        };

    for (auto pair : _packetCount)
    {
        if (pair.second > 0)
        {
            std::stringstream stream;
            stream << std::setfill('0') << std::setw(4) << std::hex << pair.first;

            component.children.emplace_back(
                FlowLayout<>{
                    Text(Style::Default(), "    ["),
                    Text(Style::Default(), stream.str()),
                    Text(Style::Default(), "]: "),
                    Text(Style::Default(), pair.second)
                });
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

    for (Client* client : clients)
    {
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

    for (const auto& cell : Server::get()->map()->cluster()->cells())
    {
        auto style = Style::Default();
        if (cell->stall.isOnCooldown)
        {
            style = Style{ Color::None, FontColor::Yellow };
        }
        else if (!cell->stall.isRegistered)
        {
            // Do not display non-stall&non-cooldown cells
            continue;
            // style = Style{ Color::None, FontColor::Red };
        }

        component.children.emplace_back(
            FlowLayout<>{
                Text(style, "    ["),
                Text(style, cell->offset().q()),
                Text(style, ","),
                Text(style, cell->offset().r()),
                Text(style, "]: "),
                Text(style, std::chrono::duration_cast<std::chrono::seconds>(TimeBase(cell->stall.remaining)).count()),
                Text(style, " ("),
                Text(style, cell->stall.isOnCooldown),
                Text(style, ")")
            });
    }

#ifndef FORCE_ASCII_DEBUG
    _vt = renderToTerm(_vt, 80, component);
    // std::cout << _numClusters << "/" << _numCells << "/" << _numStall << std::endl;
#endif
}
