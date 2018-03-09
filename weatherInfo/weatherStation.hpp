#ifndef WEATHERSTATION_H
#define WEATHERSTATION_H

#include <string>

#include "weatherInfo.hpp"
#include "../json.hpp"
#include "../contacter.hpp"

#define OPENWEATHERMAP_URL L"api.openweathermap.org" //open weather map's URL
#define OPENWEATHERMAP_VERSION L"2.5" //the version used for open weather map

/* Weather Station is a structure passed down to different window classes, which can be used to tell the weather. */
/* Weather Station caches the values that are used to call requestData(). The cache can be tempered with through set or getAPIKey, set or getLocation */
/** This is an extremely specialized struct and is made for this particular use case only. **/
struct WeatherStation {
    WeatherStation()=default;
    ~WeatherStation()=default;

    /** Gets/Sets for both APIKey and Location **/
    void setAPIKey(std::wstring APIKey) {m_APIKey = APIKey;}
    void setLocation(std::wstring Location) {m_Location = Location;}

    void setAPIKey(std::string APIKey) {setAPIKey(std::wstring(APIKey.begin(), APIKey.end()));}
    void setLocation(std::string Location) {setAPIKey(std::wstring(Location.begin(), Location.end()));}

    std::wstring getAPIKey() const {return m_APIKey;}
    std::wstring getLocation() const {return m_Location;}

    /** The requestData function. Takes in a std::string, and converts it to std::wstring, before calling the std::wstring version of the function. **/
    bool requestData(std::string APIKey, std::string Location);
    bool requestData(std::wstring APIKey, std::wstring Location); //this function will request data, and store it in the underlying weather info.

    bool requestData() noexcept(false); //requests data based on stored values.

    WeatherInfo getWeatherInfo(); //gets a copy of the weatherinfo.

    private:
    /* Objects used to connect and store data acquired from the server */
    Contacter contact;
    JsonDecoder jd;
    WeatherInfo wi;

    std::wstring m_APIKey, m_Location;
};
#endif