#pragma once


#include <string>
#include <memory>
#include <vector>





namespace nnl_audio
{

#define REQUIRE_INITIALIZED if (!isInitialized) { std::cerr << "Audio session is not initialized." << std::endl; return -1; }
/*
    Initialize the audio session. Must be called before any other calls to the API.
*/
int Initialize();

/*
    Set the volume for an audio endpoint identified by the endPointName.
*/
int SetEndpointVolume(const std::string& endPointName, float volume);


/*
    Start the loopback stream between two audio endpoints.
*/
int StartLoopbackStream(const std::string& sourceName, const std::string& sinkName);

/*
    Stop the loopback stream.
*/
int StopLoopbackStream();


/*
    Get a list of all connected output devices.
*/
int GetConnectedOutputDevices(std::vector<std::string>& deviceNames);


inline bool isInitialized = false;

} // namespace nnl_audio


