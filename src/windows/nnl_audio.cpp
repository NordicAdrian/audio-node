#include <nnl_audio.h>
#include <winerror.h> 

#include <memory>
#include <winrt/base.h>
#include <device_manager.h>
#include <loopback_stream.h>


std::unique_ptr<nnl_audio::LoopbackStream> loopbackStream;
std::unique_ptr<nnl_audio::DeviceManager> deviceManager;

winrt::com_ptr<IMMDevice> sourceDevice;
winrt::com_ptr<IMMDevice> sinkDevice;

int nnl_audio::SetEndpointVolume(const std::string &endPointName, float volume)
{
    REQUIRE_INITIALIZED;
    winrt::hresult hr;
    winrt::com_ptr<IMMDevice> device;
    hr = deviceManager->GetDeviceByName(endPointName, device);
    if (FAILED(hr))
    {
        std::cerr << "Failed to get device by name: " << endPointName << std::endl;
        return -1;
    }
    hr = deviceManager->SetDeviceVolume(device.get(), volume);
    if (FAILED(hr))
    {
        std::cerr << "Failed to set device volume: " << endPointName << std::endl;
        return -1;
    }
    return 0;
}

int nnl_audio::Initialize()
{
    winrt::hresult hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    deviceManager = std::make_unique<nnl_audio::DeviceManager>();
    hr = deviceManager->Initialize();
    if (FAILED(hr))
    {
        isInitialized = false;
        std::cerr << "Failed to initialize audio session: " << std::endl;
        return -1;
    }
    loopbackStream = std::make_unique<nnl_audio::LoopbackStream>();

    isInitialized = true;
    return 0;
}




int nnl_audio::GetConnectedOutputDevices(std::vector<std::string>& deviceNames)
{
    REQUIRE_INITIALIZED;
    std::vector<winrt::com_ptr<IMMDevice>> connectedDevices;
    winrt::hresult hr = deviceManager->GetConnectedDevices(eRender, connectedDevices);
    if (FAILED(hr))
    {
        return -1;
    }
    for (const auto& device : connectedDevices)
    {
        std::string name;
        winrt::hresult hr = deviceManager->GetDeviceName(device.get(), name);
        if (SUCCEEDED(hr))
        {
            deviceNames.push_back(name);
        }
    }
    return 0;
}

int nnl_audio::StartLoopbackStream(const std::string& sourceName, const std::string& sinkName)
{
    REQUIRE_INITIALIZED;
    winrt::hresult hr = deviceManager->GetDeviceByName(sourceName, sourceDevice);
    if (FAILED(hr))
    {
        std::cerr << "Failed to get source device." << std::endl;
        return -1;
    }
    hr = deviceManager->GetDeviceByName(sinkName, sinkDevice);
    if (FAILED(hr))
    {
        std::cerr << "Failed to get sink device." << std::endl;
        return -1;
    }
    loopbackStream->Start(sourceDevice.get(), sinkDevice.get());
    if (FAILED(hr))
    {
        std::cerr << "Failed to start loopback stream." << std::endl;
        return -1;
    }
    return 0;
}

int nnl_audio::StopLoopbackStream()
{
    REQUIRE_INITIALIZED;
    if (loopbackStream)
    {
        winrt::hresult hr = loopbackStream->Stop();
        if (SUCCEEDED(hr))
        {
            return 0;
        }
    }
    std::cerr << "Failed to stop loopback stream." << std::endl;
    return -1;
}
