/**
 * @file WebServer.cpp
 * @brief Implementation of the WebServer class.
 */

#include "WebServer.h"

/**
 * @brief Constructs a WebServer with default idle state.
 * @param serverName The name/identifier for this server.
 */
WebServer::WebServer(std::string serverName)
{
    name = serverName;
    reqStartTime = 0;
    timeRemaining = 0;
    busy = false;
}

/**
 * @brief Assigns a new request to this server and marks it busy.
 * @param r The request to be processed.
 * @param currTime The current system clock cycle.
 */
void WebServer::assignRequest(Request r, int currTime)
{
    currentRequest = r;
    reqStartTime = currTime;
    timeRemaining = r.time;
    busy = true;
}

/**
 * @brief Decrements the remaining processing time by one cycle.
 */
void WebServer::processCycle()
{
    if (timeRemaining > 0)
    {
        timeRemaining--;
    }
}

/**
 * @brief Resets the server to an idle state.
 */
void WebServer::clearRequest()
{
    timeRemaining = 0;
    busy = false;
}

/**
 * @brief Returns whether the server is currently busy.
 * @return True if the server is processing a request.
 */
bool WebServer::isBusy() const
{
    return busy;
}

/**
 * @brief Returns whether the server has finished its current request.
 * @return True if the server was busy and has no time remaining.
 */
bool WebServer::isDone() const
{
    return busy && timeRemaining == 0;
}

/**
 * @brief Returns the server's name.
 * @return The name string of this server.
 */
std::string WebServer::getName() const
{
    return name;
}