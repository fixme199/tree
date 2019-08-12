# tree-json 

## Introduction  
### Description  
Represent directory tree with json or object.

### Demo
<details>
<summary>for example. windows tree command.</summary>
<pre>
<code>
C:.
+---documents
|   |   a.txt
|   |   b.txt
|   |   c.txt
|   |
|   +---app
|   |       app_a.txt
|   |       app_b.txt
|   |       app_c.txt
|   |
|   +---music
|   \---video
+---downloads
|   |   a.exe
|   |   b.exe
|   |
|   \---test
|           test_a.exe
|           test_b.exe
|
\---favorites
</code>
</pre>
</details>

<details>
<summary>on the other hand this module.</summary>
<pre>
<code>
{
    "path": ".\\demo",
    "parent_path": ".",
    "filename": "demo",
    "extension": "",
    "is_directory": true,
    "children": [
        {
            "path": ".\\demo\\documents",
            "parent_path": ".\\demo",
            "filename": "documents",
            "extension": "",
            "is_directory": true,
            "children": [
                {
                    "path": ".\\demo\\documents\\a.txt",
                    "parent_path": ".\\demo\\documents",
                    "filename": "a.txt",
                    "extension": ".txt",
                    "is_directory": false,
                    "children": []
                },
                {
                    "path": ".\\demo\\documents\\app",
                    "parent_path": ".\\demo\\documents",
                    "filename": "app",
                    "extension": "",
                    "is_directory": true,
                    "children": [
                        {
                            "path": ".\\demo\\documents\\app\\app_a.txt",
                            "parent_path": ".\\demo\\documents\\app",
                            "filename": "app_a.txt",
                            "extension": ".txt",
                            "is_directory": false,
                            "children": []
                        },
                        {
                            "path": ".\\demo\\documents\\app\\app_b.txt",
                            "parent_path": ".\\demo\\documents\\app",
                            "filename": "app_b.txt",
                            "extension": ".txt",
                            "is_directory": false,
                            "children": []
                        },
                        {
                            "path": ".\\demo\\documents\\app\\app_c.txt",
                            "parent_path": ".\\demo\\documents\\app",
                            "filename": "app_c.txt",
                            "extension": ".txt",
                            "is_directory": false,
                            "children": []
                        }
                    ]
                },
                {
                    "path": ".\\demo\\documents\\b.txt",
                    "parent_path": ".\\demo\\documents",
                    "filename": "b.txt",
                    "extension": ".txt",
                    "is_directory": false,
                    "children": []
                },
                {
                    "path": ".\\demo\\documents\\c.txt",
                    "parent_path": ".\\demo\\documents",
                    "filename": "c.txt",
                    "extension": ".txt",
                    "is_directory": false,
                    "children": []
                },
                {
                    "path": ".\\demo\\documents\\music",
                    "parent_path": ".\\demo\\documents",
                    "filename": "music",
                    "extension": "",
                    "is_directory": true,
                    "children": []
                },
                {
                    "path": ".\\demo\\documents\\video",
                    "parent_path": ".\\demo\\documents",
                    "filename": "video",
                    "extension": "",
                    "is_directory": true,
                    "children": []
                }
            ]
        },
        {
            "path": ".\\demo\\downloads",
            "parent_path": ".\\demo",
            "filename": "downloads",
            "extension": "",
            "is_directory": true,
            "children": [
                {
                    "path": ".\\demo\\downloads\\a.exe",
                    "parent_path": ".\\demo\\downloads",
                    "filename": "a.exe",
                    "extension": ".exe",
                    "is_directory": false,
                    "children": []
                },
                {
                    "path": ".\\demo\\downloads\\b.exe",
                    "parent_path": ".\\demo\\downloads",
                    "filename": "b.exe",
                    "extension": ".exe",
                    "is_directory": false,
                    "children": []
                },
                {
                    "path": ".\\demo\\downloads\\test",
                    "parent_path": ".\\demo\\downloads",
                    "filename": "test",
                    "extension": "",
                    "is_directory": true,
                    "children": [
                        {
                            "path": ".\\demo\\downloads\\test\\test_a.exe",
                            "parent_path": ".\\demo\\downloads\\test",
                            "filename": "test_a.exe",
                            "extension": ".exe",
                            "is_directory": false,
                            "children": []
                        },
                        {
                            "path": ".\\demo\\downloads\\test\\test_b.exe",
                            "parent_path": ".\\demo\\downloads\\test",
                            "filename": "test_b.exe",
                            "extension": ".exe",
                            "is_directory": false,
                            "children": []
                        }
                    ]
                }
            ]
        },
        {
            "path": ".\\demo\\favorites",
            "parent_path": ".\\demo",
            "filename": "favorites",
            "extension": "",
            "is_directory": true,
            "children": []
        }
    ]
}
</code>
</pre>
</details>

## How to use  
### Install  
```
npm i tree-json
```

### Usage  
```
const { tree } = require('tree-json');
const object = tree('.\\demo'); // return object only.
const json = tree('.\\demo', '.\\output.json'); // and output json.
```

## Develop  
### Linux 環境準備 
`aurman -S python2`  
`which python2`  
`npm config set python /usr/bin/python2`  

`% gcc --version`  
`gcc (GCC) 9.1.0`  
`% g++ --version`  
`g++ (GCC) 9.1.0`

### Windows環境準備 
#### インストール 
`npm i -g windows-build-tools`  

##### Visual Studio Build Tools 2017  
##### python 2.7  

#### npm config 
`npm config set msvs_version 2017`  
`npm config set python "C:\Python27\python.exe"`  

### node-gyp 実行
`npm i`  
`npm rebuild node-gyp`  
`npm run build`  
`npm run test`  
