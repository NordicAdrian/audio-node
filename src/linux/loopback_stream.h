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

        static void StartLoopbackCB(pa_context* c, void* userdata);
        static void ReadLoopbackCB(pa_stream* s, size_t length, void* userdata);
        static void SourceInfoCB(pa_context* c, const pa_source_info* info, int eol, void* userdata);
        static void StopCB(int signum);


        std::unique_ptr<std::thread> m_thread;
        std::mutex m_mutex;
        std::condition_variable m_waitCondition;
        static pa_mainloop* m_mainLoop;
        static pa_context* m_context;
        static pa_stream* m_recordStream;
        static pa_stream* m_playbackStream;
        static std::string m_sourceName;
        static std::string m_sinkName;
        static pa_sample_spec m_sampleSpec;
        std::atomic<bool> m_stopRequested = false;
        std::atomic<bool> m_isRunning = false;

    };

}

}