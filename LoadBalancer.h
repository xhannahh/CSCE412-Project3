/**
 * @file LoadBalancer.h
 * @brief Defines the LoadBalancer class that manages web servers and a request queue.
 */

#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include <queue>
#include <vector>
#include <string>
#include <fstream>
#include "Request.h"
#include "WebServer.h"
#include "Firewall.h"

/**
 * @class LoadBalancer
 * @brief Manages a pool of web servers and distributes incoming requests from a queue.
 *
 * Dynamically scales servers based on queue size relative to active server count.
 * Uses a Firewall to block requests from certain IP ranges.  Configuration is
 * loaded from an external config file.
 */
class LoadBalancer
{
private:
    std::queue<Request> requestQueue; ///< Queue of pending requests.
    std::vector<WebServer> servers;   ///< Pool of active web servers.
    int systemTime;                   ///< Current simulation clock cycle.
    Firewall firewall;                ///< Firewall for IP filtering.
    int minServers;                   ///< Minimum number of servers allowed.
    int maxServers;                   ///< Maximum number of servers allowed.
    int scaleCooldown;                ///< Minimum cycles between scaling events.
    int timeSinceLastScale;           ///< Cycles elapsed since the last scaling event.
    int totalProcessed;               ///< Total requests processed.
    int totalScaled;                  ///< Total scaling events.
    int totalBlocked;                 ///< Total requests blocked by firewall.
    int scaleUpThreshold;             ///< Per-server threshold to trigger scale up.
    int scaleDownThreshold;           ///< Per-server threshold to trigger scale down.
    int minRequestTime;               ///< Minimum processing time for a request.
    int maxRequestTime;               ///< Maximum processing time for a request.
    int maxNewRequests;               ///< Maximum new requests generated per cycle.
    mutable std::ofstream logFile;    ///< Output stream for the log file.
    bool loggingEnabled;              ///< Whether file logging is active.

    /**
     * @brief Writes a message to stdout and the log file (if enabled).
     * @param msg The message to log.
     */
    void log(const std::string &msg);

    /**
     * @brief Grants BonusLoadBalancer access to private members for advanced routing and statistics.
     */
    friend class BonusLoadBalancer;

public:
    /**
     * @brief Constructs a LoadBalancer with min/max server limits.
     * @param minS Minimum number of servers.
     * @param maxS Maximum number of servers.
     */
    LoadBalancer(int minS, int maxS);

    /**
     * @brief Destructor. Closes the log file if open.
     */
    ~LoadBalancer();

    /**
     * @brief Loads configuration from a file and applies settings.
     * @param configPath Path to the configuration file.
     * @param enableFileLogging Whether to open the log file for writing.
     */
    void loadConfig(const std::string &configPath, bool enableFileLogging = true);

    /**
     * @brief Creates the initial set of web servers.
     * @param count Number of servers to create.
     */
    void initializeServers(int count);

    /**
     * @brief Generates a random Request with random IPs, time, and job type.
     * @return A randomly generated Request.
     */
    Request createRandomRequest();

    /**
     * @brief Generates new requests each cycle, adapting rate to queue pressure.
     */
    void createIncomingRequests();

    /**
     * @brief Executes one clock cycle of the simulation.
     */
    void processCycle();

    /**
     * @brief Dynamically adjusts the number of servers based on queue size.
     */
    void adjustServers();

    /**
     * @brief Removes one server from the pool.
     */
    void removeServer();

    /**
     * @brief Adds a single request to the queue.
     * @param r The request to enqueue.
     */
    void addRequest(const Request &r);

    /**
     * @brief Adds a request only if firewall rules allow it.
     * @param r The request to enqueue.
     * @return True if enqueued, false if blocked.
     */
    bool enqueueIfAllowed(const Request &r);

    /**
     * @brief Runs the full simulation for the given number of clock cycles.
     * @param cycles Number of clock cycles to simulate.
     * @param queueMultiplier Requests per server to prefill (0 to skip).
     */
    void runSimulation(int cycles, int queueMultiplier = 0);

    /**
     * @brief Returns a formatted summary string of simulation statistics.
     * @return The summary as a multi-line string.
     */
    std::string summaryString() const;

    /**
     * @brief Prints the simulation summary to stdout and the log file.
     */
    void printSummary() const;

    /**
     * @brief Gets the current size of the pending request queue.
     * @return Number of requests waiting in the queue.
     */
    size_t getQueueSize() const { return requestQueue.size(); }

    /**
     * @brief Gets the number of active servers.
     * @return Current server count.
     */
    size_t getServerCount() const { return servers.size(); }

    /**
     * @brief Gets configured maximum number of new requests per cycle.
     * @return The max new requests per cycle value.
     */
    int getMaxNewRequests() const { return maxNewRequests; }
};

#endif
