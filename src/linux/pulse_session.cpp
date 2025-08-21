#include <pulse_session.h>
#include <iostream>


nnl_audio::PulseSession::PulseSession()
{

   
}

nnl_audio::PulseSession::~PulseSession()
{
    if (m_context) {
        pa_context_disconnect(m_context);
        pa_context_unref(m_context);
        m_context = nullptr;
    }
    if (m_mainLoop) {
        pa_mainloop_free(m_mainLoop);
        m_mainLoop = nullptr;
    }
}


int nnl_audio::PulseSession::Initialize()
{
    m_mainLoop = pa_mainloop_new();
    if (!m_mainLoop) {
        std::cerr << "Failed to create PulseAudio mainloop." << std::endl;
        m_context = nullptr;
        return;
    }
    m_context = pa_context_new(pa_mainloop_get_api(m_mainLoop), "PulseSessionApp");
    if (!m_context) {
        std::cerr << "Failed to create PulseAudio context." << std::endl;
        pa_mainloop_free(m_mainLoop);
        m_mainLoop = nullptr;
    }
    if (!m_context || !m_mainLoop) return -1;
    if (pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
        std::cerr << "Failed to connect PulseAudio context." << std::endl;
        return -1;
    }
    int elapsed = 0;
    while (true) 
    {
        pa_mainloop_iterate(m_mainLoop, 1, nullptr);
        pa_context_state_t state = pa_context_get_state(m_context);
        if (state == PA_CONTEXT_READY) break;
        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
            pa_context_unref(m_context);
            m_context = nullptr;
            pa_mainloop_free(m_mainLoop);
            m_mainLoop = nullptr;
            return -1;
        }
        elapsed += 10;
        if (elapsed > TIMEOUT_MS) {
            std::cerr << "PulseAudio context connect timed out." << std::endl;
            return -1;
        }
        sleep(10);
    }

    pa_operation* op = pa_context_get_server_info(m_context, 
        [] (pa_context* c, const pa_server_info* info, void* userdata) {
            if (info && info->default_sink_name) {
                std::string* sinkName = static_cast<std::string*>(userdata);
                *sinkName = info->default_sink_name;
            }
        }, 
        &m_dev 
    );
    elapsed = 0;
    while (m_dev.empty()) {
        pa_mainloop_iterate(m_mainLoop, 1, nullptr);
        elapsed += 10;
        if (elapsed > TIMEOUT_MS) {
            std::cerr << "PulseAudio get_server_info timed out." << std::endl;
            return -1;
        }
        sleep(10);
    }
    pa_operation_unref(op);
    pa_operation* infoOp = pa_context_get_sink_info_by_name(m_context, m_dev.c_str(),
        [](pa_context* c, const pa_sink_info* info, int eol, void* userdata) {
            if (eol > 0 || !info) return;
            int* channelCount = static_cast<int*>(userdata);
            *channelCount = info->channel_map.channels;
        },
        &m_channelCount
    );
    elapsed = 0;
    while (pa_operation_get_state(infoOp) == PA_OPERATION_RUNNING) {
        pa_mainloop_iterate(m_mainLoop, 1, nullptr);
        elapsed += 10;
        if (elapsed > TIMEOUT_MS) {
            std::cerr << "PulseAudio get_sink_info_by_name timed out." << std::endl;
            pa_operation_unref(infoOp);
            return -1;
        }
        sleep(10);
    }
    pa_operation_unref(infoOp);
    return 0;
}

int nnl_audio::PulseSession::SetVolume(float volume)
{

    if (!m_context || m_dev.empty()) {
        std::cerr << "PulseAudio context or sink name not initialized." << std::endl;
        return -1;
    }
    if (pa_context_get_state(m_context) != PA_CONTEXT_READY) {
        std::cerr << "PulseAudio context not ready." << std::endl;
        return -1;
    }
    pa_cvolume cvol;
    pa_cvolume_set(&cvol, m_channelCount, pa_sw_volume_from_linear(volume));
    pa_operation* volOp = pa_context_set_sink_volume_by_name(m_context, m_dev.c_str(), &cvol, nullptr, nullptr);
    int elapsed = 0;
    while (pa_operation_get_state(volOp) == PA_OPERATION_RUNNING) 
    {
        pa_mainloop_iterate(m_mainLoop, 1, nullptr);
        elapsed += 10;
        if (elapsed > TIMEOUT_MS) {
            std::cerr << "PulseAudio set_sink_volume_by_name timed out." << std::endl;
            pa_operation_unref(volOp);
            return -1;
        }
        sleep(10);
    }
    pa_operation_unref(volOp);
    return 0;
}