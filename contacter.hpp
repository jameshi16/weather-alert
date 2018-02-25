/**
 * Contacter: James' really creative way of saying HTTP socket
**/

#ifndef CONTACTER_H
#define CONTACTER_H

#include "windows.h"
#include "winhttp.h"

#include <string>

enum ContacterErrorCode : short {
    EVERYTHING_IS_GOOD = 0,
    RIP_SERVER = 1,
    REJECTED_BY_SERVER = 2,
    REQUEST_FAILED = 3,
    PREREQUISITE_FAILED = 4,
    DATAQUERY_FAILED = 5,
    OUT_OF_MEMORY = 6,
    DATA_DOWNLOAD_FAILED = 7
};

class Contacter {
    public:
    /* Constructors & Destructors */
    Contacter()=default;
    Contacter(const Contacter&)=default;
    Contacter(Contacter&&)=default;
    virtual ~Contacter()=default;

    /* Default Copy & Move Operators */
    Contacter& operator=(Contacter&)=default;
    Contacter& operator=(Contacter&&)=default;

    /* Equality Operator */
    const bool operator==(const Contacter& other) const;

    /* Functions */
    ContacterErrorCode contact(const std::wstring userAgent,
                const std::wstring url,
                const std::wstring target); //sets up the connection
    
    ContacterErrorCode obtainData(); //obtains the data
    void severContact(); //disconnects the contacter

    std::string getData() const; //gets the underlying data

    protected:
    //session, connection, and request HINTERNET handles
    HINTERNET session = NULL, connect = NULL, request = NULL;

    private:
    std::string data = ""; //the data extracted by obtainData()
};

#endif