#pragma once


#include <vector>
#include <mmdeviceapi.h> 
#include <audiopolicy.h> 
#include <condition_variable>
#include <winrt/base.h>
#include <functional>



class AudioSessionManager : public IAudioSessionNotification
{
public:
    AudioSessionManager(IAudioSessionManager2 *audioSessionManager, std::function<void(IAudioSessionControl*)> onSessionCreated) :
        m_onSessionCreated(onSessionCreated) {
            m_audioSessionManager.attach(audioSessionManager);
            m_audioSessionManager->AddRef();
            m_audioSessionManager->RegisterSessionNotification(this);
        }

    AudioSessionManager(const AudioSessionManager&) = delete;
    AudioSessionManager& operator=(const AudioSessionManager&) = delete;
    AudioSessionManager(AudioSessionManager&&) = delete;
    AudioSessionManager& operator=(AudioSessionManager&&) = delete;
    ~AudioSessionManager()
    {
        if (m_audioSessionManager)
        {
            m_audioSessionManager->UnregisterSessionNotification(this);
        }
    }

    IAudioSessionManager2* GetAudioSessionManager() const
    {
        return m_audioSessionManager.get();
    }

private:
    STDMETHOD(QueryInterface)(REFIID riid, VOID** ppvInterface) override
    {    
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvInterface = (IUnknown*)this;
        }
        else if (__uuidof(IAudioSessionNotification) == riid)
        {
            AddRef();
            *ppvInterface = (IAudioSessionNotification*)this;
        }
        else
        {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }
    
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

    STDMETHOD(OnSessionCreated)(IAudioSessionControl* sessionControl)
    {
        if (m_onSessionCreated)
        {
            m_onSessionCreated(sessionControl);
        }
        return S_OK;
    }


    winrt::com_ptr<IAudioSessionManager2> m_audioSessionManager;
    std::function<void(IAudioSessionControl*)> m_onSessionCreated;
    ULONG refCount = 1;
};