/**
 * @file Firewall.cpp
 * @brief Implementation of the Firewall class.
 */

#include "Firewall.h"
#include <sstream>

/**
 * @brief Adds an IP range or single address to the blocked list.
 * @param range The IP address or wildcard pattern to block.
 */
void Firewall::addBlockedRange(std::string range)
{
    blockedRanges.push_back(range);
}

/**
 * @brief Checks if a given IP address matches any blocked range.
 *
 * If the range contains no wildcard ('*'), an exact match is checked.
 * If the range contains a wildcard, the portion before '*' is used as a prefix match.
 *
 * @param ip The IP address to check.
 * @return True if the IP is blocked, false otherwise.
 */
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