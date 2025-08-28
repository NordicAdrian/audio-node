#include <node.h>
#include <iostream>
#include <memory>

#include <nnl_audio.h>



void InitializeAudioSession(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() > 1)
    {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Invalid arguments").ToLocalChecked());
        return;
    }
    if (nnl_audio::Initialize() == -1)
    {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to initialize audio session").ToLocalChecked());
        return;
    }
}


void SetVolume(const v8::FunctionCallbackInfo<v8::Value>& args) 
{
    v8::Isolate* isolate = args.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsNumber()) 
    {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Volume must be a number and device ID must be a string").ToLocalChecked());
        return;
    }
    
    std::string deviceId(*v8::String::Utf8Value(isolate, args[0]));
    float volume = args[1]->NumberValue(context).ToChecked();


    if (volume < 0.0 || volume > 1.0) 
    {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Volume must be between 0.0 and 1.0").ToLocalChecked());
        return;
    }
    if (nnl_audio::SetEndpointVolume(deviceId, volume) == -1)
    {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to set endpoint volume").ToLocalChecked());
        return;
    }

    args.GetReturnValue().Set(v8::Number::New(isolate, volume));
}


void GetConnectedAudioOutputs(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = args.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    std::vector<std::string> devices;
    if (nnl_audio::GetConnectedOutputDevices(devices) == -1)
    {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to get connected output devices").ToLocalChecked());
        return;
    }
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
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString())
    {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Device IDs must be strings").ToLocalChecked());
        return;
    }
    std::string sourceId(*v8::String::Utf8Value(isolate, args[0]));
    std::string sinkId(*v8::String::Utf8Value(isolate, args[1]));

    if (nnl_audio::StartLoopbackStream(sourceId, sinkId) == -1)
    {
        isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Failed to start loopback stream").ToLocalChecked());
        return;
    }
}

void StopLoopbackStream(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = args.GetIsolate();
    if (nnl_audio::StopLoopbackStream() == -1)
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
    NODE_SET_METHOD(exports, "stopLoopbackStream", StopLoopbackStream);
}


NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)




