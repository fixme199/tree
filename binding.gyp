{
  "targets": [
    {
      "target_name": "tree",
      "sources": [
        "src/tree.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
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
