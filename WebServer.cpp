#include "WebServer.h"

WebServer::WebServer(std::string serverName)
{
    name = serverName;
    reqStartTime = 0;
    timeRemaining = 0;
}

void WebServer::assignRequest(Request r, int currTime)
{
    currentRequest = r;
    reqStartTime = currTime;
    timeRemaining = r.time;
}

void WebServer::processCycle()
{
    if (timeRemaining > 0)
    {
        timeRemaining--;
    }
}

void WebServer::clearRequest()
{
    timeRemaining = 0;
}

bool WebServer::isBusy() const
{
    return timeRemaining > 0;
}

bool WebServer::isDone() const
{
    return timeRemaining == 0;
}

std::string WebServer::getName() const
{
    return name;
}