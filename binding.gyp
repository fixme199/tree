{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "addon.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        ['OS == "win"', {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "AdditionalOptions": [
                "/std:c++latest"
              ]
            }
          }
        }], # OS == "win"
        ['OS == "mac"', {
          "cflags_cc": [
            "-std=c++1z"
          ],
          "libraries": [
            "-lstdc++fs"
          ]
        }], # OS == "mac"
      ],
    }
  ]
}
