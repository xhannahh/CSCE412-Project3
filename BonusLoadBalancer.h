/**
 * @file BonusLoadBalancer.h
 * @brief Defines the BonusLoadBalancer class for routing jobs by type (bonus feature).
 *
 * Routes streaming jobs ('S') to one load balancer and processing jobs ('P')
 * to another, allowing each workload type to be handled by dedicated servers.
 */

#ifndef BONUSLOADBALANCER_H
#define BONUSLOADBALANCER_H
#include "LoadBalancer.h"

/**
 * @class BonusLoadBalancer
 * @brief Higher-level load balancer that sorts requests by job type.
 *
 * Processing ('P') requests go to one LoadBalancer and streaming ('S') requests
 * go to another.  Each internal LoadBalancer scales independently.
 */
class BonusLoadBalancer
{
private:
    LoadBalancer processing;      ///< LoadBalancer for processing ('P') jobs.
    LoadBalancer streaming;       ///< LoadBalancer for streaming ('S') jobs.
    int routedToProcessing;       ///< Requests routed to processing LB.
    int routedToStreaming;        ///< Requests routed to streaming LB.
    std::string bonusLogFilePath; ///< Path to bonus log file.

    /**
     * @brief Writes a message to console and bonus log file.
     * @param msg The message to write.
     */
    void logBonus(const std::string &msg) const;

public:
    /**
     * @brief Constructs with min/max server limits for each internal LoadBalancer.
     * @param minS Minimum number of servers per LoadBalancer.
     * @param maxS Maximum number of servers per LoadBalancer.
     */
    BonusLoadBalancer(int minS, int maxS);

    /**
     * @brief Initializes servers for both processing and streaming LoadBalancers.
     * @param count Number of initial servers for each LoadBalancer.
     */
    void initializeServers(int count);

    /**
     * @brief Routes a request to the appropriate LoadBalancer based on job type.
     * @param r The request to route.
     */
    void routeRequest(const Request &r);

    /**
     * @brief Runs the simulation for both LoadBalancers for the given cycles.
     * @param cycles Number of clock cycles to simulate.
     */
    void runSimulation(int cycles);

    /**
     * @brief Prints a combined summary of both LoadBalancers.
     */
    void printSummary() const;

    /**
     * @brief Loads config settings and applies them to both internal LBs.
     * @param configPath Path to the configuration file.
     */
    void loadConfig(const std::string &configPath);
};

#endif
