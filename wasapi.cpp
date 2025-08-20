#include <node.h>
#include <audio_session.h>
#include <device_manager.h> 
#include <loopback_stream.h>
#include <iostream>
#include <memory>

namespace wasapi
{

    std::unique_ptr<AudioSession> audioSession;
    std::unique_ptr<DeviceManager> deviceManager;
    std::unique_ptr<LoopbackStream> loopbackStream;

    void InitializeAudioSession(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        deviceManager = std::make_unique<DeviceManager>();
        audioSession = std::make_unique<AudioSession>(deviceManager->GetDefaultOutputDevice());
        winrt::hresult hr = audioSession->Initialize();
        v8::Isolate* isolate = args.GetIsolate();
        if (FAILED(hr))
        {
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to initialize audio session").ToLocalChecked());
            return;
        }
        std::cout << "Audio session initialized successfully." << std::endl;
        args.GetReturnValue().Set(v8::Boolean::New(isolate, true));
    }


    void StartLoopbackStream(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString())
        {
            v8::Isolate* isolate = args.GetIsolate();
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Device IDs must be strings").ToLocalChecked());
            return;
        }

        v8::String::Utf8Value loopbackDeviceId(args.GetIsolate(), args[0]);
        v8::String::Utf8Value outputDeviceId(args.GetIsolate(), args[1]);
        std::string loopbackDeviceIdStr(*loopbackDeviceId ? *loopbackDeviceId : "");
        std::string outputDeviceIdStr(*outputDeviceId ? *outputDeviceId : "");

        loopbackStream = std::make_unique<LoopbackStream>();
        IMMDevice* loopbackDevice = deviceManager->GetDevice(loopbackDeviceIdStr);
        IMMDevice* outputDevice = deviceManager->GetDevice(outputDeviceIdStr);
        if (loopbackDevice && outputDevice)
        {
            loopbackStream->Start(loopbackDevice, outputDevice);
        }
        else
        {
            v8::Isolate* isolate = args.GetIsolate();
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to find devices").ToLocalChecked());
            return;
        }

    }

    void GetDevices(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        v8::Isolate* isolate = args.GetIsolate();
        v8::Local<v8::Array> deviceArray = v8::Array::New(isolate);
        
        std::vector<winrt::com_ptr<IMMDevice>> devices = deviceManager->GetConnectedDevices(eRender);
        for (size_t i = 0; i < devices.size(); ++i) 
        {
            std::string deviceName = deviceManager->GetDeviceName(devices[i].get());
            v8::Local<v8::String> deviceNameStr = v8::String::NewFromUtf8(isolate, deviceName.c_str()).ToLocalChecked();
            deviceArray->Set(isolate->GetCurrentContext(), i, deviceNameStr);
        }
        
        args.GetReturnValue().Set(deviceArray);
    }

    void SetVolume(const v8::FunctionCallbackInfo<v8::Value>& args) 
    {
        v8::Isolate* isolate = args.GetIsolate();
        v8::Local<v8::Context> context = isolate->GetCurrentContext();

        if (args.Length() < 1 || !args[0]->IsNumber()) 
        {
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Volume must be a number").ToLocalChecked());
            return;
        }
        
        float volume = args[0]->NumberValue(context).ToChecked();
        
        if (volume < 0.0 || volume > 1.0) 
        {
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Volume must be between 0.0 and 1.0").ToLocalChecked());
            return;
        }
    
        winrt::hresult hr = audioSession->SetVolume(volume);
        if (FAILED(hr)) 
        {
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to set default device volume").ToLocalChecked());
            return;
        }
        std::cout << "Volume set to: " << volume << std::endl;
        args.GetReturnValue().Set(v8::Number::New(isolate, volume));
    }


    void Initialize(v8::Local<v8::Object> exports)
    {
        NODE_SET_METHOD(exports, "setVolume", SetVolume);
        NODE_SET_METHOD(exports, "initialize", InitializeAudioSession);
        NODE_SET_METHOD(exports, "startLoopbackStream", StartLoopbackStream);
        NODE_SET_METHOD(exports, "getDevices", GetDevices);
    }


    NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)

}


