#ifndef FIREWALL_H
#define FIREWALL_H
#include <vector>
#include <string>

class Firewall
{
private:
    std::vector<std::string> blockedRanges;

public:
    void addBlockedRange(std::string range);
    bool isBlocked(std::string ip) const;
};

#endif