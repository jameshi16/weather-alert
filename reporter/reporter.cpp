#include "reporter.hpp"

#include <iostream>
#include <condition_variable>

Reporter::Reporter(const WeatherStation& ws, const Alerter& alert) : m_ws(ws), m_alert(alert), m_sleepSeconds(0), m_break(false), m_reporterStatus(false), threadedCycle() {
    if (m_ws.getAPIKey().empty() || m_ws.getLocation().empty())
        throw std::runtime_error("Empty API Keys or Empty Locations are not allowed.");
}

Reporter::~Reporter() {
    if (m_reporterStatus) {
        stop();
        threadedCycle.join(); //waits for the thread
    }
    //lets C++ destroy the rest of the objects
}

void Reporter::start(unsigned long long sleepSeconds) {
    m_sleepSeconds = sleepSeconds;
    m_break = false; //don't break out of cycle()
    threadedCycle = std::thread(Reporter::cycle, this); //moves a new std::thread to the object
}

void Reporter::stop() {
    std::lock_guard<std::mutex> lg(m_mutexReporterStatus); 
    m_break = true; //break out of cycle()
    m_cvReporterStatus.notify_all(); //notifies cycle to break immediately
}

Alerter* Reporter::getAlerter() {
    lock.lock();
    return &m_alert;
}

WeatherStation* Reporter::getWeatherStation() {
    lock.lock();
    return &m_ws;
}

void Reporter::releaseLock() {
    lock.unlock();
}

void Reporter::cycle() noexcept {
    m_reporterStatus = true;
    while (!m_break) {
        m_ws.requestData(); //requests for the data.
        if (m_ws.getWeatherInfo().weatherName == "Rain") {
            m_alert.setSoundFile("./alert.mp3");
            m_alert.playAudio();

            if (m_alert.getLastError() == FAILED_TO_PLAY)
                std::cerr << "Cannot play sound." << std::endl;
        }

        if (!m_break) { //check m_break here. if break is request before the condition variable sleep, then just skip to the next loop
            std::unique_lock<std::mutex> u_l(m_mutexReporterStatus);
            m_cvReporterStatus.wait_for(u_l, std::chrono::seconds(m_sleepSeconds)); //waits for a thread to signal
        }
    }
    m_alert.stopAudio();

    if (m_alert.getLastError() == FAILED_TO_STOP)
        std::cerr << "Cannot stop sound." << std::endl;

    m_reporterStatus = false;
    return;
}