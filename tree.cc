﻿#include <nan.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

static void is_valid_path(const fs::path path)
{

  std::string message = path.u8string();

  std::error_code ec;
  const auto entry = fs::directory_entry{path, ec};
  if (ec)
  {
    std::cout << "error_code [" << ec.value() << "] [" << ec.message() << "]" << std::endl;
    Nan::ThrowError("unexpected error.");
    return;
  }
  std::cout << "entry [" << message << "]" << std::endl;

  ec.clear();
  const fs::file_status status = entry.status(ec);
  if (ec)
  {
    std::cout << "error_code [" << ec.value() << "] [" << ec.message() << "]" << std::endl;
    Nan::ThrowError("cannot read file status.");
    return;
  }
  int permissions = static_cast<int>(status.permissions() & fs::perms::mask);
  std::cout << "permissions [" << permissions << "]" << std::endl;

  ec.clear();
  if (!entry.exists(ec) || ec)
  {
    std::cout << "error_code [" << ec.value() << "] [" << ec.message() << "]" << std::endl;
    message += " not exist.";
    Nan::ThrowError(message.c_str());
  }

  ec.clear();
  if (!entry.is_directory(ec) || ec)
  {
    std::cout << "error_code [" << ec.value() << "] [" << ec.message() << "]" << std::endl;
    message += " is not directory.";
    Nan::ThrowError(message.c_str());
  }
}

static v8::Local<v8::Object> tree(v8::Isolate *isolate, fs::path root_path)
{
  const auto KEY_PATH = v8::String::NewFromUtf8(isolate, "path");
  const auto KEY_PARENT_PATH = v8::String::NewFromUtf8(isolate, "parent_path");
  const auto KEY_FILENAME = v8::String::NewFromUtf8(isolate, "filename");
  const auto KEY_EXTENSION = v8::String::NewFromUtf8(isolate, "extension");
  const auto KEY_IS_DIRECTORY = v8::String::NewFromUtf8(isolate, "is_directory");
  const auto KEY_CHILDREN = v8::String::NewFromUtf8(isolate, "children");

  v8::Local<v8::Object> document = Nan::New<v8::Object>();
  Nan::Set(document, KEY_PATH, v8::String::NewFromUtf8(isolate, root_path.u8string().c_str()));
  Nan::Set(document, KEY_PARENT_PATH, v8::String::NewFromUtf8(isolate, root_path.parent_path().u8string().c_str()));
  Nan::Set(document, KEY_FILENAME, v8::String::NewFromUtf8(isolate, root_path.filename().u8string().c_str()));
  Nan::Set(document, KEY_EXTENSION, v8::String::NewFromUtf8(isolate, root_path.extension().u8string().c_str()));
  Nan::Set(document, KEY_IS_DIRECTORY, Nan::New(true));
  Nan::Set(document, KEY_CHILDREN, Nan::New<v8::Array>());

  root_path /= "";
  const std::string root = root_path.string<char>().c_str();
  const std::size_t n = root.size();

  std::error_code ec;
  const auto iterator = fs::recursive_directory_iterator{root, fs::directory_options::skip_permission_denied, ec};
  if (ec)
  {
    std::cout << "error_code [" << ec.value() << "] [" << ec.message() << "]" << std::endl;
    return document;
  }

  for (const fs::directory_entry &entry : iterator)
  {
    ec.clear();
    fs::file_status status = entry.status(ec);
    if (ec)
    {
      std::cout << entry.path() << "error_code [" << ec.value() << "] [" << ec.message() << "]" << std::endl;
      continue;
    }

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

  return document;
}

static void output(v8::Isolate *isolate, v8::Local<v8::Object> json_object, fs::path output_file)
{
  Nan::JSON NanJSON;
  Nan::MaybeLocal<v8::String> json_maybe_string = NanJSON.Stringify(json_object);
  const v8::Local<v8::String> json_local_string = json_maybe_string.ToLocalChecked();
  const auto json_string = v8::String::Utf8Value{isolate, json_local_string};
  const auto output_data = std::string{*json_string};

  std::ofstream writing_file;
  writing_file.open(output_file.u8string(), std::ios::out);
  writing_file << output_data;
  writing_file.close();
}

class TreeWorker : public Nan::AsyncWorker
{
public:
  TreeWorker(Nan::Callback *callback,
             fs::path root_path,
             fs::path output_file)
      : Nan::AsyncWorker(callback),
        root_path(root_path),
        output_file(output_file) {}
  ~TreeWorker() {}

  void Execute()
  {
  }

  void HandleOKCallback()
  {
    std::cout << "Execute start" << std::endl;
    const auto result = tree(v8::Isolate::GetCurrent(), root_path);
    output(v8::Isolate::GetCurrent(), result, output_file);
    std::cout << "Execute end" << std::endl;
    std::cout << "HandleOKCallback start" << std::endl;
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = {result};
    callback->Call(argc, argv);
    std::cout << "HandleOKCallback end" << std::endl;
  }

private:
  fs::path root_path, output_file;
};

NAN_METHOD(tree)
{
  std::cout << "tree" << std::endl;
  if (info.Length() != 2 && info.Length() != 3)
  {
    // 出力なし(root: string, callback: function)
    // 出力あり(root: string, output: string, callback: function)
    Nan::ThrowRangeError("Wrong number of arguments");
    return;
  }
  if ((info.Length() == 2 && (!info[0]->IsString() || !info[1]->IsFunction())) || (info.Length() == 3 && (!info[0]->IsString() || !info[1]->IsString() || !info[2]->IsFunction())))
  {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  v8::Isolate *isolate = info.GetIsolate();
  const auto arg0 = v8::String::Utf8Value{isolate, info[0]};
  const auto arg1 = v8::String::Utf8Value{isolate, info[1]};
  const fs::path root_path = fs::u8path(*arg0);
  const fs::path output_file = fs::u8path(*arg1);

  is_valid_path(root_path);
  is_valid_path(output_file.parent_path());

  Nan::Callback *callback = new Nan::Callback(info[2].As<v8::Function>());
  Nan::AsyncQueueWorker(new TreeWorker(callback, root_path, output_file));
}

NAN_METHOD(treeSync)
{
  std::cout << "treeSync" << std::endl;
  if (info.Length() != 1 && info.Length() != 2)
  {
    // 出力なし(root: string)
    // 出力あり(root: string, output: string)
    Nan::ThrowRangeError("Wrong number of arguments");
    return;
  }
  if ((info.Length() == 1 && (!info[0]->IsString())) || (info.Length() == 2 && (!info[0]->IsString() || !info[1]->IsString())))
  {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  v8::Isolate *isolate = info.GetIsolate();
  const auto arg0 = v8::String::Utf8Value{isolate, info[0]};
  const auto arg1 = v8::String::Utf8Value{isolate, info[1]};
  const fs::path root_path = fs::u8path(*arg0);
  const fs::path output_file = fs::u8path(*arg1);

  is_valid_path(root_path);
  is_valid_path(output_file.parent_path());

  const auto json_object = tree(isolate, root_path);
  output(isolate, json_object, output_file);

  info.GetReturnValue().Set(json_object);
}

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "tree", tree);
  Nan::SetMethod(target, "treeSync", treeSync);
}

NODE_MODULE(addon, init)
