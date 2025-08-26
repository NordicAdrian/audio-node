#include <nnl_audio.h>
#include <winerror.h> 
#include <wasapi_session.h>

#include <memory>
#include <winrt/base.h>

std::unique_ptr<nnl_audio::WASAPISession> audioSession;


int nnl_audio::InitializeAudioSession(const std::string& deviceId)
{
    winrt::hresult hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    audioSession = std::make_unique<nnl_audio::WASAPISession>();
    hr = audioSession->Initialize(deviceId);
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize audio session: " << std::endl;
        return -1;
    }
    return 0;
}

int nnl_audio::InitializeAudioSession()
{
    return InitializeAudioSession("");
}

int nnl_audio::SetSessionVolume(float volume)
{
    if (audioSession)
    {
        winrt::hresult hr = audioSession->SetAllSessionVolumes(volume);
        if (SUCCEEDED(hr))
        {
            return 0; 
        }
    }
    std::cerr << "Failed to set session volume." << std::endl;
    return -1; 
}


std::vector<std::string> nnl_audio::GetConnectedOutputDevices()
{
    if (!audioSession)
    {
        std::cerr << "Audio session not initialized." << std::endl;
        return {};
    }
    std::vector<winrt::com_ptr<IMMDevice>> connectedDevices = audioSession->GetConnectedDevices(eRender);
    std::vector<std::string> deviceNames;
    for (const auto& device : connectedDevices)
    {
        std::string name;
        winrt::hresult hr = audioSession->GetDeviceName(device.get(), name);
        if (SUCCEEDED(hr))
        {
            deviceNames.push_back(name);
        }
    }
    return deviceNames;
}

int nnl_audio::StartLoopbackStream(const std::string &sink)
{
    if (audioSession)
    {
        winrt::hresult hr = audioSession->StartLoopbackStream(sink);
        if (SUCCEEDED(hr))
        {
            return 0;
        }
    }
    std::cerr << "Failed to start loopback stream." << std::endl;
    return -1;
}


int nnl_audio::StopLoopbackStream()
{
    if (audioSession)
    {
        winrt::hresult hr = audioSession->StopLoopbackStream();
        if (SUCCEEDED(hr))
        {
            return 0;
        }
    }
    std::cerr << "Failed to stop loopback stream." << std::endl;
    return -1;
}
