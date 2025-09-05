#pragma once

#include <thread>
#include <pulse/pulseaudio.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>

namespace nnl_audio 
{
namespace pulse 
{


    class LoopbackStream 
    {

    public:
        LoopbackStream();
        ~LoopbackStream();

        int Start(const std::string& sourceName, const std::string& sinkName);
        int Stop();
        bool IsRunning() const { return m_isRunning; }

    private:
        // PulseAudio callbacks
        static void StartLoopbackCB(pa_context* c, void* userdata);
        static void ReadLoopbackCB(pa_stream* s, size_t length, void* userdata);
        static void SourceInfoCB(pa_context* c, const pa_source_info* info, int eol, void* userdata);

        // Internal state
        pa_mainloop* m_mainLoop = nullptr;
        pa_context* m_context = nullptr;
        pa_stream* m_recordStream = nullptr;
        pa_stream* m_playbackStream = nullptr;

        pa_sample_spec m_sampleSpec { PA_SAMPLE_FLOAT32LE, 44100, 2 };
        std::string m_sourceName;
        std::string m_sinkName;

        std::thread m_thread;
        std::atomic<bool> m_stopRequested {false};
        std::atomic<bool> m_isRunning {false};
    };

} // namespace pulse
} // namespace nnl_audio