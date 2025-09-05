#include "loopback_stream.h"
#include <pulse/pulseaudio.h>

using namespace nnl_audio::pulse;

LoopbackStream::LoopbackStream() = default;

LoopbackStream::~LoopbackStream() 
{
    Stop(); 
}

int LoopbackStream::Start(const std::string& sourceName, const std::string& sinkName) 
{
    if (m_isRunning.load()) 
    {
        std::cerr << "Loopback already running\n";
        return -1;
    }

    m_sourceName = sourceName + ".monitor"; // Use monitor source for loopback
    m_sinkName = sinkName;

    m_mainLoop = pa_mainloop_new();
    if (!m_mainLoop) {
        std::cerr << "Failed to create PulseAudio mainloop\n";
        return -1;
    }

    m_context = pa_context_new(pa_mainloop_get_api(m_mainLoop), "LoopbackStream");
    if (!m_context) {
        std::cerr << "Failed to create PulseAudio context\n";
        pa_mainloop_free(m_mainLoop);
        m_mainLoop = nullptr;
        return -1;
    }

    pa_context_set_state_callback(m_context, StartLoopbackCB, this);

    if (pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0)
    {
        std::cerr << "Failed to connect to PulseAudio\n";
        pa_context_unref(m_context);
        pa_mainloop_free(m_mainLoop);
        m_context = nullptr;
        m_mainLoop = nullptr;
        return -1;
    }

    // Run mainloop in background thread
    m_thread = std::thread([this]()
    {
        m_isRunning.store(true);
        int retval;
        pa_mainloop_run(m_mainLoop, &retval);
        m_isRunning.store(false);
    });

    std::cout << "Loopback started from " << m_sourceName << " to " << m_sinkName << "\n";
    return 0;
}

int LoopbackStream::Stop() 
{
    if (!m_isRunning.load()) return 0;

    m_stopRequested.store(true);

    if (m_mainLoop) 
    {
        pa_mainloop_quit(m_mainLoop, 0);
    }

    if (m_thread.joinable()) 
    {
        m_thread.join();
    }

    if (m_recordStream) 
    {
        pa_stream_disconnect(m_recordStream);
        pa_stream_unref(m_recordStream);
        m_recordStream = nullptr;
    }

    if (m_playbackStream) 
    {
        pa_stream_disconnect(m_playbackStream);
        pa_stream_unref(m_playbackStream);
        m_playbackStream = nullptr;
    }

    if (m_context) 
    {
        pa_context_disconnect(m_context);
        pa_context_unref(m_context);
        m_context = nullptr;
    }

    if (m_mainLoop) 
    {
        pa_mainloop_free(m_mainLoop);
        m_mainLoop = nullptr;
    }

    m_isRunning.store(false);
    std::cout << "Loopback stopped\n";
    return 0;
}



void LoopbackStream::StartLoopbackCB(pa_context* c, void* userdata) 
{
    auto* self = static_cast<LoopbackStream*>(userdata);
    if (!self) return;

    if (pa_context_get_state(c) != PA_CONTEXT_READY) return;

    // Fetch source sample spec
    pa_operation* op = pa_context_get_source_info_by_name(c, self->m_sourceName.c_str(), SourceInfoCB, &self->m_sampleSpec);
    if (op) pa_operation_unref(op);

    // Create streams
    self->m_recordStream = pa_stream_new(c, "loopback-record", &self->m_sampleSpec, nullptr);
    self->m_playbackStream = pa_stream_new(c, "loopback-playback", &self->m_sampleSpec, nullptr);

    if (!self->m_recordStream || !self->m_playbackStream) 
    {
        std::cerr << "Failed to create PulseAudio streams\n";
        return;
    }

    pa_stream_set_read_callback(self->m_recordStream, ReadLoopbackCB, self);

    if (pa_stream_connect_record(self->m_recordStream, self->m_sourceName.c_str(), nullptr, PA_STREAM_ADJUST_LATENCY) < 0) 
    {
        std::cerr << "Failed to connect record stream\n";
        return;
    }

    if (pa_stream_connect_playback(self->m_playbackStream, self->m_sinkName.c_str(), nullptr, PA_STREAM_ADJUST_LATENCY, nullptr, nullptr) < 0) 
    {
        std::cerr << "Failed to connect playback stream\n";
        return;
    }
}

void LoopbackStream::ReadLoopbackCB(pa_stream* s, size_t length, void* userdata) 
{
    auto* self = static_cast<LoopbackStream*>(userdata);
    if (!self || !self->m_playbackStream) return;

    const void* buffer;
    if (pa_stream_peek(s, &buffer, &length) < 0) 
    {
        std::cerr << "Failed to read from source stream\n";
        return;
    }

    if (buffer && length > 0) 
    {
        pa_stream_write(self->m_playbackStream, buffer, length,
                        nullptr, 0LL, PA_SEEK_RELATIVE);
    }
    pa_stream_drop(s);
}

void LoopbackStream::SourceInfoCB(pa_context*, const pa_source_info* info, int eol, void* userdata) 
{
    if (eol > 0 || !info) return;
    auto* specOut = static_cast<pa_sample_spec*>(userdata);
    *specOut = info->sample_spec;
}
