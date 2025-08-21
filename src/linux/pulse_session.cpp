#include <pulse_session.h>



nnl_audio::PulseSession::PulseSession()
{
    m_mainLoop = pa_mainloop_new();
    m_context = pa_context_new(pa_mainloop_get_api(m_mainLoop), "PulseSessionApp");

}

nnl_audio::PulseSession::~PulseSession()
{
    if (m_context)
    {
        pa_context_disconnect(m_context);
        pa_context_unref(m_context);
    }
    if (m_mainLoop)
    {
        pa_mainloop_free(m_mainLoop);
    }
}


int nnl_audio::PulseSession::Initialize()
{
    pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
    while (true) 
    {
    pa_mainloop_iterate(m_mainLoop, 1, nullptr);
    pa_context_state_t state = pa_context_get_state(m_context);
        if (state == PA_CONTEXT_READY) break;
        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
            pa_context_unref(m_context);
            pa_mainloop_free(m_mainLoop);
            return -1;
        }
    }

    pa_operation* op = pa_context_get_server_info(m_context, 
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
        pa_mainloop_iterate(m_mainLoop, 1, nullptr);
    }
    return 0;
}

int nnl_audio::PulseSession::SetVolume(float volume)
{
    if (!m_context)
    {
        return -1;
    }
    pa_cvolume cvol;
    pa_cvolume_set(&cvol, 2, pa_sw_volume_from_linear(volume)); // 2 = stereo channels
    pa_context_set_sink_volume_by_name(m_context, m_dev.c_str(), &cvol, nullptr, nullptr);
    return 0;
}