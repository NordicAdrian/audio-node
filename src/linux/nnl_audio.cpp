#include <nnl_audio.h>
#include <pulse/pulseaudio.h>
#include <string>


std::string dev;


int nnl_audio::InitializeAudioSession(const std::string& deviceId)
{
    dev = deviceId;
    return 0;
}

int nnl_audio::InitializeAudioSession()
{
    return InitializeAudioSession("");
}

int nnl_audio::SetSessionVolume(float volume)
{
    pa_mainloop* mainloop = pa_mainloop_new();
    pa_context* context = pa_context_new(pa_mainloop_get_api(mainloop), "SetVolumeApp");
    pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    while (true) 
    {
        pa_mainloop_iterate(mainloop, 1, nullptr);
        pa_context_state_t state = pa_context_get_state(context);
        if (state == PA_CONTEXT_READY) break;
        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) 
        {
            pa_context_unref(context);
            pa_mainloop_free(mainloop);
            return -1;
        }
    }


    pa_context_set_sink_volume_by_name(context, dev.c_str(), pa_sw_volume_from_linear(volume), nullptr, nullptr);


    pa_context_disconnect(context);
    pa_context_unref(context);
    pa_mainloop_free(mainloop);

    return 0;
}




