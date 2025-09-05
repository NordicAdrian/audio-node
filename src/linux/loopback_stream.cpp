#include <loopback_stream.h>
#include <pulse/pulseaudio.h>


int nnl_audio::pulse::LoopbackStream::Start(const std::string &sourceName, const std::string &sinkName)
{
    if (CreateContext(&m_context, &m_mainLoop) != 0)
    {
        std::cerr << "Failed to create PulseAudio context." << std::endl;
        return -1;
    }

    m_sourceName = sourceName;
    m_sinkName = sinkName;

    pa_context_set_state_callback(m_context, StartLoopbackCB, nullptr);
    int retVal;
    std::cout << "Starting loopback stream from " << sourceName << " to " << sinkName << std::endl;
    if (pa_mainloop_run(m_mainLoop, &retVal) < 0) 
    {
        std::cerr << "Failed to run main loop." << std::endl;
        return -1;
    }


    pa_context_set_state_callback(m_context, StartLoopbackCB, nullptr);
    int retVal;
    std::cout << "Starting loopback stream from " << m_sourceName << " to " << m_sinkName << std::endl;
    m_thread = std::make_unique<std::thread>([this, &retVal]() {
        m_isRunning = true;
        if (pa_mainloop_run(m_mainLoop, &retVal) < 0) 
        {
            std::cerr << "Failed to run main loop." << std::endl;
            return -1;
        }
    });

    pa_context_disconnect(m_context);
    pa_mainloop_free(m_mainLoop);
    pa_context_unref(m_context);
}

void nnl_audio::pulse::LoopbackStream::StartLoopbackCB(pa_context *c, void *userdata)
{
    pa_context_state_t state = pa_context_get_state(c);
    if (state != PA_CONTEXT_READY) 
    {
        std::cerr << "PulseAudio context is not ready." << std::endl;
        return;
    }

    pa_operation* op = pa_context_get_source_info_by_name(c, m_sourceName.c_str(), SourceInfoCB, &m_sampleSpec);
    if (EnsureOperation(op, m_mainLoop) != 0) 
    {
        std::cerr << "Failed to get source info." << std::endl;
        return;
    }

    m_recordStream = pa_stream_new(c, "loopback-record", &m_sampleSpec, nullptr);
    m_playbackStream = pa_stream_new(c, "loopback-playback", &m_sampleSpec, nullptr);

    pa_stream_set_read_callback(m_recordStream, ReadLoopbackCB, this);

    const char* sourceNameStr = m_sourceName.c_str();
    if (pa_stream_connect_record(m_recordStream, sourceNameStr, nullptr, PA_STREAM_ADJUST_LATENCY) < 0)
    {
        std::cerr << "Failed to connect record stream." << std::endl;
        return;
    }
    const char* sinkNameStr = m_sinkName.c_str();
    if (pa_stream_connect_playback(m_playbackStream, sinkNameStr, nullptr, PA_STREAM_ADJUST_LATENCY, nullptr, nullptr) < 0)
    {
        std::cerr << "Failed to connect playback stream." << std::endl;
        return;
    }
}

void nnl_audio::pulse::LoopbackStream::ReadLoopbackCB(pa_stream *s, size_t length, void *userdata)
{
    const void *data;
    if (pa_stream_peek(s, &data, &length) < 0) 
    {
        std::cerr << "Failed to read data from source stream." << std::endl;
        return;
    }
    if (data && length > 0 && m_playbackStream) 
    {
        pa_stream_write(m_playbackStream, data, length, nullptr, 0, PA_SEEK_RELATIVE);
    }
    pa_stream_drop(s);
}

void nnl_audio::pulse::LoopbackStream::SourceInfoCB(pa_context *c, const pa_source_info *info, int eol, void *userdata)
{
    if (eol >0 || !info) return;
    pa_sample_spec* specOut = static_cast<pa_sample_spec*>(userdata);
    *specOut = info->sample_spec;
}

void nnl_audio::pulse::LoopbackStream::StopCB(int signum)
{
    m_stopRequested.store(true);
}

int nnl_audio::pulse::LoopbackStream::Stop()
{
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
    return 0;
}



bool nnl_audio::LoopbackStream::IsRunning() const
{
    return m_isRunning;
}





