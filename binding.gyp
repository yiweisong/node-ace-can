{
  "targets": [
    {
      "target_name": "ace_can",
      "sources": [ "src/ace_can.cpp" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "deps/busmust/include",
        "deps/pcan/include"
      ],
      "libraries": [
        "<(module_root_dir)/deps/busmust/lib/x64/BMAPI64.lib",
        "<(module_root_dir)/deps/pcan/lib/x64/PCANBasic.lib"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "dependencies": [ "<!(node -p \"require('node-addon-api').gyp\")" ],
      "conditions": [
        ['OS=="win"',
          {
            'msvs_settings': {
              'VCCLCompilerTool': { 'ExceptionHandling': 1 },
              'VCLinkerTool':{
                'DelayLoadDLLs':['BMAPI64.dll','PCANBasic.dll']
              }
            }
          }
        ]
      ]
    }
  ]
}
