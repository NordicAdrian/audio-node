#pragma once


#include <pulse/pulseaudio.h>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <iostream>
#include <string>

namespace nnl_audio
{


    namespace pulse
    {

        
        constexpr int TIMEOUT_MS = 1000;


        int SetEndpointVolume(const std::string& endPointName, float volume);
    

        int EnsureOperation(pa_operation* op, pa_mainloop* mainLoop);
        int CreateContext(pa_context** c, pa_mainloop** m);


        int GetChannelCount(const std::string& deviceName, int& channelCount);
        int GetConnectedOutputDevices(std::vector<std::string>& deviceNames);
    }


}




