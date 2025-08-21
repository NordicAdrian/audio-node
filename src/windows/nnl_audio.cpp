#include <nnl_audio.h>
#include <winerror.h> 
#include <wasapi_session.h>
#include <device_manager.h>

#include <memory>
#include <winrt/base.h>

std::unique_ptr<nnl_audio::WASAPISession> audioSession;
std::unique_ptr<nnl_audio::DeviceManager> deviceManager;



int nnl_audio::InitializeAudioSession(const std::string& deviceId)
{
    winrt::hresult hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    deviceManager = std::make_unique<nnl_audio::DeviceManager>();
    hr = deviceManager->Initialize();
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize device manager: " << std::endl;
        return -1;
    }

    IMMDevice* device = deviceManager->GetDevice(deviceId);
    if (!device)
    {
        std::cout << "Failed to get device: " << deviceId << std::endl;
        std::cout << "Using default device." << std::endl;
        device = deviceManager->GetDefaultOutputDevice();
    }

    audioSession = std::make_unique<nnl_audio::WASAPISession>(device);
    hr = audioSession->Initialize();
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize audio session: " << std::endl;
        return -1;
    }
    return 0;
}

int nnl_audio::InitializeAudioSession()
{
    winrt::hresult hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    deviceManager = std::make_unique<nnl_audio::DeviceManager>();
    hr = deviceManager->Initialize();
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize device manager: " << std::endl;
        return -1;
    }

    IMMDevice* device = deviceManager->GetDefaultOutputDevice();
    audioSession = std::make_unique<nnl_audio::WASAPISession>(device);
    hr = audioSession->Initialize();
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize audio session: " << std::endl;
        return -1;
    }
    return 0;
}

int nnl_audio::SetSessionVolume(float volume)
{
    if (audioSession)
    {
        winrt::hresult hr = audioSession->SetVolume(volume);
        if (SUCCEEDED(hr))
        {
            return 0; 
        }
    }
    std::cerr << "Failed to set session volume." << std::endl;
    return -1; 
}
