#include "control/FileVolumeController.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <stdexcept>
#include <iomanip>

namespace AudioMixer
{
    VolumeEvent parseVolumeLine(const std::string& line)
    {
        std::istringstream in(line);
        std::tm tm{};
        char dot = 0;
        int millis = 0;
        int feed = 0;
        int percent = 0;
        in >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S") >> dot >> millis >> feed >> percent;

        Micros seconds = timegm(&tm);
        return {seconds * MICROS_IN_SECOND + millis * 1000, static_cast<std::size_t>(feed), percent};
    }


    FileVolumeController::FileVolumeController(const std::string& path, VolumeTable& volTable, const Timeline& timeline):m_volumeTable(volTable), m_timeline(timeline)
    {
        std::ifstream file(path);
        if (!file.is_open())
            throw std::runtime_error("cannot open volume script " + path);

        std::string line;

        while(std::getline(file, line))
            if(!line.empty())
                m_events.push_back(parseVolumeLine(line));
       
        //reorder in case the volumefile is not in order (unlikeley but safer)
        //use stable_sort for even safer reordering
        std::stable_sort(m_events.begin(), m_events.end(),
        [](const VolumeEvent& a, const VolumeEvent& b) { return a.time < b.time; });
                
    }

    void FileVolumeController::start()
    {
        m_thread = std::thread(&FileVolumeController::run, this);
    }

    void FileVolumeController::run()
    {
        for(const auto& event : m_events)
        {
            
            if(!waitUntil(m_timeline.playTime(event.time)))
                return; //thread stopped

            m_volumeTable.setVolume(event.feed, event.percent);
        }
    }

    void FileVolumeController::stop()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopRequested = true;
        }
        //wake up the thread
        m_wake.notify_all();
        if (m_thread.joinable())
            m_thread.join();
    }

    bool FileVolumeController::waitUntil(Clock::time_point deadline)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return !m_wake.wait_until(lock, deadline, [this] { return m_stopRequested; });
    }

    FileVolumeController::~FileVolumeController()
    {
        stop();
    }

}