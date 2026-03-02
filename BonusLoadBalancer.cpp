/**
 * @file BonusLoadBalancer.cpp
 * @brief Implementation of the BonusLoadBalancer class.
 */

#include "BonusLoadBalancer.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
using namespace std;

namespace
{
    const char *CRESET = "\033[0m";
    const char *CBLUE = "\033[34m";
    const char *CGREEN = "\033[32m";
    const char *CRED = "\033[31m";
    const char *CCYAN = "\033[36m";
    const char *CYELLOW = "\033[33m";

    string colorize(const string &msg)
    {
        const char *c = nullptr;
        if (msg.find("Starting BONUS") != string::npos)
            c = CBLUE;
        else if (msg.find("Routed") != string::npos)
            c = CGREEN;
        else if (msg.find("BLOCKED") != string::npos)
            c = CRED;
        else if (msg.find("Queue:") != string::npos)
            c = CCYAN;
        else if (msg.find("SUMMARY") != string::npos)
            c = CYELLOW;
        return c ? string(c) + msg + CRESET : msg;
    }
}

/**
 * @brief Constructs with min/max server limits for each internal LoadBalancer.
 * @param minS Minimum number of servers per LoadBalancer.
 * @param maxS Maximum number of servers per LoadBalancer.
 */
BonusLoadBalancer::BonusLoadBalancer(int minS, int maxS)
    : processing(minS, maxS), streaming(minS, maxS),
      routedToProcessing(0), routedToStreaming(0),
      bonusLogFilePath("bonusloadbalancer.log") {}

/**
 * @brief Writes a message to console and bonus log file.
 * @param msg The message to write.
 */
void BonusLoadBalancer::logBonus(const string &msg) const
{
    cout << colorize(msg) << endl;
    ofstream out(bonusLogFilePath, ios::app);
    if (out.is_open())
        out << msg << endl;
}

/**
 * @brief Initializes servers for both processing and streaming LoadBalancers.
 * @param count Number of initial servers for each LoadBalancer.
 */
void BonusLoadBalancer::initializeServers(int count)
{
    processing.initializeServers(count);
    streaming.initializeServers(count);
}

/**
 * @brief Routes a request to the appropriate LoadBalancer based on job type.
 * @param r The request to route.
 */
void BonusLoadBalancer::routeRequest(const Request &r)
{
    if (r.jobType == 'S')
    {
        streaming.addRequest(r);
        routedToStreaming++;
    }
    else
    {
        processing.addRequest(r);
        routedToProcessing++;
    }
}

/**
 * @brief Runs the simulation for both LoadBalancers for the given cycles.
 * @param cycles Number of clock cycles to simulate.
 */
void BonusLoadBalancer::runSimulation(int cycles)
{
    ofstream(bonusLogFilePath, ios::trunc).close(); // reset log
    routedToProcessing = 0;
    routedToStreaming = 0;
    logBonus("\n========================================");
    logBonus("Starting BONUS Load Balancer Simulation...");
    logBonus("========================================");
    logBonus("Total Cycles: " + to_string(cycles));
    logBonus("Processing Servers: " + to_string(processing.getServerCount()) +
             " | Streaming Servers: " + to_string(streaming.getServerCount()));
    logBonus("");

    const int maxNew = processing.getMaxNewRequests();
    if (maxNew <= 0)
    {
        logBonus("Error: max_new_requests not configured or invalid.");
        return;
    }

    // Disable internal request generation so all requests flow through the bonus router
    processing.maxNewRequests = 0;
    streaming.maxNewRequests = 0;

    int pRouted = 0, sRouted = 0, pBlocked = 0, sBlocked = 0;
    for (int i = 0; i < cycles; ++i)
    {
        int n = rand() % (maxNew + 1);
        for (int j = 0; j < n; ++j)
        {
            Request req = processing.createRandomRequest();
            if (req.jobType == 'P')
            {
                if (processing.enqueueIfAllowed(req))
                {
                    routedToProcessing++;
                    pRouted++;
                }
                else
                    pBlocked++;
            }
            else
            {
                if (streaming.enqueueIfAllowed(req))
                {
                    routedToStreaming++;
                    sRouted++;
                }
                else
                    sBlocked++;
            }
        }

        if ((i + 1) % 100 == 0)
        {
            logBonus("[Cycle " + to_string(i + 1) +
                     "] Routed P:" + to_string(pRouted) +
                     " S:" + to_string(sRouted) +
                     " | Blocked P:" + to_string(pBlocked) +
                     " S:" + to_string(sBlocked));
            logBonus("  Processing Queue: " + to_string(processing.getQueueSize()) +
                     " | Streaming Queue: " + to_string(streaming.getQueueSize()));
            pRouted = sRouted = pBlocked = sBlocked = 0;
        }

        processing.processCycle();
        streaming.processCycle();
    }

    logBonus("\n========================================");
    logBonus("Simulation Complete.");
    logBonus("========================================\n");
}

/**
 * @brief Prints a combined summary of both LoadBalancers.
 */
void BonusLoadBalancer::printSummary() const
{
    logBonus("========================================");
    logBonus("BONUS LOAD BALANCER ROUTING SUMMARY");
    logBonus("========================================");
    int total = routedToProcessing + routedToStreaming;
    int procActive = processing.getServerCount();
    int streamActive = streaming.getServerCount();
    int totalActive = procActive + streamActive;
    int totalInactive = (processing.maxServers - procActive) +
                        (streaming.maxServers - streamActive);
    int remainingQueue = static_cast<int>(processing.getQueueSize() +
                                          streaming.getQueueSize());
    int rejected = processing.totalBlocked + streaming.totalBlocked;
    logBonus("Simulation Status: Complete");
    logBonus("Total Routed: " + to_string(total) +
             " | Processing: " + to_string(routedToProcessing) +
             " | Streaming: " + to_string(routedToStreaming));
    logBonus("Active Servers: " + to_string(totalActive) +
             " | Inactive Servers: " + to_string(totalInactive));
    logBonus("Remaining Requests in Queue: " + to_string(remainingQueue));
    logBonus("Rejected/Discarded Requests: " + to_string(rejected));
    logBonus("\n--- PROCESSING LB ---");
    int procInactive = processing.maxServers - procActive;
    logBonus("Active Servers:   " + to_string(procActive));
    logBonus("Inactive Servers: " + to_string(procInactive));
    logBonus(processing.summaryString());
    logBonus("--- STREAMING LB ---");
    int streamInactive = streaming.maxServers - streamActive;
    logBonus("Active Servers:   " + to_string(streamActive));
    logBonus("Inactive Servers: " + to_string(streamInactive));
    logBonus(streaming.summaryString());
    logBonus("========================================");
}

/**
 * @brief Loads config settings and applies them to both internal LBs.
 * @param configPath Path to the configuration file.
 */
void BonusLoadBalancer::loadConfig(const string &configPath)
{
    processing.loadConfig(configPath, false);
    streaming.loadConfig(configPath, false);
    ifstream file(configPath);
    if (!file.is_open())
        return;

    string line;
    while (getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        size_t eq = line.find('=');
        if (eq == string::npos)
            continue;

        string key = line.substr(0, eq);
        string val = line.substr(eq + 1);
        auto trim = [](string &s)
        {
            size_t a = s.find_first_not_of(" \t\r\n");
            size_t b = s.find_last_not_of(" \t\r\n");
            s = (a == string::npos) ? "" : s.substr(a, b - a + 1);
        };
        trim(key);
        trim(val);

        if (key == "bonus_log_file" && !val.empty())
        {
            bonusLogFilePath = val;
            break;
        }
    }
}
