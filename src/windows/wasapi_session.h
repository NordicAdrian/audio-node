#pragma once


#include <thread>
#include <mmdeviceapi.h> // Include for IMMDevice
#include <audiopolicy.h> // Include for IAudioSessionEvents
#include <endpointvolume.h> // Include for IAudioEndpointVolume
#include <mutex>
#include <condition_variable>
#include <winrt/base.h>
#include <audio_session_manager.h>
#include <audio_session_control.h>
#include <device_manager.h>
#include <loopback_stream.h>
#include <map>

namespace nnl_audio
{



class WASAPISession : public DeviceManager
{
public:
    WASAPISession(float volume = 0.5f) : DeviceManager(), m_volume(volume) {}
    WASAPISession(const WASAPISession&) = delete;
    WASAPISession& operator=(const WASAPISession&) = delete;
    WASAPISession(WASAPISession&&) = delete;
    WASAPISession& operator=(WASAPISession&&) = delete;
    ~WASAPISession()
    {

    }
    /*
        Initializes the WASAPI session using the specified device. If the device is empty,
        the default device will be used.
    */
    winrt::hresult Initialize(const std::string &name);

    winrt::hresult StartLoopbackStream(const std::string& deviceName);
    winrt::hresult StopLoopbackStream();
    winrt::hresult SetAllSessionVolumes(float volume);
    bool IsInitialized() const { return m_isInitialized; }
    bool IsLoopbackStreamRunning() const { return m_loopbackStream && m_loopbackStream->IsRunning(); }

private:
    winrt::com_ptr<IMMDevice> m_sink;
    winrt::com_ptr<IMMDevice> m_loopBackSink;
    std::unique_ptr<LoopbackStream> m_loopbackStream;
    winrt::com_ptr<IAudioEndpointVolume> m_endpointVolume;
    std::unique_ptr<AudioSessionManager> m_audioSessionManager;
    std::unordered_map<LPWSTR, std::unique_ptr<AudioSessionControl>> m_audioSessionControls;
    /*
        Initializes the WASAPI session using the default device.
    */
    winrt::hresult InitializeDefault();
    /*
        Initializes the WASAPI session using the specified device.
    */
    winrt::hresult InitializeByName(const std::string& deviceName);

    winrt::hresult SetSessionVolume(IAudioSessionControl* sessionControl, float volume);
    winrt::hresult FetchAudioControls();
    winrt::hresult AddSessionControl(IAudioSessionControl* sessionControl);
    winrt::hresult QueryHardwareSupport(bool& hardwareSupport);
    winrt::hresult InitializeSink(winrt::com_ptr<IMMDevice> sink);

    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;
    STDMETHOD(OnDeviceAdded)(LPCWSTR pwstrDeviceId) override;
    STDMETHOD(OnDeviceRemoved)(LPCWSTR pwstrDeviceId) override;
    STDMETHOD(OnDeviceStateChanged)(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;

    bool m_isInitialized = false;
    float m_volume;
    bool m_hardwareSupport = false;
    bool m_useDefaultSink = true;

    LPCWSTR m_sinkID;
    LPCWSTR m_loopBackSinkID;


};
}