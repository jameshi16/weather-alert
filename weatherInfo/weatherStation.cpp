#include "weatherStation.hpp"

bool WeatherStation::requestData(std::string APIKey, std::string Location) {
    return requestData(std::wstring(APIKey.begin(), APIKey.end()), std::wstring(Location.begin(), Location.end())); //converts std::string to std::wstring
    //contacter uses wstring, because I prefer the future
}

bool WeatherStation::requestData(std::wstring APIKey, std::wstring Location) {
    /* Clears all the objects */
    contact = Contacter();
    jd = JsonDecoder();
    wi = WeatherInfo();

    /* Connect and obtain data */
    if (contact.contact(L"JamesLab Softwares", //agent
                        OPENWEATHERMAP_URL, //url
                        std::wstring() + L"/data/" + OPENWEATHERMAP_VERSION + L"/weather?q=" + Location + L"&appid=" + APIKey)) { //target
            return false; //failed, the function returns 0 if success.
        }

    if (contact.obtainData())
        return false; //failed, the function returns 0 if success.

    contact.severContact(); //severs the contact

    /* Decodes data */
    jd.decode(contact.getData()); //decodes the data. TODO: This is incomplete, JsonDecoder has not much error handling yet.

    /* Copies data to useful structure */
    try {
        wi.readWeatherData(jd); //reads the weather data from the decoded json.
    }
    catch (std::out_of_range& e) {
        return false;
    }

    return true; //read success!
}

WeatherInfo WeatherStation::getWeatherInfo() {
    return wi;
}