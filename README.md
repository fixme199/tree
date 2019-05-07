# tree

test.cpp
```C++
#include <stdio.h>
#include <iostream>
#include <filesystem>

class test{
public :
	void out(){
	printf("test test");
	}
};

namespace fs = std::filesystem;

int main(char*args []){
	test t;
	t.out();

	std::error_code ec;
  for (const fs::directory_entry &x : fs::recursive_directory_iterator(".",
                                                                       fs::directory_options::skip_permission_denied, ec))
  {
    std::cout << "exists : " << x.exists() << std::endl;
    std::cout << "path : " << x.path() << std::endl;
    std::cout << "is_directory : " << x.is_directory() << std::endl;
  }
	getchar();
	return 0;
}
```
コマンド  
`g++ -c -std=c++1z test.cpp`  
`g++ test.o -lstdc++fs`
