#include "Firewall.h"
#include <sstream>

void Firewall::addBlockedRange(std::string range)
{
    blockedRanges.push_back(range);
}

bool Firewall::isBlocked(std::string ip) const
{
    for (const std::string &range : blockedRanges)
    {
        if (range.find('*') == std::string::npos)
        {
            if (ip == range)
                return true;
        }
        else
        {
            std::string prefix = range.substr(0, range.find('*'));
            if (ip.compare(0, prefix.size(), prefix) == 0)
                return true;
        }
    }
    return false;
}