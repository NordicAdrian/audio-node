#include <nnl_audio.h>
#include <pulse_session.h>
#include <string>


std::unique_ptr<nnl_audio::PulseSession> audioSession;


int nnl_audio::InitializeAudioSession(const std::string& deviceId)
{
    InitializeAudioSession();
    return 0;
}

int nnl_audio::InitializeAudioSession()
{
    audioSession = std::make_unique<nnl_audio::PulseSession>();
    audioSession->Initialize();
    return 0;
}



int nnl_audio::SetSessionVolume(float volume)
{
    if (!audioSession) return -1;
    return audioSession->SetVolume(volume);
}




