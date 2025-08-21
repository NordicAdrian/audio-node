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

    int Initialize();

    int SetVolume(float volume);



private:

    std::unique_ptr<pa_mainloop> m_mainLoop;
    std::unique_ptr<pa_context> m_context;
    std::string m_dev;
};


}

