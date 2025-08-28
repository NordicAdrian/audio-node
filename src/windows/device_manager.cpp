#include <device_manager.h>
#include <endpointvolume.h>



winrt::hresult nnl_audio::DeviceManager::Initialize()
{
    winrt::hresult hr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(m_deviceEnumerator.put()));
    if (FAILED(hr))
    {
        std::cout << "Failed to create MMDeviceEnumerator instance: " << std::endl;
        return hr;
    }

    hr = m_deviceEnumerator->RegisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        std::cout << "Failed to register endpoint notification callback: " << std::endl;
        return hr;
    }
    m_isInitialized = true;
    return S_OK;
}




winrt::hresult nnl_audio::DeviceManager::GetConnectedDevices(EDataFlow dataFlow, std::vector<winrt::com_ptr<IMMDevice>>& devices)
{
    REQUIRE_DEVICE_MANAGER_INITIALIZED();
    winrt::hresult hr;
    winrt::com_ptr<IMMDeviceCollection> collection;
    hr = m_deviceEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, collection.put());
    if (FAILED(hr))
    {
        std::cout << "Failed to enumerate audio endpoints: " << std::endl;
        return hr;
    }
    UINT count = 0;
    hr = collection->GetCount(&count);
    if (FAILED(hr))
    {
        std::cout << "Failed to get count of audio endpoints: " << std::endl;
        return hr;
    }
    for (UINT i = 0; i < count; ++i)
    {
        winrt::com_ptr<IMMDevice> connectedDevice;
        if (FAILED(collection->Item(i, connectedDevice.put())))
        {
            continue;
        }
        devices.push_back(connectedDevice);
    }
    return S_OK;
}

winrt::hresult nnl_audio::DeviceManager::GetConnectedDeviceIDs(EDataFlow dataFlow, std::vector<LPCWSTR>& deviceIDs)
{
    REQUIRE_DEVICE_MANAGER_INITIALIZED();

    std::vector<winrt::com_ptr<IMMDevice>> connectedDevices;
    winrt::hresult hr = GetConnectedDevices(dataFlow, connectedDevices);
    if (FAILED(hr))
    {
        std::cout << "Failed to get connected devices: " << std::endl;
        return hr;
    }
    for (int i = 0; i < connectedDevices.size(); i++)
    {
        winrt::com_ptr<IMMDevice> device = connectedDevices[i];
        LPWSTR id;
        if (winrt::check_hresult(device->GetId(&id)) != S_OK)
        {
            continue;
        }
        deviceIDs.push_back(id);
    }
    return S_OK;
}



winrt::hresult nnl_audio::DeviceManager::GetDefaultInputDevice(winrt::com_ptr<IMMDevice>& device)
{
    REQUIRE_DEVICE_MANAGER_INITIALIZED();
    return m_deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, device.put());
}

winrt::hresult nnl_audio::DeviceManager::GetDefaultOutputDevice(winrt::com_ptr<IMMDevice>& device)
{
    REQUIRE_DEVICE_MANAGER_INITIALIZED();
    return m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.put());
}

winrt::hresult nnl_audio::DeviceManager::GetDeviceByID(LPCWSTR ID, winrt::com_ptr<IMMDevice> &device)
{
    REQUIRE_DEVICE_MANAGER_INITIALIZED();
    winrt::hresult hr = m_deviceEnumerator->GetDevice(ID, device.put());
    if (FAILED(hr))
    {
        std::cout << "Failed to get device by ID: " << std::endl;
        return hr;
    }
    return hr;
}


winrt::hresult nnl_audio::DeviceManager::GetDeviceByName(const std::string &name, winrt::com_ptr<IMMDevice>& device)
{
    REQUIRE_DEVICE_MANAGER_INITIALIZED();
    winrt::hresult hr;
    std::vector<winrt::com_ptr<IMMDevice>> connectedDevices;
    hr = GetConnectedDevices(eRender, connectedDevices);
    if (FAILED(hr))
    {
        std::cout << "Failed to get connected devices: " << std::endl;
        return hr;
    }
    for (auto dev : connectedDevices)
    {
        std::string devId;
        hr = GetDeviceName(dev.get(), devId);
        if (hr == S_OK && devId.compare(name) == 0)
        {
            device = dev;
            return S_OK;
        }
    }
    std::cout << "Failed to get device by name: " << name << std::endl;
    return E_FAIL;
}



winrt::hresult nnl_audio::DeviceManager::GetDeviceIsConnected(LPCWSTR ID, EDataFlow dataFlow)
{
    REQUIRE_DEVICE_MANAGER_INITIALIZED();
    std::vector<LPCWSTR> deviceIDs;
    winrt::hresult hr = GetConnectedDeviceIDs(dataFlow, deviceIDs);
    if (FAILED(hr))
    {
        std::cout << "Failed to get connected device IDs: " << std::endl;
        return hr;
    }
    for (int i = 0; i < deviceIDs.size(); i++)
    {
        if (wcscmp(deviceIDs[i], ID) == 0)
        {
            return S_OK;
        }
    }
    return S_FALSE;
}




winrt::hresult nnl_audio::DeviceManager::GetDeviceName(IMMDevice* device, std::string& name)
{
    REQUIRE_DEVICE_MANAGER_INITIALIZED();

    winrt::hresult hr;
    winrt::com_ptr<IPropertyStore> propertyStore;
    hr = device->OpenPropertyStore(STGM_READ, propertyStore.put());
    if (FAILED(hr))
    {
        return hr;
    }
    PROPVARIANT friendlyName;
    PropVariantInit(&friendlyName);

    hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
    if (FAILED(hr))
    {
        return hr;
    }


    std::wstring wname(friendlyName.pwszVal);
    name = std::string(wname.begin(), wname.end());

    PropVariantClear(&friendlyName);
    return S_OK;
}


winrt::hresult nnl_audio::DeviceManager::GetDeviceID(IMMDevice* device, LPCWSTR& deviceID)
{
    REQUIRE_DEVICE_MANAGER_INITIALIZED();

    LPWSTR id;
    winrt::hresult hr = device->GetId(&id);
    if (FAILED(hr))
    {
        std::cout << "Failed to get device ID: " << std::endl;
        return hr;
    }
    deviceID = id;
    return S_OK;
}

winrt::hresult nnl_audio::DeviceManager::SetDeviceVolume(IMMDevice *device, float volume)
{
    winrt::hresult hr;
    winrt::com_ptr<IAudioEndpointVolume> endpointVolume;
    hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, endpointVolume.put_void());
    if (FAILED(hr))
    {
        std::cout << "Failed to activate endpoint volume: " << std::endl;
        return hr;
    }

    hr = endpointVolume->SetMasterVolumeLevelScalar(volume, NULL);
    if (FAILED(hr))
    {
        std::cout << "Failed to set device volume: " << std::endl;
        return hr;
    }
    return S_OK;
}
