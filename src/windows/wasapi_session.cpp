#include <wasapi_session.h>
#include <audioclient.h>
#include <iostream>






winrt::hresult nnl_audio::WASAPISession::Initialize(const std::string &name)
{
    winrt::hresult hr;
    if (!m_isInitialized)
    {
        hr = InitializeNotificationClient();
        if (FAILED(hr))
        {
            std::cout << "Failed to initialize notification client: " << std::endl;
            return hr;
        }
        m_isInitialized = true;
    }
    return name.empty() ? InitializeDefault() : InitializeByName(name);
}


winrt::hresult nnl_audio::WASAPISession::InitializeDefault()
{
    winrt::hresult hr = m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, m_sink.put());
    if (FAILED(hr))
    {
        std::cout << "Failed to get default audio endpoint: " << std::endl;
        return hr;
    }
    hr = InitializeSink(m_sink);
    if (FAILED(hr))
    {
        std::cout << "Failed to initialize audio endpoint: " << std::endl;
        return hr;
    }
    m_useDefaultSink = true;
    return S_OK;  
}

winrt::hresult nnl_audio::WASAPISession::InitializeByName(const std::string &deviceName)
{
    winrt::hresult hr = GetDeviceByName(deviceName, m_sink);
    if (FAILED(hr))
    {
        std::cout << "Failed to get audio endpoint by name: " << deviceName << std::endl;
        return hr;
    }
    hr = InitializeSink(m_sink);
    if (FAILED(hr))
    {
        std::cout << "Failed to initialize audio endpoint: " << std::endl;
        return hr;
    }
    m_useDefaultSink = false;
    return S_OK;
}

winrt::hresult nnl_audio::WASAPISession::StartLoopbackStream(const std::string &deviceName)
{
    if (!IsInitialized())
    {
        std::cout << "WASAPISession not initialized." << std::endl;
        return E_FAIL;
    }
    m_loopbackStream = std::make_unique<LoopbackStream>();
    winrt::hresult hr = GetDeviceByName(deviceName, m_loopBackSink);
    if (FAILED(hr))
    {
        std::cout << "Failed to get loopback sink: " << deviceName << std::endl;
        return hr;
    }
    hr = GetDeviceID(m_loopBackSink.get(), m_loopBackSinkID);
    if (FAILED(hr))
    {
        std::cout << "Failed to get loopback sink ID: " << std::endl;
        return hr;
    }
    if (wcscmp(m_loopBackSinkID, m_sinkID) == 0)
    {
        std::cout << "Loopback sink cannot be the same as the output sink." << std::endl;
        return E_FAIL;
    }
    hr = m_loopbackStream->Start(m_sink.get(), m_loopBackSink.get());
    if (FAILED(hr))
    {
        std::cout << "Failed to initialize loopback stream: " << std::endl;
        return hr;
    }
    return S_OK;
}

winrt::hresult nnl_audio::WASAPISession::StopLoopbackStream()
{
    if (m_loopbackStream && m_loopbackStream->IsRunning())
    {
        return m_loopbackStream->Stop();
    }
    return S_OK;
}

winrt::hresult nnl_audio::WASAPISession::SetAllSessionVolumes(float volume)
{
    for (const auto& [displayName, sessionControl] : m_audioSessionControls)
    {
        SetSessionVolume(sessionControl->GetAudioSessionControl(), volume);
    }
    m_volume = volume;
    return S_OK;
}

winrt::hresult nnl_audio::WASAPISession::SetSessionVolume(IAudioSessionControl* sessionControl, float volume)
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

winrt::hresult nnl_audio::WASAPISession::FetchAudioControls()
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
            std::cout << "Failed to get nnl_audio session: " << std::endl;
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

winrt::hresult nnl_audio::WASAPISession::AddSessionControl(IAudioSessionControl *sessionControl)
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
                std::cout << "nnl_audio session disconnected: " << displayName << std::endl;
                m_audioSessionControls.erase(displayName);
            }
        },
        [this] (float newVolume)
        {

        });
    hr = SetSessionVolume(audioSessionControl->GetAudioSessionControl(), m_volume);
    m_audioSessionControls[displayName] = std::move(audioSessionControl);
    std::cout << "nnl_audio session added: " << displayName << std::endl;
    return S_OK;

}

winrt::hresult nnl_audio::WASAPISession::QueryHardwareSupport(bool& hardwareSupport)
{
    winrt::hresult hr;
    DWORD support;
    hr = m_endpointVolume->QueryHardwareSupport(&support);
    if (support & ENDPOINT_HARDWARE_SUPPORT_VOLUME)
    {
        hardwareSupport = true;
    }
    else
    {
        hardwareSupport = false;
    }
    return hr;
}

winrt::hresult nnl_audio::WASAPISession::InitializeSink(winrt::com_ptr<IMMDevice> sink)
{
    m_sink = std::move(sink);
    winrt::hresult hr = GetDeviceID(m_sink.get(), m_sinkID);
    if (FAILED(hr))
    {
        std::cout << "Failed to get device ID: " << std::endl;
        return hr;
    }
    winrt::com_ptr<IAudioSessionManager2> sessionManager;
    hr = m_sink->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, NULL, sessionManager.put_void());
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
    hr = m_sink->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, m_endpointVolume.put_void());
    if (FAILED(hr))
    {
        std::cout << "Failed to activate IAudioEndpointVolume: " << std::endl;
        return hr;
    }
    hr = QueryHardwareSupport(m_hardwareSupport);
    if (FAILED(hr))
    {
        std::cout << "Failed to query hardware support: " << std::endl;
        return hr;
    }
    std::cout << "Hardware support: " << (m_hardwareSupport ? "Yes" : "No") << std::endl;
    hr = FetchAudioControls();
    if (FAILED(hr))
    {
        std::cout << "Failed to fetch audio controls: " << std::endl;
        return hr;
    }
    hr = SetAllSessionVolumes(m_volume);
    if (FAILED(hr))
    {
        std::cout << "Failed to set all session volumes: " << std::endl;
        return hr;
    }
    return S_OK;
}


STDMETHODIMP_(HRESULT __stdcall)
nnl_audio::WASAPISession::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    winrt::hresult hr;
    if (pwstrDeviceId == nullptr)
    {
        std::cout << "Device added with invalid ID." << std::endl;
        return S_OK;
    }
    if (wcscmp(pwstrDeviceId, m_sinkID) == 0)
    {
        std::cout << "Device added but was already connected: " << pwstrDeviceId << std::endl;
        return S_OK;
    }
    std::cout << "New session device added: " << pwstrDeviceId << std::endl;
    if (m_useDefaultSink)
    {
        hr = InitializeDefault();
        if (FAILED(hr))
        {
            std::cout << "Failed to switch to new default device: " << pwstrDeviceId << std::endl;
            return hr;
        }
    }
}

STDMETHODIMP_(HRESULT __stdcall)
nnl_audio::WASAPISession::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    winrt::hresult hr;
    if (wcscmp(pwstrDeviceId, m_sinkID) == 0)
    {
        std::cout << "Session device removed, changing to default device." << pwstrDeviceId << std::endl;
        hr = InitializeDefault();
        if (FAILED(hr))
        {
            std::cout << "Failed to switch to default device: " << pwstrDeviceId << std::endl;
            return hr;
        }
    }
    if (wcscmp(pwstrDeviceId, m_loopBackSinkID) == 0)
    {
        std::cout << "Loopback device removed: " << pwstrDeviceId << std::endl;
        StopLoopbackStream();
    }
    std::cout << "Device removed, not related to current session: " << pwstrDeviceId << std::endl;
    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall)
nnl_audio::WASAPISession::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    winrt::hresult hr;
    if (wcscmp(pwstrDeviceId, m_sinkID) == 0)
    {
        if (dwNewState == DEVICE_STATE_ACTIVE)
        {
            std::cout << "Session device state changed: " << pwstrDeviceId << ", New State: Active" << std::endl;
        }
        else
        {
            std::cout << "Session device state changed: " << pwstrDeviceId << ", New State: Inactive" << std::endl;
            hr = StopLoopbackStream();
        }
        return S_OK;
    }
    if (wcscmp(pwstrDeviceId, m_loopBackSinkID) == 0)
    {
        if (dwNewState == DEVICE_STATE_ACTIVE)
        {
            std::cout << "Loopback device state changed: " << pwstrDeviceId << ", New State: Active" << std::endl;
        }
        else
        {
            std::cout << "Loopback device state changed: " << pwstrDeviceId << ", New State: Inactive" << std::endl;
            hr = StopLoopbackStream();
        }
        return S_OK;
    }
    std::cout << "Device state changed, not related to current session: " << pwstrDeviceId << std::endl;
    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall)
nnl_audio::WASAPISession::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    winrt::hresult hr;
    if (flow == eRender && role == eConsole && wcscmp(pwstrDefaultDeviceId, m_sinkID) != 0)
    {
        std::cout << "Default session device changed: " << pwstrDefaultDeviceId << std::endl;
        if (m_useDefaultSink)
        {
            hr = InitializeDefault();
            if (FAILED(hr))
            {
                std::cout << "Failed to switch to new default device: " << pwstrDefaultDeviceId << std::endl;
                return hr;
            }
            hr = StopLoopbackStream();
            if (FAILED(hr))
            {
                std::cout << "Failed to stop loopback stream: " << pwstrDefaultDeviceId << std::endl;
                return hr;
            }

        }
    }
    std::cout << "Default device changed but not related to current session: " << pwstrDefaultDeviceId << std::endl;
    return S_OK;
}
