#include <nan.h>

#include <iostream>
#include <algorithm>
#include <filesystem>

using namespace v8;

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

  Isolate *isolate = info.GetIsolate();
  String::Utf8Value arg0(isolate, info[0]);
  std::string rootPath(*arg0);
  std::cout << rootPath << std::endl;

  Local<Object> document = Object::New(isolate);

  auto key_path = String::NewFromUtf8(isolate, "path");
  auto key_parent_path = String::NewFromUtf8(isolate, "parent_path");
  auto key_filename = String::NewFromUtf8(isolate, "filename");
  auto key_extension = String::NewFromUtf8(isolate, "extension");
  auto key_is_directory = String::NewFromUtf8(isolate, "is_directory");

  std::error_code ec;
  for (const fs::directory_entry &x : fs::recursive_directory_iterator(rootPath,
                                                                       fs::directory_options::skip_permission_denied, ec))
  {
    auto path = String::NewFromUtf8(isolate, x.path().string<char>().c_str());
    auto parent_path = String::NewFromUtf8(isolate, x.path().parent_path().string<char>().c_str());
    auto filename = String::NewFromUtf8(isolate, x.path().filename().string<char>().c_str());
    auto extension = String::NewFromUtf8(isolate, x.path().extension().string<char>().c_str());
    auto is_directory = Boolean::New(isolate, x.is_directory());

    Local<Object> child = Object::New(isolate);
    child->Set(key_path, path);
    child->Set(key_parent_path, parent_path);
    child->Set(key_filename, filename);
    child->Set(key_extension, extension);
    child->Set(key_is_directory, is_directory);

    document->Set(path, child);
  }

  info.GetReturnValue().Set(document);
}

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "hello", Method);
}

NODE_MODULE(addon, init)
