/**
 * @file Request.h
 * @brief Defines the Request struct used to represent a web request.
 */

#ifndef REQUEST_H
#define REQUEST_H
#include <string>

/**
 * @struct Request
 * @brief Represents a single web request with source/destination IPs, processing time, and job type.
 */
struct Request
{
    std::string ipIn;   ///< Source IP address of the request.
    std::string ipOut;  ///< Destination IP address of the request.
    int time;           ///< Time in clock cycles required to process the request.
    char jobType;       ///< Job type: 'P' for processing, 'S' for streaming.

    /**
     * @brief Default constructor. Initializes with empty IPs, time 0, and job type 'P'.
     */
    Request() : ipIn(""), ipOut(""), time(0), jobType('P') {}

    /**
     * @brief Parameterized constructor.
     * @param in Source IP address.
     * @param out Destination IP address.
     * @param time Processing time in clock cycles.
     * @param type Job type character ('P' or 'S').
     */
    Request(std::string in, std::string out, int time, char type)
        : ipIn(in), ipOut(out), time(time), jobType(type) {}
};

#endif