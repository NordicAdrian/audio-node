#include <pulse_session.h>



nnl_audio::PulseSession::PulseSession()
{
    m_mainLoop = std::make_unique<pa_mainloop>();
    m_context = std::make_unique<pa_context>(pa_mainloop_get_api(m_mainLoop.get()), "PulseSessionApp");

}

nnl_audio::PulseSession::~PulseSession()
{
    if (m_context)
    {
        pa_context_disconnect(m_context.get());
    }
    if (m_mainLoop)
    {
        pa_mainloop_free(m_mainLoop.get());
    }
}


int nnl_audio::PulseSession::Initialize()
{
    pa_context_connect(m_context.get(), nullptr, PA_CONTEXT_NOFLAGS, nullptr);
    while (true) 
    {
        pa_mainloop_iterate(m_mainLoop.get(), 1, nullptr);
        pa_context_state_t state = pa_context_get_state(m_context.get());
        if (state == PA_CONTEXT_READY) break;
        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
            pa_context_unref(m_context.get());
            pa_mainloop_free(m_mainLoop.get());
            return -1;
        }
    }

    pa_operation* op = pa_context_get_server_info(m_context.get(), 
        [] (pa_context* c, const pa_server_info* info, void* userdata) {
            if (info && info->default_sink_name) 
            {
                std::string* sinkName = static_cast<std::string*>(userdata);
                *sinkName = info->default_sink_name;
            }
        }, 
        &m_dev 
    );

    while (m_dev.empty()) 
    {
        pa_mainloop_iterate(m_mainLoop.get(), 1, nullptr);
    }
    return 0;
}

int nnl_audio::PulseSession::SetVolume(float volume)
{
    if (!m_context)
    {
        return -1;
    }
    pa_context_set_sink_volume_by_name(m_context.get(), m_dev.c_str(), pa_sw_volume_from_linear(volume), nullptr, nullptr);
    return 0;
}