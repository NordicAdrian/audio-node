#pragma once


#include <pulse/pulseaudio.h>
#include <memory>
#include <string>

namespace nnl_audio
{


    namespace pulse
    {

        constexpr int TIMEOUT_MS = 1000;
        inline pa_stream* playbackStream = nullptr;
        inline pa_stream* recordStream = nullptr;

        int SetEndpointVolume(const std::string& endPointName, float volume);
        int StartLoopbackStream(const std::string& sourceName, const std::string& sinkName);
        int StopLoopbackStream();
        int EnsureOperation(pa_operation* op, pa_mainloop* mainLoop, void* userdata, int timeoutMs = TIMEOUT_MS);


        int CreateContext(pa_context** c, pa_mainloop** m);
        void ContextStateCB(pa_context* c, void* userdata);
        void ReadCB(pa_stream* s, size_t length, void* userdata);
        void SourceInfoCB(pa_context* c, const pa_source_info* info, int eol, void* userdata);



        int GetChannelCount(const std::string& deviceName, int& channelCount);
    }


}




