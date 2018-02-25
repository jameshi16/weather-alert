#include "weatherInfo.hpp"

#include <string> //stod, stoi
#include <iostream> //cerr
#include <sstream> //string stream

WeatherInfo::WeatherInfo(JsonDecoder& jDecoder) {
    readWeatherData(jDecoder);
}

std::string WeatherInfo::getWeather_fancy() {
    std::stringstream buffer;
    buffer << "Here at " << countryName << ", it's " << temperature
            << "F and the humidity is " << humidity << "%. There is currently "
            << weatherDesc << ", and the wind is " << wind_speed << std::endl;

    return buffer.str(); 
}

void WeatherInfo::readWeatherData(JsonDecoder& jDecoder) {
    try {
        coord.setX(std::stod(jDecoder["coord"]["lon"].value));
        coord.setY(std::stod(jDecoder["coord"]["lat"].value));
        weatherId = std::stoi(jDecoder["weather"][0]["id"].value);

        weatherName = jDecoder["weather"][0]["main"].value;
        weatherDesc = jDecoder["weather"][0]["description"].value;
        weatherIcon = jDecoder["weather"][0]["icon"].value;

        temperature = std::stod(jDecoder["main"]["temp"].value);
        pressure = std::stoi(jDecoder["main"]["pressure"].value);
        humidity = std::stoi(jDecoder["main"]["humidity"].value);
        temp_min = std::stod(jDecoder["main"]["temp_min"].value);
        temp_max = std::stod(jDecoder["main"]["temp_max"].value);
        visibility = std::stoi(jDecoder["visibility"].value);
        
        wind_speed = std::stod(jDecoder["wind"]["speed"].value);
        wind_direction = std::stoi(jDecoder["wind"]["deg"].value);

        countryCode = jDecoder["sys"]["country"].value;
        countryName = jDecoder["name"].value;
    }
    catch (std::out_of_range e) {
        std::cerr << "Reading of Weather Data failed. Message: " << e.what() << std::endl;
    }
}