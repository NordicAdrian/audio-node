#pragma once

#include <thread>
#include <pulse/pulseaudio.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>
#include <memory>



namespace nnl_audio
{

namespace pulse
{


    class LoopbackStream
    {

    public:
        LoopbackStream() = default;
        ~LoopbackStream() = default;

        int Start(const std::string& sourceName, const std::string& sinkName);
        int Stop();
        bool IsRunning() const { return m_isRunning; }


    private:

        void StartLoopbackCB(pa_context* c, void* userdata);
        void ReadLoopbackCB(pa_stream* s, size_t length, void* userdata);
        void SourceInfoCB(pa_context* c, const pa_source_info* info, int eol, void* userdata);
        void StopCB(int signum);


        std::unique_ptr<std::thread> m_thread;
        std::mutex m_mutex;
        std::condition_variable m_waitCondition;
        pa_mainloop* m_mainLoop = nullptr;
        pa_context* m_context = nullptr;
        pa_stream* m_recordStream = nullptr;
        pa_stream* m_playbackStream = nullptr;
        std::string m_sourceName;
        std::string m_sinkName;
        pa_sample_spec m_sampleSpec = { PA_SAMPLE_S16LE, 44100, 2 }; 
        std::atomic<bool> m_stopRequested = false;
        std::atomic<bool> m_isRunning = false;

    };

}

}