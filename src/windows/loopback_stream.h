#pragma once


#include <thread>
#include <mmdeviceapi.h> // Include for IMMDevice
#include <audiopolicy.h> // Include for IAudioSessionEvents
#include <mutex>
#include <condition_variable>
#include <winrt/base.h>

namespace nnl_audio
{


class LoopbackStream 
{
public:
    LoopbackStream() = default;
    
    winrt::hresult Start(IMMDevice* source, IMMDevice* sink);
    winrt::hresult Stop();
    winrt::hresult Wait();
    bool IsRunning() const { return m_isRunning; }

private:

    winrt::hresult run(IMMDevice* source, IMMDevice* sink);

    winrt::com_ptr<IAudioSessionControl> m_audioSessionRenderControl;
    winrt::com_ptr<IAudioSessionControl> m_audioSessionCaptureControl;

    std::unique_ptr<std::thread> m_thread;
    std::mutex m_mutex;
    std::condition_variable m_pauseCondition;
    std::condition_variable m_waitCondition;

    volatile bool m_isRunning = false;
    volatile bool m_isPaused = false;


};
}
