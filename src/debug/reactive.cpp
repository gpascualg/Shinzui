#include "debug/reactive.hpp"

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

Reactive::Reactive()
{
    _vt = new VirtualTerminal();
}

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

    auto component =
        StackLayout<>{
            Text(Style::Default(), "AuraServer debug information: "),
            
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

    component.children.emplace_back(
            FlowLayout<>{
                Text(Style::Default(), "Map (clusters/cells/stall): "),
                Text(Style::Default(), _numClusters),
                Text(Style::Default(), "/"),
                Text(Style::Default(), _numCells),
                Text(Style::Default(), "/"),
                Text(Style::Default(), _numStall)
            });

    // superProgressBar(0.01 * _i, 0.02 * _i, 0.03 * _i);
    *_vt = renderToTerm(*_vt, 80, component);
    // _i += 1;
}
