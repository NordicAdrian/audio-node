#include <nnl_audio.h>
#include <string>
#include <memory>
#include <pulse/pulseaudio.h>
#include <pulse_session.h>





int nnl_audio::Initialize()
{

}

int GetConnectedOutputDevices(std::vector<std::string>& deviceNames)
{
    return pulse::GetConnectedOutputDevices(deviceNames);
}



int nnl_audio::SetEndpointVolume(const std::string& endPointName, float volume)
{
    return pulse::SetEndpointVolume(endPointName, volume);
}


int nnl_audio::StartLoopbackStream(const std::string& sourceName, const std::string& sinkName)
{
    return pulse::StartLoopbackStream(sourceName, sinkName);
}


int nnl_audio::StopLoopbackStream()
{
    return pulse::StopLoopbackStream();
}






