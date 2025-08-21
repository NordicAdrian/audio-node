#pragma once

#include <vector>
#include <mmdeviceapi.h> 
#include <audiopolicy.h> 
#include <condition_variable>
#include <winrt/base.h>
#include <functional>


class AudioSessionControl : public IAudioSessionEvents
{

public:
    AudioSessionControl(IAudioSessionControl* audioSessionControl, std::function<void()> OnSessionDisconnected, std::function<void(float newVolume)> OnSimpleVolumeChanged) :
        m_OnSessionDisconnected(OnSessionDisconnected),
        m_OnSimpleVolumeChanged(OnSimpleVolumeChanged)
         {
            m_audioSessionControl.attach(audioSessionControl);
            m_audioSessionControl->AddRef();
            m_audioSessionControl->RegisterAudioSessionNotification(this);

         }

    IAudioSessionControl* GetAudioSessionControl() const
    {
        return m_audioSessionControl.get();
    }


    STDMETHOD(OnDisplayNameChanged) (LPCWSTR /*NewDisplayName*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnIconPathChanged) (LPCWSTR /*NewIconPath*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnChannelVolumeChanged) (DWORD /*ChannelCount*/, float /*NewChannelVolumes*/[], DWORD /*ChangedChannel*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnGroupingParamChanged) (LPCGUID /*NewGroupingParam*/, LPCGUID /*EventContext*/) {return S_OK; };
    STDMETHOD(OnStateChanged) (AudioSessionState NewState) { return S_OK; }
    STDMETHOD(OnSessionDisconnected) (AudioSessionDisconnectReason DisconnectReason) { m_OnSessionDisconnected(); return S_OK; }
    STDMETHOD(OnSimpleVolumeChanged) (float newSimpleVolume, BOOL newMute, LPCGUID eventContext) { m_OnSimpleVolumeChanged(newSimpleVolume); return S_OK; }


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
private:
    ULONG refCount = 1;

    std::function<void()> m_OnSessionDisconnected;
    std::function<void(float newVolume)> m_OnSimpleVolumeChanged;
    winrt::com_ptr<IAudioSessionControl> m_audioSessionControl;

};