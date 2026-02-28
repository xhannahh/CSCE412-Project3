/**
 * @file WebServer.h
 * @brief Defines the WebServer class that processes requests.
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H
#include "Request.h"
#include <string>

/**
 * @class WebServer
 * @brief Simulates a web server that processes one request at a time.
 */
class WebServer
{
private:
    std::string name;       ///< Name/identifier of the server.
    Request currentRequest; ///< The request currently being processed.
    int reqStartTime;       ///< The clock cycle when the current request was assigned.
    int timeRemaining;      ///< Clock cycles remaining to finish the current request.
    bool busy;              ///< Whether the server is currently processing a request.

public:
    /**
     * @brief Constructs a WebServer with the given name.
     * @param serverName The name/identifier for this server.
     */
    WebServer(std::string serverName);

    /**
     * @brief Assigns a request to this server.
     * @param r The request to process.
     * @param currTime The current system clock cycle.
     */
    void assignRequest(Request r, int currTime);

    /**
     * @brief Advances processing by one clock cycle.
     */
    void processCycle();

    /**
     * @brief Clears the current request and marks the server as idle.
     */
    void clearRequest();

    /**
     * @brief Checks if the server is currently processing a request.
     * @return True if the server is busy, false otherwise.
     */
    bool isBusy() const;

    /**
     * @brief Checks if the server has finished its current request.
     * @return True if the server was busy and has completed processing.
     */
    bool isDone() const;

    /**
     * @brief Gets the name of this server.
     * @return The server name string.
     */
    std::string getName() const;
};

#endif