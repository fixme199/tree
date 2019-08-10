#include <nan.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

NAN_METHOD(hello)
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
  const auto KEY_PATH = v8::String::NewFromUtf8(isolate, "path");
  const auto KEY_PARENT_PATH = v8::String::NewFromUtf8(isolate, "parent_path");
  const auto KEY_FILENAME = v8::String::NewFromUtf8(isolate, "filename");
  const auto KEY_EXTENSION = v8::String::NewFromUtf8(isolate, "extension");
  const auto KEY_IS_DIRECTORY = v8::String::NewFromUtf8(isolate, "is_directory");
  const auto KEY_CHILDREN = v8::String::NewFromUtf8(isolate, "children");

  const v8::String::Utf8Value arg0{isolate, info[0]};
  fs::path root_path = fs::u8path(*arg0);

  const auto document = Nan::New<v8::Object>();
  document->Set(KEY_PATH, v8::String::NewFromUtf8(isolate, root_path.u8string().c_str()));
  document->Set(KEY_PARENT_PATH, v8::String::NewFromUtf8(isolate, root_path.parent_path().u8string().c_str()));
  document->Set(KEY_FILENAME, v8::String::NewFromUtf8(isolate, root_path.filename().u8string().c_str()));
  document->Set(KEY_EXTENSION, v8::String::NewFromUtf8(isolate, root_path.extension().u8string().c_str()));
  document->Set(KEY_IS_DIRECTORY, Nan::New(true));
  document->Set(KEY_CHILDREN, Nan::New<v8::Array>());

  root_path /= "";
  const std::string root = root_path.string<char>().c_str();
  const std::size_t n = root.size();

  std::error_code ec;
  for (const fs::directory_entry &entry : fs::recursive_directory_iterator{root,
                                                                           fs::directory_options::skip_permission_denied, ec})
  {
    const fs::path path = entry.path().string<char>().replace(0, n, "");
    const fs::path parent_path = path.parent_path();

    v8::Local<v8::Array> children = v8::Local<v8::Array>::Cast(Nan::Get(document, KEY_CHILDREN).ToLocalChecked());
    int length = children->Length();
    for (auto it = parent_path.begin(), e = parent_path.end(); it != e; ++it)
    {
      const auto parent = children->Get(length - 1)->ToObject();
      children = v8::Local<v8::Array>::Cast(Nan::Get(parent, KEY_CHILDREN).ToLocalChecked());
      length = children->Length();
    }

    const auto child = Nan::New<v8::Object>();
    Nan::Set(child, KEY_PATH, v8::String::NewFromUtf8(isolate, entry.path().u8string().c_str()));
    Nan::Set(child, KEY_PARENT_PATH, v8::String::NewFromUtf8(isolate, entry.path().parent_path().u8string().c_str()));
    Nan::Set(child, KEY_FILENAME, v8::String::NewFromUtf8(isolate, entry.path().filename().u8string().c_str()));
    Nan::Set(child, KEY_EXTENSION, v8::String::NewFromUtf8(isolate, entry.path().extension().u8string().c_str()));
    Nan::Set(child, KEY_IS_DIRECTORY, Nan::New(entry.is_directory()));
    Nan::Set(child, KEY_CHILDREN, Nan::New<v8::Array>());
    Nan::Set(children, length, child);
  }

  Nan::JSON NanJSON;
  Nan::MaybeLocal<v8::String> result_document = NanJSON.Stringify(document);
  v8::Local<v8::String> stringified = result_document.ToLocalChecked();
  v8::String::Utf8Value hoge(isolate, stringified);
  std::string out_hoge(*hoge);

  std::string output = "output.json";
  std::ofstream writing_file;
  writing_file.open(output, std::ios::out);
  writing_file << out_hoge;
  writing_file.close();

  info.GetReturnValue().Set(document);
}

NAN_METHOD(world)
{
  std::cout << "world" << std::endl;
}

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "hello", hello);
  Nan::SetMethod(target, "world", world);
}

NODE_MODULE(addon, init)

static v8::Local<v8::Object> directory_entry(v8::Local<v8::Object> object, fs::path path)
{
  return object;
}
