#include <device_manager.h>



winrt::hresult nnl_audio::DeviceManager::InitializeNotificationClient()
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
    return S_OK;
}




std::vector<winrt::com_ptr<IMMDevice>> nnl_audio::DeviceManager::GetConnectedDevices(EDataFlow dataFlow)
{
    std::vector<winrt::com_ptr<IMMDevice>> connectedDevices;
    winrt::com_ptr<IMMDeviceCollection> collection;
    winrt::check_hresult(m_deviceEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, collection.put()));
    UINT count = 0;
    winrt::check_hresult(collection->GetCount(&count));
    for (UINT i = 0; i < count; ++i)
    {
        winrt::com_ptr<IMMDevice> connectedDevice;
        if (FAILED(collection->Item(i, connectedDevice.put())))
        {
            continue;
        }
        connectedDevices.push_back(connectedDevice);
    }
    return connectedDevices;
}

std::vector<LPCWSTR> nnl_audio::DeviceManager::GetConnectedDeviceIDs(EDataFlow dataFlow)
{
    std::vector<LPCWSTR> deviceIDs;
    std::vector<winrt::com_ptr<IMMDevice>> connectedDevices = GetConnectedDevices(dataFlow);
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
    return deviceIDs;
}



IMMDevice* nnl_audio::DeviceManager::GetDefaultInputDevice()
{
    winrt::com_ptr<IMMDevice> device;
    winrt::check_hresult(m_deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, device.put()));
    return device.get();
}

winrt::hresult nnl_audio::DeviceManager::GetDefaultOutputDevice(winrt::com_ptr<IMMDevice>& device)
{
    return m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.put());
}

winrt::hresult nnl_audio::DeviceManager::GetDeviceByID(LPCWSTR ID, winrt::com_ptr<IMMDevice> &device)
{
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
    winrt::hresult hr;
    for (auto dev : GetConnectedDevices(eRender))
    {
        std::string devId;
        hr = GetDeviceName(dev.get(), devId);
        if (hr == S_OK && devId.compare(name) == 0)
        {
            device = dev;
            return S_OK;
        }
    }
    return E_FAIL;
}



winrt::hresult nnl_audio::DeviceManager::GetDeviceIsConnected(LPCWSTR ID, EDataFlow dataFlow)
{
    std::vector<LPCWSTR> deviceIDs = GetConnectedDeviceIDs(dataFlow);
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




STDMETHODIMP_(HRESULT __stdcall)
nnl_audio::DeviceManager::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    return S_OK;
}
