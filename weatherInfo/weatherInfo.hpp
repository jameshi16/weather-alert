#ifndef WEATHERINFO_H
#define WEATHERINFO_H

#include "../json/jsonDecoder.hpp" //can build weatherinfo from jsonDecoder
#include "../utility/point.h" //need a point for coords

/* WeatherInfo is created based off the API for OpenWeatherMap.org, so the data here corresponds to the api there. */
struct WeatherInfo {
    WeatherInfo(JsonDecoder& jDecoder); //construct the weather information from JSON

    std::string getWeather_fancy(); //writes the weather in a fancy fashion and returns it as a string
    void readWeatherData(JsonDecoder& jDecoder);

    /* Data Objects */
    Point<double> coord;
    short weatherId;

    std::string weatherName;
    std::string weatherDesc;
    std::string weatherIcon;

    double temperature;
    int pressure;
    short humidity;
    double temp_min;
    double temp_max;
    long visibility;

    double wind_speed;
    int wind_direction;

    std::string countryCode;
    std::string countryName;
};

#endif