{
  "targets": [
    {
      "target_name": "nnl_audio_node",
      "sources" : [],
      "conditions": [
        ['OS=="win"', {
          "sources": [
              "src/windows/device_manager.cpp", 
              "src/windows/loopback_stream.cpp", 
              "src/windows/nnl_audio.cpp",
              "nnl_audio.cpp" 
            ],
          "include_dirs": [
              "src",
              "src/windows"
          ],
          "libraries": [
            "windowsapp.lib"
          ]
        }],
        ['OS=="linux"', {
          "sources": [
              "src/linux/nnl_audio.cpp",
              "src/linux/pulse_session.cpp",
              "nnl_audio.cpp"
          ],
          "include_dirs": [
              "src",
              "src/linux"
          ],
          "libraries": [
              "-lpulse",
              "-lpulse-simple"
          ]
        }]
      ]
    }
  ]
}