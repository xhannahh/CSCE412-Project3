/**
 * @file Firewall.h
 * @brief Defines the Firewall class for IP-based request filtering.
 */

#ifndef FIREWALL_H
#define FIREWALL_H
#include <vector>
#include <string>

/**
 * @class Firewall
 * @brief Filters incoming requests by checking IP addresses against blocked ranges.
 *
 * Supports exact IP matching and wildcard prefix matching (e.g., "192.168.*").
 */
class Firewall
{
private:
    std::vector<std::string> blockedRanges; ///< List of blocked IP addresses or wildcard patterns.

public:
    /**
     * @brief Adds an IP range or address to the blocked list.
     * @param range The IP address or wildcard pattern to block (e.g., "10.0.0.*").
     */
    void addBlockedRange(std::string range);

    /**
     * @brief Checks if an IP address is blocked by the firewall.
     * @param ip The IP address to check.
     * @return True if the IP matches a blocked range, false otherwise.
     */
    bool isBlocked(std::string ip) const;
};

#endif