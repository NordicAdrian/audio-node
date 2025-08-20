#pragma once


#include <thread>
#include <mmdeviceapi.h> // Include for IMMDevice
#include <audiopolicy.h> // Include for IAudioSessionEvents
#include <mutex>
#include <condition_variable>
#include <winrt/base.h>

namespace wasapi
{



class LoopbackStream : public IAudioSessionEvents
{
public:
    LoopbackStream() = default;
    
    
    void Start(IMMDevice* loopbackDevice, IMMDevice* outputDevice);
    void Stop();
    void Pause();
    void Unpause();
    void Wait();
    

private:

    winrt::com_ptr<IAudioSessionControl> m_audioSessionRenderControl;
    winrt::com_ptr<IAudioSessionControl> m_audioSessionCaptureControl;

    std::unique_ptr<std::thread> m_thread;
    std::mutex m_mutex;
    std::condition_variable m_pauseCondition;
    std::condition_variable m_waitCondition;

    volatile bool m_isRunning = false;
    volatile bool m_isPaused = false;

    winrt::hresult run(IMMDevice* loopbackDevice, IMMDevice* outputDevice);


    STDMETHOD(OnDisplayNameChanged) (LPCWSTR /*NewDisplayName*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnIconPathChanged) (LPCWSTR /*NewIconPath*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnSimpleVolumeChanged) (float /*NewSimpleVolume*/, BOOL /*NewMute*/, LPCGUID /*EventContext*/) { return S_OK; }
    STDMETHOD(OnChannelVolumeChanged) (DWORD /*ChannelCount*/, float /*NewChannelVolumes*/[], DWORD /*ChangedChannel*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnGroupingParamChanged) (LPCGUID /*NewGroupingParam*/, LPCGUID /*EventContext*/) {return S_OK; };
    STDMETHOD(OnStateChanged) (AudioSessionState /*NewState*/) { return S_OK; };
    STDMETHOD(OnSessionDisconnected) (AudioSessionDisconnectReason DisconnectReason);

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

    ULONG refCount = 1;
    

};
}
