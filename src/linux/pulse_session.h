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
    pa_mainloop* m_mainLoop;
    pa_context* m_context;
    std::string m_dev;
};


}

