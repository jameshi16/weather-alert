#include "contacter.hpp"

#include <algorithm>
#include <sstream>

ContacterErrorCode Contacter::contact(const std::string userAgent, 
                        const std::string url,
                        const std::string target) {
    // Opens a connection to the server
    session = WinHttpOpen(reinterpret_cast<const wchar_t*>(userAgent.c_str()), 
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
                WINHTTP_NO_PROXY_NAME,
                WINHTTP_NO_PROXY_BYPASS, 0);

    // Checks the session to ensure that we've got a connection
    if (!session)
        return RIP_SERVER;

    // Specifies a web server to connect to.
    connect = WinHttpConnect(session, 
                            reinterpret_cast<const wchar_t*>(url.c_str()),
                            INTERNET_DEFAULT_HTTP_PORT, 0);
                
    // Ensure that connection is established
    if (!connect)
        return REJECTED_BY_SERVER;
    
    request = WinHttpOpenRequest(connect, L"GET", 
                                reinterpret_cast<const wchar_t*>(target.c_str()), 
                                NULL, 
                                WINHTTP_NO_REFERER,
                                WINHTTP_DEFAULT_ACCEPT_TYPES,
                                0);

    // Ensure that the request has been created
    bool requestCompleted = WinHttpSendRequest(request,
                                                WINHTTP_NO_ADDITIONAL_HEADERS,
                                                0, WINHTTP_NO_REQUEST_DATA, 0,
                                                0, 0);

    if (!requestCompleted)
        return REQUEST_FAILED;

    return EVERYTHING_IS_GOOD;
}

ContacterErrorCode Contacter::obtainData() {
    /* Pre-requisite Tests, all 3 HINTERNET variables must have a value */
    if (session == NULL || connect == NULL || request == NULL)
        return PREREQUISITE_FAILED;

    bool dataAvailable = WinHttpReceiveResponse(request, NULL);
    long unsigned int dataSize = 0;
    long unsigned int readSize = 0;
    std::stringstream dataStream; //I'm using a stringstream for convinience

    if (!dataAvailable)
        return EVERYTHING_IS_GOOD;

    do {
        // Checks if there is any data.
        if (!WinHttpQueryDataAvailable(request, &dataSize))
            return DATAQUERY_FAILED;

        if (dataAvailable == 0)
            break; //if there is no data to extract, then  don't bother
        
        // Allocates space to acquire data
        char *data = new char[dataSize + 1];
        if (!data)
            return OUT_OF_MEMORY;
        else {
            std::fill(data, data + dataSize + 1, '\0'); //zeros all the memory

            if (!WinHttpReadData(request, &data, dataSize, &readSize))
                return DATA_DOWNLOAD_FAILED;
            
            dataStream << data;
        }
    } while (dataSize > 0);

    // Transfers the data in the string to the string in obtainData().
    data = dataStream.str();
    return EVERYTHING_IS_GOOD;
}

void Contacter::severContact() {
    if (request) WinHttpCloseHandle(request);
    if (connect) WinHttpCloseHandle(connect);
    if (session) WinHttpCloseHandle(session);
}