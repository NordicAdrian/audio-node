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

    winrt::hresult Initialize();
    bool IsInitialized() const { return m_isInitialized; }

    winrt::hresult GetConnectedDevices(EDataFlow dataFlow, std::vector<winrt::com_ptr<IMMDevice>>& devices);
    winrt::hresult GetConnectedDeviceIDs(EDataFlow dataFlow, std::vector<LPCWSTR>& deviceIDs);

    winrt::hresult GetDefaultInputDevice(winrt::com_ptr<IMMDevice>& device);
    winrt::hresult GetDefaultOutputDevice(winrt::com_ptr<IMMDevice>& device);
    winrt::hresult GetDeviceByID(LPCWSTR ID, winrt::com_ptr<IMMDevice>& device);
    winrt::hresult GetDeviceByName(const std::string &name, winrt::com_ptr<IMMDevice>& device);

    winrt::hresult GetDeviceIsConnected(LPCWSTR ID, EDataFlow dataFlow);
    winrt::hresult GetDeviceName(IMMDevice* device, std::string& name);
    winrt::hresult GetDeviceID(IMMDevice* device, LPCWSTR& deviceID);

    static winrt::hresult SetDeviceVolume(IMMDevice* device, float volume);


protected:

    #define REQUIRE_DEVICE_MANAGER_INITIALIZED() if (!IsInitialized()) { std::cerr << "DeviceManager not initialized\n"; return E_FAIL; }

    winrt::com_ptr<IMMDeviceEnumerator> m_deviceEnumerator;

    // IMMNotificationClient methods
    STDMETHOD(OnDeviceAdded)(LPCWSTR pwstrDeviceId) override {return S_OK; }
    STDMETHOD(OnDeviceRemoved)(LPCWSTR pwstrDeviceId) override { return S_OK; }
    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override { return S_OK; }
    STDMETHOD(OnDeviceStateChanged)(LPCWSTR pwstrDeviceId, DWORD dwNewState) override { return S_OK; }

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

    bool m_isInitialized = false;

};
}