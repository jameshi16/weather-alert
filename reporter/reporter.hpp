#ifndef REPORTER_H
#define REPORTER_H

#include "../weatherInfo/weatherStation.hpp" //for WeatherStation
#include "../alerter/alerter.hpp"
#include <string> //for wstring
#include <atomic> //for atomic bool, and unsigned long long
#include <thread> //for threading
#include <mutex> //for thread locking

const unsigned long long defaultSleepSeconds = 300; //5 minutes

//The reporter class. To construct the class, there must be a 
//Weather Station object with cached (either by calling requestData() once, or setting them manually
//and also an alerter object.
class Reporter {
    public:
    Reporter()=delete;

    Reporter(const WeatherStation& ws, const Alerter& alert);
    Reporter(const Reporter&)=delete;
    virtual ~Reporter()=default;

    void start(unsigned long long sleepSeconds); //returns immediately. Starts the reporter.
    void stop(); //returns immediately. Stops the reporter.

    bool hasStarted() {return m_reporterStatus;}
    bool hasStopped() {return !m_reporterStatus;}

    Alerter* getAlerter(); //locks the reporter from accessing alert and weather station. Use releaseLock after complete
    WeatherStation* getWeatherStation(); //locks the reporter from accessign alert and weather station. Use releaseLock after complete

    void releaseLock();

    protected:
    void cycle() noexcept;

    private:
    WeatherStation m_ws;
    Alerter m_alert;

    std::atomic_ullong m_sleepSeconds; //amount of seconds before cycle() does something again
    std::atomic_bool m_break; //whether or not reporter should cycle
    std::atomic_bool m_reporterStatus; //cycle's way of telling if it has started or stopped. true = started, false = stopped

    std::thread threadedCycle; //the thread running cycle()
    std::mutex lock;
};

#endif