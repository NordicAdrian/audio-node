#include <nnl_audio.h>
#include <string>
#include <memory>
#include <pulse/pulseaudio.h>
#include <pulse_session.h>
#include <loopback_stream.h>



std::unique_ptr<nnl_audio::pulse::LoopbackStream> loopbackStream;

int nnl_audio::Initialize()
{
    loopbackStream = std::make_unique<nnl_audio::pulse::LoopbackStream>();
    return 0;
}

int nnl_audio::GetConnectedOutputDevices(std::vector<std::string>& deviceNames)
{
    return pulse::GetConnectedOutputDevices(deviceNames);
}



int nnl_audio::SetEndpointVolume(const std::string& endPointName, float volume)
{
    return pulse::SetEndpointVolume(endPointName, volume);
}


int nnl_audio::StartLoopbackStream(const std::string& sourceName, const std::string& sinkName)
{
    std::string src = sourceName;
    return loopbackStream->Start(src, sinkName);
}


int nnl_audio::StopLoopbackStream()
{
    return loopbackStream->Stop();
}






