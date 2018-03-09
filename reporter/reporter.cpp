#include "reporter.hpp"

Reporter::Reporter(const WeatherStation& ws, const Alerter& alert) : m_ws(ws), m_alert(alert), m_sleepSeconds(0), m_break(false), threadedCycle() {
    if (m_ws.getAPIKey().empty() || m_ws.getLocation().empty())
        throw std::runtime_error("Empty API Keys or Empty Locations are not allowed.");
}

void Reporter::start(unsigned long long sleepSeconds) {
    m_sleepSeconds = sleepSeconds;
    m_break = false; //don't break out of cycle()
    threadedCycle = std::thread(Reporter::cycle, this); //moves a new std::thread to the object
}

void Reporter::stop() {
    m_break = true; //break out of cycle()
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
        }

        std::this_thread::sleep_for(std::chrono::seconds(m_sleepSeconds));
    }
    m_reporterStatus = false;
    return;
}