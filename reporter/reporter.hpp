#ifndef REPORTER_H
#define REPORTER_H

#include "../weatherInfo/weatherStation.hpp" //for WeatherStation
#include "../alerter/alerter.hpp"
#include <string> //for wstring
#include <atomic> //for atomic bool, and unsigned long long
#include <condition_variable> //for a condition variable, for the worker thread to exit while sleeping
#include <thread> //for threading
#include <mutex> //for thread locking

const unsigned long long defaultSleepSeconds = 300; //5 minutes

//The reporter class. To construct the class, there must be a 
//Weather Station object with cached (either by calling requestData() once, or setting them manually
//and also an alerter object.
class Reporter {
    public:
    Reporter()=delete;

    Reporter(const WeatherStation& ws, Alerter&& alert);
    Reporter(const Reporter&)=delete;
    virtual ~Reporter();

    void start(unsigned long long sleepSeconds); //returns immediately. Starts the reporter.
    void stop(); //returns immediately. Stops the reporter.

    bool hasStarted() {return m_reporterStatus;}
    bool hasStopped() {return !m_reporterStatus;}

    HRESULT HandleEvent(UINT_PTR pEventPtr); //passes the event handling to the underlying alerter

    protected:
    void cycle() noexcept;

    private:
    WeatherStation m_ws;
    Alerter m_alert;

    std::atomic_ullong m_sleepSeconds; //amount of seconds before cycle() does something again
    std::atomic_bool m_break; //whether or not reporter should cycle
    std::atomic_bool m_reporterStatus; //whether or not the reporter thread is running
    std::condition_variable m_cvReporterStatus; //used to notify cv if it should act upon a task immediately (i.e. stop sleeping)

    std::thread threadedCycle; //the thread running cycle()
    std::mutex m_alertLock;
    std::mutex m_mutexReporterStatus; //used to notify cv if it should act upon a task immediately (i.e. stop sleeping)
};

#endif