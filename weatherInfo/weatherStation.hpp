#ifndef WEATHERSTATION_H
#define WEATHERSTATION_H

#include <string>

#include "weatherInfo.hpp"
#include "../json.hpp"
#include "../contacter.hpp"

#define OPENWEATHERMAP_URL L"api.openweathermap.org" //open weather map's URL
#define OPENWEATHERMAP_VERSION L"2.5" //the version used for open weather map

/* Weather Station is a structure passed down to different window classes, which can be used to tell the weather. */
/** This is an extremely specialized struct and is made for this particular use case only. **/
struct WeatherStation {
    WeatherStation()=default;
    ~WeatherStation()=default;

    /** The requestData function. Takes in a std::string, and converts it to std::wstring, before calling the std::wstring version of the function. **/
    bool requestData(std::string APIKey, std::string Location);
    bool requestData(std::wstring APIKey, std::wstring Location); //this function will request data, and store it in the underlying weather info.

    WeatherInfo getWeatherInfo(); //gets a copy of the weatherinfo.

    private:
    /* Objects used to connect and store data acquired from the server */
    Contacter contact;
    JsonDecoder jd;
    WeatherInfo wi;
};
#endif