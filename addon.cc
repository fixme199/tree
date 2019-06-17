#include <nan.h>

#include <iostream>
#include <algorithm>
#include <filesystem>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

namespace fs = std::filesystem;

NAN_METHOD(Method)
{
  if (info.Length() < 1)
  {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }
  if (!info[0]->IsString())
  {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  v8::Isolate *isolate = info.GetIsolate();
  v8::String::Utf8Value arg0(isolate, info[0]);
  std::string rootPath(*arg0);
  std::cout << rootPath << std::endl;
  // Document document;
  // document.SetObject();
  // Document::AllocatorType &allocator = document.GetAllocator();
  //
  // document.AddMember("Name", "XYZ", allocator);
  // document.AddMember("Rollnumer", 2, allocator);
  // Document child;
  // child.SetObject();
  // document.AddMember("child", child, allocator);
  //
  // StringBuffer buffer;
  // Writer<StringBuffer> writer(buffer);
  // document.Accept(writer);
  //
  // std::cout << buffer.GetString() << std::endl;

  std::error_code ec;
  for (const fs::directory_entry &x : fs::recursive_directory_iterator(rootPath,
                                                                       fs::directory_options::skip_permission_denied, ec))
  {
    std::cout << "--- start ---" << std::endl;
    std::cout << "filename : " << x.path().filename() << std::endl;
    std::cout << "parent_path : " << x.path().parent_path() << std::endl;
    std::cout << "stem : " << x.path().stem() << std::endl;
    std::cout << "extension : " << x.path().extension() << std::endl;
    std::cout << "path : " << x.path() << std::endl;
    std::cout << "is_directory : " << x.is_directory() << std::endl;
    std::cout << "--- end ---" << std::endl;
  }
  info.GetReturnValue().Set(Nan::New("world").ToLocalChecked());
}

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "hello", Method);
}

NODE_MODULE(addon, init)
