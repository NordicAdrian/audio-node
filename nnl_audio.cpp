#include <node.h>
#include <iostream>
#include <memory>

#include <nnl_audio.h>

namespace nnl_audio
{


    void InitializeAudioSession(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        v8::Isolate* isolate = args.GetIsolate();
        
        if (args.Length() < 1)
        {
            if (InitializeAudioSession() == -1)
            {
                isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to initialize audio session").ToLocalChecked());
                return;
            }
        }
        else if (args.Length() == 1 && args[0]->IsString())
        {
            std::string deviceId(*v8::String::Utf8Value(isolate, args[0]));

            if (InitializeAudioSession(deviceId) == -1)
            {
                isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to initialize audio session").ToLocalChecked());
                return;
            }
        }
        else
        {
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Invalid arguments, must contain one valid device ID or none").ToLocalChecked());
            return;
        }
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
        if (SetSessionVolume(volume) == -1)
        {
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to set session volume").ToLocalChecked());
            return;
        }
       
        args.GetReturnValue().Set(v8::Number::New(isolate, volume));
    }


    void GetConnectedAudioOutputs(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        v8::Isolate* isolate = args.GetIsolate();
        v8::Local<v8::Context> context = isolate->GetCurrentContext();

        std::vector<std::string> devices = GetConnectedOutputDevices();
        v8::Local<v8::Array> deviceArray = v8::Array::New(isolate, static_cast<int>(devices.size()));
        for (size_t i = 0; i < devices.size(); ++i) 
        {
            v8::Local<v8::String> deviceNameStr = v8::String::NewFromUtf8(isolate, devices[i].c_str()).ToLocalChecked();
            deviceArray->Set(context, static_cast<uint32_t>(i), deviceNameStr).Check();
        }
        args.GetReturnValue().Set(deviceArray);
    }

    void StartLoopbackStream(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        v8::Isolate* isolate = args.GetIsolate();
        
        if (args.Length() < 1 || !args[0]->IsString())
        {
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Device ID must be a string").ToLocalChecked());
            return;
        }

        std::string deviceId(*v8::String::Utf8Value(isolate, args[0]));

        if (StartLoopbackStream(deviceId) == -1)
        {
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to start loopback stream").ToLocalChecked());
            return;
        }
    }

    void StopLoopbackStream(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        v8::Isolate* isolate = args.GetIsolate();
        
        if (StopLoopbackStream() == -1)
        {
            isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to stop loopback stream").ToLocalChecked());
            return;
        }
    }


    void Initialize(v8::Local<v8::Object> exports)
    {
        NODE_SET_METHOD(exports, "setVolume", SetVolume);
        NODE_SET_METHOD(exports, "initialize", InitializeAudioSession);
        NODE_SET_METHOD(exports, "getConnectedAudioOutputs", GetConnectedAudioOutputs);
        NODE_SET_METHOD(exports, "startLoopbackStream", StartLoopbackStream);
    }


    NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)


}

    // void StartLoopbackStream(const v8::FunctionCallbackInfo<v8::Value>& args)
    // {
    //     if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString())
    //     {
    //         v8::Isolate* isolate = args.GetIsolate();
    //         isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Device IDs must be strings").ToLocalChecked());
    //         return;
    //     }

    //     v8::String::Utf8Value loopbackDeviceId(args.GetIsolate(), args[0]);
    //     v8::String::Utf8Value outputDeviceId(args.GetIsolate(), args[1]);
    //     std::string loopbackDeviceIdStr(*loopbackDeviceId ? *loopbackDeviceId : "");
    //     std::string outputDeviceIdStr(*outputDeviceId ? *outputDeviceId : "");

    //     loopbackStream = std::make_unique<LoopbackStream>();
    //     IMMDevice* loopbackDevice = deviceManager->GetDevice(loopbackDeviceIdStr);
    //     IMMDevice* outputDevice = deviceManager->GetDevice(outputDeviceIdStr);
    //     if (loopbackDevice && outputDevice)
    //     {
    //         loopbackStream->Start(loopbackDevice, outputDevice);
    //     }
    //     else
    //     {
    //         v8::Isolate* isolate = args.GetIsolate();
    //         isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to find devices").ToLocalChecked());
    //         return;
    //     }

    // }


    
    // void GetDevices(const v8::FunctionCallbackInfo<v8::Value>& args)
    // {
    //     v8::Isolate* isolate = args.GetIsolate();
    //     v8::Local<v8::Array> deviceArray = v8::Array::New(isolate);
        
    //     std::vector<winrt::com_ptr<IMMDevice>> devices = deviceManager->GetConnectedDevices(eRender);
    //     for (size_t i = 0; i < devices.size(); ++i) 
    //     {
    //         std::string deviceName = deviceManager->GetDeviceName(devices[i].get());
    //         v8::Local<v8::String> deviceNameStr = v8::String::NewFromUtf8(isolate, deviceName.c_str()).ToLocalChecked();
    //         deviceArray->Set(isolate->GetCurrentContext(), i, deviceNameStr);
    //     }
        
    //     args.GetReturnValue().Set(deviceArray);
    // }