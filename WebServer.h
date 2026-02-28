#ifndef WEBSERVER_H
#define WEBSERVER_H
#include "Request.h"
#include <string>

class WebServer
{
private:
    std::string name;
    Request currentRequest;
    int reqStartTime;
    int timeRemaining;

public:
    WebServer(std::string serverName);
    void assignRequest(Request r, int currTime);
    void processCycle();
    void clearRequest();
    bool isBusy() const;
    bool isDone() const;
    std::string getName() const;
};

#endif