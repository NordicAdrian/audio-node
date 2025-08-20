#include <loopback_stream.h>
#include <audioclient.h> // Required for IAudioClient



//https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/multimedia/audio/CaptureSharedEventDriven/WASAPICapture.cpp

void wasapi::LoopbackStream::Start(IMMDevice* loopbackDevice, IMMDevice *outputDevice)
{
    m_thread.reset(new std::thread([this, loopbackDevice, outputDevice]
    {
        m_isRunning = true;
        winrt::hresult hr = run(loopbackDevice, outputDevice);
        if (FAILED(hr))
        {
            return;
        }
    }));
}



void wasapi::LoopbackStream::Wait()
{
    std::unique_lock<std::mutex> l(m_mutex);
    m_waitCondition.wait(l);
}


void wasapi::LoopbackStream::Stop()
{
    if (m_isRunning)
    {
        {
            std::lock_guard<std::mutex> l(m_mutex);
            m_isRunning = false;
            m_waitCondition.notify_all();
        }
        Wait();
    }
    if (m_thread && m_thread->joinable())
    {
        m_thread->join();
    }


    m_audioSessionCaptureControl->UnregisterAudioSessionNotification(this);
    m_audioSessionRenderControl->UnregisterAudioSessionNotification(this);
}

void wasapi::LoopbackStream::Pause()
{
    std::lock_guard<std::mutex> l(m_mutex);
    m_isPaused = true;
}

void wasapi::LoopbackStream::Unpause()
{
    {
        std::lock_guard<std::mutex> l(m_mutex);
        m_isPaused = false;
    }
    m_pauseCondition.notify_one();
}

winrt::hresult wasapi::LoopbackStream::run(IMMDevice* loopbackDevice, IMMDevice *outputDevice)
{

    
    winrt::hresult hr;

    winrt::com_ptr<IAudioClient> inputAudioClient;
    winrt::com_ptr<IAudioClient> outputAudioClient;

    winrt::com_ptr<IAudioCaptureClient> captureClient;
    winrt::com_ptr<IAudioRenderClient> renderClient;
    WAVEFORMATEX *format;
    

    try
    {
        winrt::check_hresult(loopbackDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, inputAudioClient.put_void()));
        winrt::check_hresult(outputDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, outputAudioClient.put_void()));

        winrt::check_hresult(outputAudioClient->GetMixFormat(&format));

        winrt::check_hresult(inputAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 10000000, 0, format, NULL));
        winrt::check_hresult(outputAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, 10000000, 0, format, NULL));

        winrt::check_hresult(inputAudioClient->GetService(__uuidof(IAudioSessionControl), m_audioSessionCaptureControl.put_void()));
        winrt::check_hresult(outputAudioClient->GetService(__uuidof(IAudioSessionControl), m_audioSessionRenderControl.put_void()));

        winrt::check_hresult(m_audioSessionCaptureControl->RegisterAudioSessionNotification(this));
        winrt::check_hresult(m_audioSessionRenderControl->RegisterAudioSessionNotification(this));

        winrt::check_hresult(inputAudioClient->GetService(__uuidof(IAudioCaptureClient), captureClient.put_void()));
        winrt::check_hresult(outputAudioClient->GetService(__uuidof(IAudioRenderClient), renderClient.put_void()));

        winrt::check_hresult(inputAudioClient->Start());
        winrt::check_hresult(outputAudioClient->Start());

    }
    catch (winrt::hresult_error e)
    {
        m_isRunning = false;
        return e.code();
    }
    do
    {
        BYTE* captureBuffer;
        BYTE* renderBuffer;

        UINT32 nFrames;
        DWORD flags;
        UINT32 nNewSamples;
        winrt::hresult hr;

        if (m_isPaused)
        {
            {
                std::unique_lock<std::mutex> l(m_mutex);
                m_pauseCondition.wait(l);
            }
        }
        {
            std::unique_lock<std::mutex> l(m_mutex);
            hr = captureClient->GetBuffer(&captureBuffer, &nFrames, &flags, NULL, NULL);
            if (FAILED(hr))
            {
                m_isRunning = false;
                return hr;
            }
            hr = renderClient->GetBuffer(nFrames, &renderBuffer);
            if (FAILED(hr))
            {
                m_isRunning = false;
                return hr;
            }
        }
        if (nFrames == 0)
        {
            continue;
        }

        memcpy(renderBuffer, captureBuffer, format->nBlockAlign * nFrames);
        hr = captureClient->ReleaseBuffer(nFrames);
        if (FAILED(hr))
        {
            m_isRunning = false;
            return hr;
        }
        hr = renderClient->ReleaseBuffer(nFrames, 0);
        if (FAILED(hr))
        {
            m_isRunning = false;
            return hr;
        }

        
    } while (m_isRunning);

    inputAudioClient->Stop();
    outputAudioClient->Stop();
    m_waitCondition.notify_one();

    return S_OK;
    
    
}

STDMETHODIMP_(HRESULT __stdcall)
wasapi::LoopbackStream::OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason)
{
    Stop();
    return S_OK;
}
