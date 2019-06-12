# tree 

#### ビルドコマンド メモ 
`g++ -c -std=c++1z test.cpp`  
`g++ test.o -lstdc++fs` 

または  
`g++ -std=c++1z test.cpp -o test.exe -lstdc++fs` 

### Linux 環境準備 
`aurman -S python2`  
`which python2`  
// /usr/bin/python2  
`npm config set python /usr/bin/python2`  
### Windows環境準備 
#### インストール 
##### Visual Studio Build Tools 2017  
https://visualstudio.microsoft.com/ja/downloads/  
##### python 2.7  
https://www.python.org/downloads/windows/  
#### npm config 
`npm config set msvs_version 2017`  
`npm config set python "C:\Python27\python.exe"`  
#### 確認 nodeバージョン 
`v10.16.0`  

### node-gyp 実行
`npm i`  
`npm rebuild node-gyp`  
`npx node-gyp rebuild`  
`node hello.js`  
