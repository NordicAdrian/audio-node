{
  "targets": [
    {
      "target_name": "wasapi_node",
      "sources": ["wasapi/audio_session.cpp", "wasapi/device_manager.cpp", "wasapi/loopback_stream.cpp", "wasapi.cpp" ],
      "include_dirs": [
        "wasapi"
      ],
      "libraries": [
        "windowsapp.lib"
      ]
    }
  ]
}