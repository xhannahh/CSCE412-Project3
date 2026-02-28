#ifndef REQUEST_H
#define REQUEST_H
#include <string>

struct Request
{
    std::string ipIn;
    std::string ipOut;
    int time;
    char jobType;

    Request() : ipIn(""), ipOut(""), time(0), jobType('P') {}
    Request(std::string in, std::string out, int time, char type)
        : ipIn(in), ipOut(out), time(time), jobType(type) {}
};

#endif