#include <audio_session.h>
#include <audioclient.h> // Required for IAudioClient
#include <iostream>





winrt::hresult wasapi::AudioSession::Initialize()
{
    winrt::hresult hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    
    winrt::com_ptr<IMMDeviceEnumerator> deviceEnumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(deviceEnumerator.put()));
    if (FAILED(hr))
    {
        std::cout << "Failed to create MMDeviceEnumerator instance: " << std::endl;
        return hr;
    }

    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, m_device.put());
    if (FAILED(hr))
    {
        std::cout << "Failed to get default audio endpoint: " << std::endl;
        return hr;
    }
    winrt::com_ptr<IAudioSessionManager2> sessionManager;
    hr = m_device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, NULL, sessionManager.put_void());
    if (FAILED(hr))
    {
        std::cout << "Failed to activate IAudioSessionManager: " << std::endl;
        return hr;
    }
    m_audioSessionManager = std::make_unique<AudioSessionManager>(sessionManager.get(), 
        [this] (IAudioSessionControl* sessionControl) 
        {
            winrt::hresult hr = AddSessionControl(sessionControl);
            if (FAILED(hr))
            {
                std::cout << "Failed to add session control: " << std::endl;
            }
    });


    FetchAudioControls();
    SetVolume(m_volume);

    return S_OK;
}

winrt::hresult wasapi::AudioSession::SetVolume(float volume)
{
    for (const auto& [displayName, sessionControl] : m_audioSessionControls)
    {
        SetVolume(sessionControl->GetAudioSessionControl(), volume);
    }
    m_volume = volume;
    return S_OK;
}

winrt::hresult wasapi::AudioSession::SetVolume(IAudioSessionControl* sessionControl, float volume)
{
    winrt::hresult hr;
    winrt::com_ptr<IAudioSessionControl2> sessionControl2;
    hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), sessionControl2.put_void());
    if (FAILED(hr))
    {
        std::cout << "Failed to get IAudioSessionControl2: " << std::endl;
        return hr;
    }
    winrt::com_ptr<ISimpleAudioVolume> audioVolume;
    hr = sessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), audioVolume.put_void());
    if (FAILED(hr))
    {
        std::cout << "Failed to get ISimpleAudioVolume: " << std::endl;
        return hr;
    }
    hr = audioVolume->SetMasterVolume(volume, NULL);
    if (FAILED(hr))
    {
        std::cout << "Failed to set master volume: " << std::endl;
        return hr;
    }
    return S_OK;
}

winrt::hresult wasapi::AudioSession::FetchAudioControls()
{
    winrt::hresult hr;
    IAudioSessionManager2* sessionManager = m_audioSessionManager->GetAudioSessionManager();
    winrt::com_ptr<IAudioSessionEnumerator> sessionEnumerator;
    hr = sessionManager->GetSessionEnumerator(sessionEnumerator.put());
    if (FAILED(hr))
    {
        std::cout << "Failed to get IAudioSessionEnumerator: " << std::endl;
        return hr;
    }
    int sessionCount;
    hr = sessionEnumerator->GetCount(&sessionCount);
    if (FAILED(hr))
    {
        std::cout << "Failed to get session count: " << std::endl;
        return hr;
    }

    for (int i = 0; i < sessionCount; ++i)
    {
        winrt::com_ptr<IAudioSessionControl> sessionControl;
        hr = sessionEnumerator->GetSession(i, sessionControl.put());
        if (FAILED(hr))
        {
            std::cout << "Failed to get audio session: " << std::endl;
            continue;
        }
        hr = AddSessionControl(sessionControl.get());
        if (FAILED(hr))
        {
            std::cout << "Failed to add session control: " << std::endl;
            continue;
        }
    }
    return S_OK;
}

winrt::hresult wasapi::AudioSession::AddSessionControl(IAudioSessionControl *sessionControl)
{
    winrt::hresult hr;
    LPWSTR displayName;
    hr = sessionControl->GetDisplayName(&displayName);
    if (FAILED(hr))
    {
        std::cout << "Failed to get display name: " << std::endl;
        return hr;
    }

    auto audioSessionControl = std::make_unique<AudioSessionControl>(sessionControl,
        [this, displayName]
        {
            if (m_audioSessionControls.find(displayName) != m_audioSessionControls.end())
            {
                std::cout << "Audio session disconnected: " << displayName << std::endl;
                m_audioSessionControls.erase(displayName);
            }
        },
        [this] (float newVolume)
        {

        });
    hr = SetVolume(audioSessionControl->GetAudioSessionControl(), m_volume);
    m_audioSessionControls[displayName] = std::move(audioSessionControl);
    std::cout << "Audio session added: " << displayName << std::endl;
    return S_OK;

}
