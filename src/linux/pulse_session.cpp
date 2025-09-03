#include <pulse_session.h>
#include <iostream>
#include <vector>




int nnl_audio::pulse::SetEndpointVolume(const std::string& endPointName, float volume)
{
    pa_context* context;
    pa_mainloop* mainLoop;
    if (CreateContext(&context, &mainLoop) != 0)
     {
        std::cerr << "Failed to create PulseAudio context." << std::endl;
        return -1;
    }

    int channelCount;
    if (GetChannelCount(endPointName, channelCount) != 0) 
    {
        std::cerr << "Failed to get channel count." << std::endl;
        return -1;
    }

    pa_cvolume cvol;
    pa_cvolume_set(&cvol, channelCount, pa_sw_volume_from_linear(volume));
    pa_operation* volOp = pa_context_set_sink_volume_by_name(context, endPointName.c_str(), &cvol, nullptr, nullptr);
    if (EnsureOperation(volOp, mainLoop) != 0) 
    {
        std::cerr << "Failed to set sink volume." << std::endl;
        return -1;
    }

    pa_context_disconnect(context);
    pa_context_unref(context);
    pa_operation_unref(volOp);
    pa_mainloop_free(mainLoop);
    return 0;
}


int nnl_audio::pulse::GetConnectedOutputDevices(std::vector<std::string>& deviceNames)
{
    pa_context* context;
    pa_mainloop* mainLoop;
    if (CreateContext(&context, &mainLoop) != 0)
    {
        std::cerr << "Failed to create PulseAudio context." << std::endl;
        return -1;
    }

    struct GetSinkNamesData {
        std::vector<std::string>* deviceNames;
        bool done;
    } data = { &deviceNames, false };

    auto sinkInfoCallback = [](pa_context* c, const pa_sink_info* info, int eol, void* userdata) {
        GetSinkNamesData* data = static_cast<GetSinkNamesData*>(userdata);
        if (eol > 0) {
            data->done = true;
            return;
        }
        if (info) {
            data->deviceNames->emplace_back(info->name);
        }
    };

    pa_operation* op = pa_context_get_sink_info_list(context, sinkInfoCallback, &data);
    if (EnsureOperation(op, mainLoop) != 0) 
    {
        std::cerr << "Failed to get sink info list." << std::endl;
        return -1;
    }
    pa_operation_unref(op);

    pa_context_disconnect(context);
    pa_mainloop_free(mainLoop);
    pa_context_unref(context);
    return 0;
}



int nnl_audio::pulse::StartLoopbackStream(std::string& sourceName, const std::string& sinkName)
{
    pa_context* context;
    pa_mainloop* mainLoop;
    if (CreateContext(&context, &mainLoop) != 0)
    {
        std::cerr << "Failed to create PulseAudio context." << std::endl;
        return -1;
    }

    pa_context_set_state_callback(context, StartLoopbackCB, &sourceName);
    int retVal;
    if (pa_mainloop_run(mainLoop, &retVal) < 0) 
    {
        std::cerr << "Failed to run main loop." << std::endl;
        return -1;
    }

    pa_context_disconnect(context);
    pa_mainloop_free(mainLoop);
    pa_context_unref(context);

}

int nnl_audio::pulse::StopLoopbackStream()
{
    if (recordStream) 
    {
        pa_stream_disconnect(recordStream);
        pa_stream_unref(recordStream);
        recordStream = nullptr;
    }
    if (playbackStream) 
    {
        pa_stream_disconnect(playbackStream);
        pa_stream_unref(playbackStream);
        playbackStream = nullptr;
    }
    return 0;
}

int nnl_audio::pulse::EnsureOperation(pa_operation *op, pa_mainloop *mainLoop)
{
    if (!op) return -1;

    int elapsed = 0;
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) 
    {
        pa_mainloop_iterate(mainLoop, 1, nullptr);
        elapsed += 10;
        if (elapsed > TIMEOUT_MS) 
        {
            std::cerr << "PulseAudio operation timed out." << std::endl;
            pa_operation_unref(op);
            return -1;
        }
        usleep(10);
    }
    return 0;
}

int nnl_audio::pulse::CreateContext(pa_context **c, pa_mainloop **m)
{
    *m = pa_mainloop_new();
    if (!*m) return -1;

    *c = pa_context_new(pa_mainloop_get_api(*m), "PulseAudio");
    if (!*c) {
        pa_mainloop_free(*m);
        return -1;
    }
    pa_context_connect(*c, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
    while (true)
    {
        pa_context_state_t state = pa_context_get_state(*c);
        if (state == PA_CONTEXT_READY) break;
        if (!PA_CONTEXT_IS_GOOD(state)) {
            std::cerr << "Connection to PulseAudio failed.\n";
            return 1;
        }
        pa_mainloop_iterate(*m, 1, nullptr);
    }
    return 0;
}

void nnl_audio::pulse::StartLoopbackCB(pa_context *c, void *userdata)
{
    pa_context_state_t state = pa_context_get_state(c);
    if (state != PA_CONTEXT_READY) 
    {
        std::cerr << "PulseAudio context is not ready." << std::endl;
        return;
    }

    pa_sample_spec ss;
    pa_operation* op = pa_context_get_source_info_by_name(c, static_cast<const char*>(userdata), SourceInfoCB, &ss);
    pa_mainloop* mainLoop = pa_mainloop_new();
    if (!mainLoop) {
        std::cerr << "Failed to create main loop." << std::endl;
        return;
    }
    if (EnsureOperation(op, mainLoop) != 0) 
    {
        std::cerr << "Failed to get source info." << std::endl;
        return;
    }

    recordStream = pa_stream_new(c, "loopback-record", &ss, nullptr);
    playbackStream = pa_stream_new(c, "loopback-playback", &ss, nullptr);

    pa_stream_set_read_callback(recordStream, ReadCB, nullptr);

    if (pa_stream_connect_record(recordStream, sourceName.c_str(), nullptr, PA_STREAM_ADJUST_LATENCY) < 0)
    {
        std::cerr << "Failed to connect record stream." << std::endl;
        return;
    }
    if (pa_stream_connect_playback(playbackStream, sinkName.c_str(), nullptr, PA_STREAM_ADJUST_LATENCY) < 0)
    {
        std::cerr << "Failed to connect playback stream." << std::endl;
        return;
    }
}

void nnl_audio::pulse::ReadLoopbackCB(pa_stream *s, size_t length, void *userdata)
{
    const void *data;
    if (pa_stream_peek(s, &data, &length) < 0) {
        std::cerr << "Failed to read data from source stream." << std::endl;
        return;
    }
    if (data && length > 0 && playbackStream) 
    {
        pa_stream_write(playbackStream, data, length, nullptr, 0, PA_SEEK_RELATIVE);
    }
    pa_stream_drop(s);
}

void nnl_audio::pulse::SourceInfoCB(pa_context *c, const pa_source_info *info, int eol, void *userdata)
{
    if (eol >0 || !info) return;
    *pa_sample_spec specOut = static_cast<pa_sample_spec*>(userdata);
    *specOut = info->sample_spec;   
}

int nnl_audio::pulse::GetChannelCount(const std::string& deviceName, int& channelCount)
{
    pa_context* context;
    pa_mainloop* mainLoop;
    if (CreateContext(&context, &mainLoop) != 0)
    {
        std::cerr << "Failed to create PulseAudio context." << std::endl;
        return -1;
    }

    struct GetSinkInfoData {
        int channelCount;
        bool done;
    } data = {0, false};

    auto sinkInfoCallback = [](pa_context* c, const pa_sink_info* info, int eol, void* userdata) {
        GetSinkInfoData* data = static_cast<GetSinkInfoData*>(userdata);
        if (eol > 0) {
            data->done = true;
            return;
        }
        if (info) {
            data->channelCount = info->channel_map.channels;
        }
    };

    pa_operation* op = pa_context_get_sink_info_by_name(context, deviceName.c_str(), sinkInfoCallback, &data);
    if (EnsureOperation(op, mainLoop) != 0) 
    {
        std::cerr << "Failed to get sink info." << std::endl;
        return -1;
    }
    pa_operation_unref(op);

    if (data.channelCount == 0) 
    {
        std::cerr << "Failed to get channel count for device: " << deviceName << std::endl;
        return -1;
    }

    channelCount = data.channelCount;
    pa_context_disconnect(context);
    pa_mainloop_free(mainLoop);
    pa_context_unref(context);
    return 0;
}

