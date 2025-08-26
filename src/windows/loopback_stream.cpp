#include <loopback_stream.h>
#include <audioclient.h> // Required for IAudioClient
#include <iostream>


//https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/multimedia/nnl_audio/CaptureSharedEventDriven/WASAPICapture.cpp

winrt::hresult nnl_audio::LoopbackStream::Start(IMMDevice* source, IMMDevice *sink)
{
    m_thread.reset(new std::thread([this, source, sink]
    {
        m_isRunning = true;
        winrt::hresult hr = run(source, sink);
        if (FAILED(hr))
        {
            std::cout << "Failed to start loopback stream: " << hr << std::endl;
            return hr;
        }
    }));
    return S_OK;
}

winrt::hresult  nnl_audio::LoopbackStream::Wait()
{
    std::unique_lock<std::mutex> l(m_mutex);
    m_waitCondition.wait(l);
    return S_OK;
}


winrt::hresult nnl_audio::LoopbackStream::Stop()
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
    std::cout << "Loopback stream stopped." << std::endl;
    return S_OK;

}



winrt::hresult nnl_audio::LoopbackStream::run(IMMDevice* source, IMMDevice *sink)
{

    
    winrt::hresult hr;

    winrt::com_ptr<IAudioClient> inputAudioClient;
    winrt::com_ptr<IAudioClient> outputAudioClient;

    winrt::com_ptr<IAudioCaptureClient> captureClient;
    winrt::com_ptr<IAudioRenderClient> renderClient;
    WAVEFORMATEX *format;
    

    hr = source->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, inputAudioClient.put_void());   if (FAILED(hr)) return hr;

    hr = sink->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, outputAudioClient.put_void());    if (FAILED(hr)) return hr;

    hr = outputAudioClient->GetMixFormat(&format); if (FAILED(hr)) return hr;

    hr = inputAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 10000000, 0, format, NULL); if (FAILED(hr)) return hr;
    hr = outputAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, 10000000, 0, format, NULL); if (FAILED(hr)) return hr;

    hr = inputAudioClient->GetService(__uuidof(IAudioSessionControl), m_audioSessionCaptureControl.put_void()); if (FAILED(hr)) return hr;
    hr = outputAudioClient->GetService(__uuidof(IAudioSessionControl), m_audioSessionRenderControl.put_void()); if (FAILED(hr)) return hr;

    hr = inputAudioClient->GetService(__uuidof(IAudioCaptureClient), captureClient.put_void()); if (FAILED(hr)) return hr;
    hr = outputAudioClient->GetService(__uuidof(IAudioRenderClient), renderClient.put_void()); if (FAILED(hr)) return hr;

    hr = inputAudioClient->Start(); if (FAILED(hr)) return hr;
    hr = outputAudioClient->Start(); if (FAILED(hr)) return hr;

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
                std::cout << "Failed to get capture buffer: " << hr << std::endl;
                m_isRunning = false;
                return hr;
            }
            hr = renderClient->GetBuffer(nFrames, &renderBuffer);
            if (FAILED(hr))
            {
                std::cout << "Failed to get render buffer: " << hr << std::endl;
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
            std::cout << "Failed to release capture buffer: " << hr << std::endl;
            m_isRunning = false;
            return hr;
        }
        hr = renderClient->ReleaseBuffer(nFrames, 0);
        if (FAILED(hr))
        {
            std::cout << "Failed to release render buffer: " << hr << std::endl;
            m_isRunning = false;
            return hr;
        }

        
    } while (m_isRunning);

    inputAudioClient->Stop();
    outputAudioClient->Stop();
    m_waitCondition.notify_one();

    return S_OK;
    
    
}

