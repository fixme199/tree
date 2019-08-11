#include <nan.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

static v8::Local<v8::Object> tree(v8::Isolate *isolate, fs::path root_path)
{
  const auto KEY_PATH = v8::String::NewFromUtf8(isolate, "path");
  const auto KEY_PARENT_PATH = v8::String::NewFromUtf8(isolate, "parent_path");
  const auto KEY_FILENAME = v8::String::NewFromUtf8(isolate, "filename");
  const auto KEY_EXTENSION = v8::String::NewFromUtf8(isolate, "extension");
  const auto KEY_IS_DIRECTORY = v8::String::NewFromUtf8(isolate, "is_directory");
  const auto KEY_CHILDREN = v8::String::NewFromUtf8(isolate, "children");

  const auto document = Nan::New<v8::Object>();
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

  return document;
}

// info[0] : root path
// info[1] : out path
// info[2] : callback
NAN_METHOD(tree)
{
  if (info.Length() != 3)
  {
    std::cout << "Length" << std::endl;
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }
  if (!info[0]->IsString() || !info[1]->IsString() || !info[2]->IsFunction())
  {
    std::cout << "type" << std::endl;
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  v8::Isolate *isolate = info.GetIsolate();
  const v8::String::Utf8Value arg0{isolate, info[0]};
  const v8::String::Utf8Value arg1{isolate, info[0]};
  const fs::path root_path = fs::u8path(*arg0);
  const fs::path output_path = fs::u8path(*arg1);

  std::error_code ec;
  if (!fs::exists(root_path, ec) || ec)
  {
    std::cout << root_path << " not exists. [" << ec.value() << "] " << ec.message() << std::endl;
    Nan::ThrowTypeError("root not exists.");
    return;
  }

  ec.clear();
  if (!fs::is_directory(root_path, ec) || ec)
  {
    std::cout << root_path << " is not directory. [" << ec.value() << "] " << ec.message() << std::endl;
    Nan::ThrowTypeError("root is not directory.");
    return;
  }

  ec.clear();
  if (!fs::exists(output_path.parent_path(), ec) || ec)
  {
    std::cout << output_path.parent_path() << " not exists. [" << ec.value() << "] " << ec.message() << std::endl;
    Nan::ThrowTypeError("root not exists.");
    return;
  }

  ec.clear();
  if (!fs::is_directory(output_path.parent_path(), ec) || ec)
  {
    std::cout << output_path.parent_path() << " is not directory. [" << ec.value() << "] " << ec.message() << std::endl;
    Nan::ThrowTypeError("root is not directory.");
    return;
  }

  v8::Local<v8::Object> document = tree(isolate, root_path);
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

NAN_METHOD(treeSync)
{
  std::cout << "treeSync" << std::endl;
}

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "tree", tree);
  Nan::SetMethod(target, "treeSync", treeSync);
}

NODE_MODULE(addon, init)
