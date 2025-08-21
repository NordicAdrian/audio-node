#pragma once


#include <string>
#include <memory>

namespace nnl_audio
{

int SetSessionVolume(float volume);


int InitializeAudioSession(const std::string& deviceId);
int InitializeAudioSession();


} // namespace nnl_audio


