#pragma once


#include <string>
#include <memory>
#include <vector>

namespace nnl_audio
{

/*
    Set the volume for all audio sessions for the current device session.
*/
int SetSessionVolume(float volume);

/*
    Initialize the audio session for a specific device. If the device is removed the default device is chosen.
*/
int InitializeAudioSession(const std::string& deviceId);

/*
    Initialize the audio session for the default device. If the default device changes it will be re-initialized.
*/
int InitializeAudioSession();


int StartLoopbackStream(const std::string& deviceId);

int StopLoopbackStream();


std::vector<std::string> GetConnectedOutputDevices();





} // namespace nnl_audio


