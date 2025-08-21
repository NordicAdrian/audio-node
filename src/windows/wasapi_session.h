#pragma once


#include <thread>
#include <mmdeviceapi.h> // Include for IMMDevice
#include <audiopolicy.h> // Include for IAudioSessionEvents
#include <mutex>
#include <condition_variable>
#include <winrt/base.h>
#include <audio_session_manager.h>
#include <audio_session_control.h>
#include <device_manager.h>
#include <map>

namespace nnl_audio
{



class WASAPISession
{
public:
    WASAPISession(IMMDevice* device, float volume = 0.5f) : m_volume(volume) {
        m_device.attach(device);
    }
    winrt::hresult Initialize();
    winrt::hresult SetVolume(float volume);

private:

    winrt::com_ptr<IMMDevice> m_device;
    std::unique_ptr<AudioSessionManager> m_audioSessionManager;
    std::unordered_map<LPWSTR, std::unique_ptr<AudioSessionControl>> m_audioSessionControls;


    winrt::hresult SetVolume(IAudioSessionControl* sessionControl, float volume);
    winrt::hresult FetchAudioControls();
    winrt::hresult AddSessionControl(IAudioSessionControl* sessionControl);

    float m_volume;


};
}