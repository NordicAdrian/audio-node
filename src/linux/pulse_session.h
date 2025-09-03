#pragma once


#include <pulse/pulseaudio.h>
#include <vector>
#include <memory>
#include <string>

namespace nnl_audio
{


    namespace pulse
    {

        struct LoopbackData {
            pa_stream* playbackStream = nullptr;
            pa_stream* recordStream = nullptr;
            pa_mainloop* mainLoop = nullptr;
            std::string sourceName;
            std::string sinkName;
        };

        constexpr int TIMEOUT_MS = 1000;


        int SetEndpointVolume(const std::string& endPointName, float volume);
        int StartLoopbackStream(std::string& sourceName, const std::string& sinkName);
        int StopLoopbackStream();
    

        int EnsureOperation(pa_operation* op, pa_mainloop* mainLoop);
        int CreateContext(pa_context** c, pa_mainloop** m);
        void StartLoopbackCB(pa_context* c, void* userdata);
        void ReadLoopbackCB(pa_stream* s, size_t length, void* userdata);
        void SourceInfoCB(pa_context* c, const pa_source_info* info, int eol, void* userdata);



        int GetChannelCount(const std::string& deviceName, int& channelCount);
        int GetConnectedOutputDevices(std::vector<std::string>& deviceNames);
    }


}




