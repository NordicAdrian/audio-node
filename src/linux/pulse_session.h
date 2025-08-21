#pragma once


#include <pulse/pulseaudio.h>
#include <memory>
#include <string>

namespace nnl_audio
{

class PulseSession
{


public:
    PulseSession();
    ~PulseSession();

    // Disable copy and move
    PulseSession(const PulseSession&) = delete;
    PulseSession& operator=(const PulseSession&) = delete;
    PulseSession(PulseSession&&) = delete;
    PulseSession& operator=(PulseSession&&) = delete;

    int Initialize();
    int SetVolume(float volume);

private:
    pa_mainloop* m_mainLoop;
    pa_context* m_context;
    std::string m_dev;
    int m_channelCount = 2; // Default to stereo, will be set from sink info
    static constexpr int TIMEOUT_MS = 3000; // Timeout for async operations
};


}

