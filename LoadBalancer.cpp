/**
 * @file LoadBalancer.cpp
 * @brief Implementation of the LoadBalancer class.
 */

#include "LoadBalancer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

namespace
{
    const char *CRESET = "\033[0m";
    const char *CCYAN = "\033[36m";
    const char *CGREEN = "\033[32m";
    const char *CMAGENTA = "\033[35m";
    const char *CRED = "\033[31m";
    const char *CYELLOW = "\033[33m";
    const char *CBLUE = "\033[34m";

    void trimInPlace(string &text)
    {
        const size_t first = text.find_first_not_of(" \t\r\n");
        if (first == string::npos)
        {
            text.clear();
            return;
        }
        const size_t last = text.find_last_not_of(" \t\r\n");
        text = text.substr(first, last - first + 1);
    }

    string randomIp()
    {
        return to_string(rand() % 256) + "." + to_string(rand() % 256) + "." +
               to_string(rand() % 256) + "." + to_string(rand() % 256);
    }

    string colorize(const string &msg)
    {
        const char *c = nullptr;
        if (msg.find("SCALE UP") != string::npos)
            c = CGREEN;
        else if (msg.find("SCALE DOWN") != string::npos)
            c = CMAGENTA;
        else if (msg.find("BLOCKED") != string::npos)
            c = CRED;
        else if (msg.find("CHECKPOINT") != string::npos)
            c = CCYAN;
        else if (msg.find("SIMULATION STARTED") != string::npos)
            c = CBLUE;
        else if (msg.find("SUMMARY") != string::npos)
            c = CYELLOW;
        return c ? string(c) + msg + CRESET : msg;
    }
}

/**
 * @brief Constructs a LoadBalancer with min/max server limits and default config.
 * @param minS Minimum number of servers.
 * @param maxS Maximum number of servers.
 */
LoadBalancer::LoadBalancer(int minS, int maxS)
    : systemTime(0), minServers(minS), maxServers(maxS),
      scaleCooldown(5), timeSinceLastScale(0),
      totalProcessed(0), totalScaled(0), totalBlocked(0),
      scaleUpThreshold(80), scaleDownThreshold(50),
      minRequestTime(1), maxRequestTime(10),
      maxNewRequests(5), loggingEnabled(false)
{
    srand(time(nullptr));
}

/**
 * @brief Destructor. Closes the log file if open.
 */
LoadBalancer::~LoadBalancer()
{
    if (logFile.is_open())
        logFile.close();
}

/**
 * @brief Writes a message to stdout and the log file (if enabled).
 * @param msg The message to log.
 */
void LoadBalancer::log(const string &msg)
{
    cout << colorize(msg) << endl;
    if (loggingEnabled && logFile.is_open())
        logFile << msg << endl;
}

/**
 * @brief Loads configuration from a file and applies settings.
 * @param configPath Path to the configuration file.
 * @param enableFileLogging Whether to open the log file for writing.
 */
void LoadBalancer::loadConfig(const string &configPath, bool enableFileLogging)
{
    ifstream file(configPath);
    if (!file.is_open())
    {
        cout << "Warning: Could not open config file '" << configPath
             << "'. Using defaults." << endl;
        return;
    }
    string line, logFilePath;
    while (getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        size_t eq = line.find('=');
        if (eq == string::npos)
            continue;
        string key = line.substr(0, eq);
        string val = line.substr(eq + 1);
        trimInPlace(key);
        trimInPlace(val);

        if (key == "min_servers")
            minServers = stoi(val);
        else if (key == "max_servers")
            maxServers = stoi(val);
        else if (key == "scale_cooldown")
            scaleCooldown = stoi(val);
        else if (key == "scale_up_threshold")
            scaleUpThreshold = stoi(val);
        else if (key == "scale_down_threshold")
            scaleDownThreshold = stoi(val);
        else if (key == "min_request_time")
            minRequestTime = stoi(val);
        else if (key == "max_request_time")
            maxRequestTime = stoi(val);
        else if (key == "max_new_requests")
            maxNewRequests = stoi(val);
        else if (key == "log_file")
            logFilePath = val;
        else if (key == "blocked_ip")
            firewall.addBlockedRange(val);
    }
    file.close();
    if (enableFileLogging && !logFilePath.empty())
    {
        logFile.open(logFilePath);
        loggingEnabled = logFile.is_open();
        if (!loggingEnabled)
            cout << "Warning: Could not open log file '" << logFilePath << "'." << endl;
    }
    log("===== Configuration Loaded =====");
    log("  Servers: " + to_string(minServers) + "-" + to_string(maxServers) +
        " | Cooldown: " + to_string(scaleCooldown) + " cycles");
    log("  Scale Up: " + to_string(scaleUpThreshold) + "/server"
                                                       " | Scale Down: " +
        to_string(scaleDownThreshold) + "/server");
    log("  Request Time: " + to_string(minRequestTime) + "-" +
        to_string(maxRequestTime) + " cycles"
                                    " | Max New/Cycle: " +
        to_string(maxNewRequests));
    log("================================");
}

/**
 * @brief Creates the initial pool of web servers.
 * @param count Number of servers to create.
 */
void LoadBalancer::initializeServers(int count)
{
    for (int i = 0; i < count; i++)
        servers.emplace_back("Server_" + to_string(i + 1));
    log("Initialized " + to_string(count) + " servers.");
}

/**
 * @brief Generates a random Request with random IPs, time, and job type.
 * @return A randomly generated Request.
 */
Request LoadBalancer::createRandomRequest()
{
    Request r;
    r.ipIn = randomIp();
    r.ipOut = randomIp();
    r.time = rand() % (maxRequestTime - minRequestTime + 1) + minRequestTime;
    r.jobType = (rand() % 2 == 0) ? 'P' : 'S';
    return r;
}

/**
 * @brief Generates new requests each cycle, adapting rate to queue pressure.
 */
void LoadBalancer::createIncomingRequests()
{
    if (maxNewRequests <= 0)
        return;
    const int sc = max(1, static_cast<int>(servers.size()));
    const int qs = static_cast<int>(requestQueue.size());
    const int lower = scaleDownThreshold * sc;
    const int upper = scaleUpThreshold * sc;
    int n = 0;
    if (qs > upper)
        n = rand() % 2;
    else if (qs < lower)
    {
        int lo = max(1, maxNewRequests / 2);
        n = lo + rand() % (maxNewRequests - lo + 1);
    }
    else
        n = rand() % (max(1, maxNewRequests / 2) + 1);

    for (int i = 0; i < n; i++)
        enqueueIfAllowed(createRandomRequest());
}

/**
 * @brief Executes one clock cycle of the simulation.
 */
void LoadBalancer::processCycle()
{
    systemTime++;
    timeSinceLastScale++;
    createIncomingRequests();
    for (auto &s : servers)
        if (!s.isBusy() && !requestQueue.empty())
        {
            s.assignRequest(requestQueue.front(), systemTime);
            requestQueue.pop();
        }

    for (auto &s : servers)
        if (s.isBusy())
        {
            s.processCycle();
            if (s.isDone())
            {
                totalProcessed++;
                s.clearRequest();
            }
        }

    adjustServers();
}

/**
 * @brief Dynamically adjusts server count based on queue size.
 */
void LoadBalancer::adjustServers()
{
    if (timeSinceLastScale < scaleCooldown)
        return;
    int sc = static_cast<int>(servers.size());
    int qs = static_cast<int>(requestQueue.size());
    if (qs > scaleUpThreshold * sc && sc < maxServers)
    {
        servers.emplace_back("Server_" + to_string(sc + 1));
        totalScaled++;
        timeSinceLastScale = 0;
        log("[Cycle " + to_string(systemTime) + "] SCALE UP -> Servers: " +
            to_string(sc + 1) + " | Queue: " + to_string(qs));
    }
    else if (qs < scaleDownThreshold * sc && sc > minServers)
    {
        removeServer();
        if (static_cast<int>(servers.size()) < sc)
        {
            totalScaled++;
            timeSinceLastScale = 0;
            log("[Cycle " + to_string(systemTime) + "] SCALE DOWN -> Servers: " +
                to_string(servers.size()) + " | Queue: " + to_string(qs));
        }
    }
}

/**
 * @brief Removes the last server from the pool.
 */
void LoadBalancer::removeServer()
{
    if (!servers.empty())
        servers.pop_back();
}

/**
 * @brief Adds a single request to the queue.
 * @param r The request to enqueue.
 */
void LoadBalancer::addRequest(const Request &r)
{
    requestQueue.push(r);
}

/**
 * @brief Adds a request only if firewall rules allow it.
 * @param r The request to enqueue.
 * @return True if enqueued, false if blocked.
 */
bool LoadBalancer::enqueueIfAllowed(const Request &r)
{
    if (firewall.isBlocked(r.ipIn))
    {
        totalBlocked++;
        log("[Cycle " + to_string(systemTime) + "] BLOCKED -> IP: " + r.ipIn);
        return false;
    }
    requestQueue.push(r);
    return true;
}

/**
 * @brief Runs the full simulation for the given number of clock cycles.
 * @param cycles Number of clock cycles to simulate.
 * @param queueMultiplier Requests per server to prefill.
 */
void LoadBalancer::runSimulation(int cycles, int queueMultiplier)
{
    log("\n========================================");
    log("LOAD BALANCER SIMULATION STARTED");
    log("========================================");
    log("Total Cycles: " + to_string(cycles) +
        " | Initial Servers: " + to_string(servers.size()));
    log("----------------------------------------");

    if (queueMultiplier > 0)
    {
        const int target = static_cast<int>(servers.size()) * queueMultiplier;
        for (int i = 0; i < target; i++)
            requestQueue.push(createRandomRequest());
        log("Initial queue: " + to_string(requestQueue.size()) + " requests.");
    }
    for (int i = 0; i < cycles; i++)
    {
        processCycle();
        if ((i + 1) % 1000 == 0)
            log("[Cycle " + to_string(systemTime) + "] CHECKPOINT -> Servers: " +
                to_string(servers.size()) + " | Queue: " +
                to_string(requestQueue.size()));
    }
    log("----------------------------------------");
    log("Simulation Complete.");
    log("========================================");
}

/**
 * @brief Returns a formatted summary string of simulation statistics.
 * @return The summary as a multi-line string.
 */
string LoadBalancer::summaryString() const
{
    int sc = static_cast<int>(servers.size());
    string s;
    s += "\n========================================\n";
    s += "LOAD BALANCER SIMULATION SUMMARY\n";
    s += "========================================\n\n";
    s += "Simulation Time:      " + to_string(systemTime) + " cycles\n";
    s += "Final Active Servers: " + to_string(sc) + "\n\n";
    s += "--- Request Statistics ---\n";
    s += "Processed: " + to_string(totalProcessed) +
         " | Blocked: " + to_string(totalBlocked) +
         " | Ending Queue: " + to_string(requestQueue.size()) + "\n\n";
    s += "--- Scaling Statistics ---\n";
    s += "Scaling Events: " + to_string(totalScaled) +
         " | Servers: " + to_string(minServers) + "-" + to_string(maxServers) + "\n";
    s += "Scale Up:   " + to_string(scaleUpThreshold) + " x " + to_string(sc) +
         " = " + to_string(scaleUpThreshold * sc) + " requests\n";
    s += "Scale Down: " + to_string(scaleDownThreshold) + " x " + to_string(sc) +
         " = " + to_string(scaleDownThreshold * sc) + " requests\n";
    s += "Cooldown:   " + to_string(scaleCooldown) + " cycles\n\n";
    s += "--- Request Configuration ---\n";
    s += "Time Range: " + to_string(minRequestTime) + "-" +
         to_string(maxRequestTime) + " cycles"
                                     " | Max New/Cycle: " +
         to_string(maxNewRequests) + "\n";
    s += "\n========================================\n";
    return s;
}

/**
 * @brief Prints the simulation summary to stdout and the log file.
 */
void LoadBalancer::printSummary() const
{
    string s = summaryString();
    cout << s;
    if (loggingEnabled && logFile.is_open())
        logFile << s;
}