#include <nan.h>

#include <iostream>
#include <algorithm>
#include <filesystem>

namespace demo
{

namespace fs = std::filesystem;

NAN_METHOD(Method)
{
	std::error_code ec;
  for (const fs::directory_entry &x : fs::recursive_directory_iterator(".",
                                                                       fs::directory_options::skip_permission_denied, ec))
  {
    std::cout << "exists : " << x.exists() << std::endl;
    std::cout << "path : " << x.path() << std::endl;
    std::cout << "is_directory : " << x.is_directory() << std::endl;
  }
  info.GetReturnValue().Set(Nan::New("world").ToLocalChecked());
}

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "hello", Method);
}

NODE_MODULE(addon, init)
} // namespace demo
