#ifndef RXTERM_TERMINAL_HPP
#define RXTERM_TERMINAL_HPP


#include <iostream>
#include <algorithm>
#include <string>
#include <rxterm/utils.hpp>

namespace rxterm {

struct VirtualTerminal {
  std::string buffer;

  std::string computeTransition(std::string const& next) const {
    if(buffer == next) return "";
    
    unsigned const n = std::count(buffer.begin(), buffer.end(), '\n');
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    return clearLines(std::max((int)n, (int)w.ws_row)) + "\033[0m;" + next;
  }

  static std::string hide() { return "\033[0;8m"; }

  VirtualTerminal flip(std::string const& next) const {
    auto const& transition = computeTransition(next);
    if(transition == "") return *this;
    std::cout << transition << hide();
    std::flush(std::cout);
    return {next};
  }
};

}

#endif
