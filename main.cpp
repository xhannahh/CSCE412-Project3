/**
 * @file main.cpp
 * @brief Entry point for the Load Balancer simulation.
 *
 * Reads configuration from loadbalancer.cfg and runs the selected simulation(s).
 * Usage: ./loadbalancer [normal|bonus|both]
 */

#include "LoadBalancer.h"
#include "BonusLoadBalancer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>

/**
 * @brief Reads an integer value from the config file for a given key.
 * @param path Config file path.
 * @param key The key to look up.
 * @param def Default value if key is missing.
 * @return The parsed integer or the default.
 */
static int cfgInt(const std::string &path, const std::string &key, int def)
{
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        auto eq = line.find('=');
        if (eq == std::string::npos)
            continue;
        std::string k = line.substr(0, eq), v = line.substr(eq + 1);
        auto strip = [](std::string &s)
        {
            auto a = s.find_first_not_of(" \t\r\n");
            auto b = s.find_last_not_of(" \t\r\n");
            s = (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
        };
        strip(k);
        strip(v);
        if (k == key)
            return std::stoi(v);
    }
    return def;
}

/**
 * @brief Main function. Parses CLI args, loads config, and runs simulation(s).
 * @return 0 on success, 1 on invalid arguments.
 */
int main(int argc, char *argv[])
{
    const std::string cfg = "loadbalancer.cfg";
    const std::map<std::string, int> modes = {{"normal", 1}, {"bonus", 2}, {"both", 3}};
    int mode = 3;
    if (argc == 2)
    {
        auto it = modes.find(argv[1]);
        if (it == modes.end())
        {
            std::cout << "Usage: " << argv[0] << " [normal|bonus|both]\n";
            return 1;
        }
        mode = it->second;
    }
    int minS = cfgInt(cfg, "min_servers", 2);
    int maxS = cfgInt(cfg, "max_servers", 10);
    int initS = cfgInt(cfg, "initial_servers", 10);
    int cycles = cfgInt(cfg, "total_cycles", 10000);
    int qMult = cfgInt(cfg, "initial_queue_multiplier", 100);
    std::cout << "\n================================================\n"
              << "LOAD BALANCER SYSTEM STARTUP\n"
              << "================================================\n"
              << "Config: " << cfg
              << " | Servers: " << initS << " (" << minS << "-" << maxS << ")"
              << " | Cycles: " << cycles
              << " | Queue x" << qMult << "\n"
              << "================================================\n\n";
    if (mode & 1)
    {
        LoadBalancer lb(minS, maxS);
        lb.loadConfig(cfg);
        lb.initializeServers(initS);
        lb.runSimulation(cycles, qMult);
        lb.printSummary();
    }
    if (mode & 2)
    {
        BonusLoadBalancer blb(minS, maxS);
        blb.loadConfig(cfg);
        blb.initializeServers(initS);
        blb.runSimulation(cycles);
        blb.printSummary();
    }
    std::cout << "\n================================================\n"
              << "ALL SIMULATIONS COMPLETE\n"
              << "================================================\n\n";
    return 0;
}