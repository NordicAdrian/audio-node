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

    winrt::hresult Initialize();

    std::vector<winrt::com_ptr<IMMDevice>> GetConnectedDevices(EDataFlow dataFlow);
    std::vector<LPCWSTR> GetConnectedDeviceIDs(EDataFlow dataFlow);

    IMMDevice* GetDefaultInputDevice();
    IMMDevice* GetDefaultOutputDevice();
    IMMDevice* GetDevice(LPCWSTR ID);
    IMMDevice* GetDevice(const std::string &id);

    winrt::hresult GetDeviceIsConnected(LPCWSTR ID, EDataFlow dataFlow);

    std::string GetDeviceName(IMMDevice* device);
    std::string GetDeviceName(LPCWSTR ID);
    LPCWSTR GetDeviceID(IMMDevice* device);

private:

    winrt::com_ptr<IMMDeviceEnumerator> m_deviceEnumerator;

    // IMMNotificationClient methods
    STDMETHOD(OnDeviceAdded)(LPCWSTR pwstrDeviceId) override;
    STDMETHOD(OnDeviceRemoved)(LPCWSTR pwstrDeviceId) override;
    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;

    STDMETHOD(OnDeviceStateChanged)(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
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