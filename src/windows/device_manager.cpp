#include <device_manager.h>



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

IMMDevice* nnl_audio::DeviceManager::GetDefaultOutputDevice()
{
    winrt::com_ptr<IMMDevice> device;
    winrt::check_hresult(m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.put()));
    return device.get();
}

IMMDevice* nnl_audio::DeviceManager::GetDevice(LPCWSTR ID)
{
    winrt::com_ptr<IMMDevice> device;
    winrt::check_hresult(m_deviceEnumerator->GetDevice(ID, device.put()));
    return device.get();
}

IMMDevice* nnl_audio::DeviceManager::GetDevice(const std::string &id)
{
    for (const auto& device : GetConnectedDevices(eRender))
    {
        std::string deviceId = GetDeviceName(device.get());
        if (deviceId.compare(id) == 0)
        {
            return device.get();
        }
    }
    return nullptr;
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




std::string nnl_audio::DeviceManager::GetDeviceName(IMMDevice* device)
{
    winrt::com_ptr<IPropertyStore> propertyStore;
    winrt::check_hresult(device->OpenPropertyStore(STGM_READ, propertyStore.put()));

    PROPVARIANT friendlyName;
    PropVariantInit(&friendlyName);

    winrt::check_hresult(propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName));

    std::wstring wname(friendlyName.pwszVal);
    std::string name(wname.begin(), wname.end());

    PropVariantClear(&friendlyName);
    return name;
}

std::string nnl_audio::DeviceManager::GetDeviceName(LPCWSTR ID)
{
    winrt::com_ptr<IMMDevice> device;
    winrt::check_hresult(m_deviceEnumerator->GetDevice(ID, device.put()));
    return GetDeviceName(device.get());
}

LPCWSTR nnl_audio::DeviceManager::GetDeviceID(IMMDevice *device)
{
    LPWSTR id;
    winrt::check_hresult(device->GetId(&id));
    return id;
}

STDMETHODIMP_(HRESULT __stdcall)
nnl_audio::DeviceManager::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall)
nnl_audio::DeviceManager::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall)
nnl_audio::DeviceManager::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall)
nnl_audio::DeviceManager::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    return S_OK;
}
