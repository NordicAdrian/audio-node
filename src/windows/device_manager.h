#pragma once

#include <unordered_map>
#include <string>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <Audioclient.h>
#include <iostream>
#include <Wmp.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <chrono>
#include <functional>
#include <winrt/base.h>



namespace nnl_audio
{

class DeviceManager : public IMMNotificationClient
{


public:

    DeviceManager() = default;
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;
    DeviceManager(DeviceManager&&) = delete;
    DeviceManager& operator=(DeviceManager&&) = delete;
    ~DeviceManager()
    {
        UnregisterDeviceNotification(this);
        if (m_deviceEnumerator)
        {
            m_deviceEnumerator->Release();
        }
    }

    winrt::hresult InitializeNotificationClient();

    std::vector<winrt::com_ptr<IMMDevice>> GetConnectedDevices(EDataFlow dataFlow);
    std::vector<LPCWSTR> GetConnectedDeviceIDs(EDataFlow dataFlow);

    IMMDevice* GetDefaultInputDevice();
    winrt::hresult GetDefaultOutputDevice(winrt::com_ptr<IMMDevice>& device);
    winrt::hresult GetDeviceByID(LPCWSTR ID, winrt::com_ptr<IMMDevice>& device);
    winrt::hresult GetDeviceByName(const std::string &name, winrt::com_ptr<IMMDevice>& device);

    winrt::hresult GetDeviceIsConnected(LPCWSTR ID, EDataFlow dataFlow);
    winrt::hresult GetDeviceName(IMMDevice* device, std::string& name);
    winrt::hresult GetDeviceID(IMMDevice* device, LPCWSTR& deviceID);

protected:

    winrt::com_ptr<IMMDeviceEnumerator> m_deviceEnumerator;

    // IMMNotificationClient methods
    virtual STDMETHOD(OnDeviceAdded)(LPCWSTR pwstrDeviceId) = 0;
    virtual STDMETHOD(OnDeviceRemoved)(LPCWSTR pwstrDeviceId) = 0;
    virtual STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) = 0;
    virtual STDMETHOD(OnDeviceStateChanged)(LPCWSTR pwstrDeviceId, DWORD dwNewState) = 0;

    STDMETHOD(OnPropertyValueChanged)(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override { return S_OK; }
    STDMETHOD_(ULONG, AddRef)() override { return InterlockedIncrement(&refCount); }
    STDMETHOD_(ULONG, Release)() override
    {
        ULONG ulRef = InterlockedDecrement(&refCount);
        if (0 == ulRef)
        {
            delete this;
        }
        return ulRef;
    }
    STDMETHOD(QueryInterface)(REFIID riid, VOID** ppvInterface) override
    {
        if (IID_IUnknown == riid || __uuidof(IMMNotificationClient) == riid)
        {
            *ppvInterface = (IMMNotificationClient*)this;
            AddRef();
            return S_OK;
        }
        *ppvInterface = NULL;
        return E_NOINTERFACE;
    }

    LONG refCount = 1;

};
}