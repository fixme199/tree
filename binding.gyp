{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "addon.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "./rapidjson/include"
      ],
      "conditions": [
        ['OS == "win"', {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "AdditionalOptions": [
                "/std:c++17"
              ]
            }
          }
        }], # OS == "win"
        ['OS == "linux"', {
          "cflags_cc": [
            "-std=c++1z"
          ],
          "libraries": [
            "-lstdc++fs"
          ]
        }], # OS == "linux"
      ],
    }
  ]
}
