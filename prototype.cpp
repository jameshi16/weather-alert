#define PROTOTYPE

// The prototype compilation unit. I used this to see if I am able to contact Open Weather Map effectively.
#ifdef PROTOTYPE

#include <iostream>
#include <string>

#include "windows.h"
#include "contacter.hpp"
#include "json.hpp"
#include "weatherInfo/weatherInfo.hpp"

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
    Contacter contact; //the contacter class
    JsonDecoder jDecoder; //the JSON reader class
    WeatherInfo wi; //the weather info class
    
    /* User Input */
    std::string APIKey; //the API key
    std::string location; //the location to get weather data

    std::cout << "Enter API Key: ";
    std::getline(std::cin, APIKey);

    std::cout << "Enter location: ";
    std::getline(std::cin, location);

    /* Contact the server */
    contact.contact(L"JamesLab Softwares", L"api.openweathermap.org", L"/data/2.5/weather?q=" + std::wstring(location.begin(), location.end()) + L"&appid=" + std::wstring(APIKey.begin(), APIKey.end()));
    contact.obtainData(); //obtains the data
    contact.severContact(); //closes the connection

    jDecoder.decode(contact.getData()); //decodes the JSON acquired
    wi.readWeatherData(jDecoder); //obtains the weather data

    if (wi.weatherName != "Rain")
        std::cout << "All's good, probably." << std::endl;
    else std::cout << "ITS RAINING!" << std::endl;
    
    return 0;
}

#endif 