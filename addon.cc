#include <nan.h>

#include <iostream>
#include <fstream>
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

  auto key_path = String::NewFromUtf8(isolate, "path");
  auto key_parent_path = String::NewFromUtf8(isolate, "parent_path");
  auto key_filename = String::NewFromUtf8(isolate, "filename");
  auto key_extension = String::NewFromUtf8(isolate, "extension");
  auto key_is_directory = String::NewFromUtf8(isolate, "is_directory");
  auto key_children = String::NewFromUtf8(isolate, "children");

  Local<Object> document = Nan::New<Object>();
  Local<Array> document_children = Nan::New<Array>();
  fs::path document_path = rootPath;
  document_path /= "";
  rootPath = document_path.string<char>().c_str();
  std::cout << "root path = " << rootPath << std::endl;
  std::size_t n = rootPath.size();
  document->Set(key_path, String::NewFromUtf8(isolate, document_path.string<char>().c_str()));
  document->Set(key_parent_path, String::NewFromUtf8(isolate, document_path.parent_path().string<char>().c_str()));
  document->Set(key_filename, String::NewFromUtf8(isolate, document_path.filename().string<char>().c_str()));
  document->Set(key_extension, String::NewFromUtf8(isolate, document_path.extension().string<char>().c_str()));
  document->Set(key_is_directory, Nan::New(true));
  document->Set(key_children, document_children);

  std::error_code ec;
  for (const fs::directory_entry &x : fs::recursive_directory_iterator(rootPath,
                                                                       fs::directory_options::skip_permission_denied, ec))
  {
    // ルートディレクトリを削除
    fs::path path = x.path().string<char>().replace(0, n, "");
    std::cout << "--- recursive directory iterator ---" << std::endl;
    std::cout << path << std::endl;

    // 親ディレクトリまで探索
    Local<Object> parent = document;
    auto parent_path = --path.end();
    for (auto element = path.begin(); element != parent_path; ++element)
    {
      std::cout << "--- path iterator ---" << std::endl;
      std::cout << *element << std::endl;
      Local<String> mine = String::NewFromUtf8(isolate, (*element).string<char>().c_str());
      // key children がなければ追加
      if (!Nan::Has(parent, key_children).FromJust())
      {
        // std::cout << "!FromJust" << std::endl;
        Local<Array> children = Nan::New<Array>();
        parent->Set(key_children, children);
      }

      Local<Array> children = Local<Array>::Cast(Nan::Get(parent, key_children).ToLocalChecked());
      int length = children->Length();
      bool found = false;
      for (int i = 0; i < length; i++)
      {
        Local<Object> v = children->Get(i)->ToObject();
        Local<String> filename = Nan::Get(v, key_filename).ToLocalChecked()->ToString();
        if (Nan::Equals(filename, mine).FromJust())
        {
          // std::cout << "FromJust" << std::endl;
          parent = v;
          found = true;
          break;
        }
      }

      if (!found)
      {
        // std::cout << "not found" << std::endl;
        Local<Object> child = Nan::New<Object>();
        Nan::Set(child, key_path, mine);
        Nan::Set(child, key_parent_path, mine);
        Nan::Set(child, key_filename, mine);
        Nan::Set(child, key_extension, mine);
        Nan::Set(child, key_is_directory, mine);
        Nan::Set(children, length, child);
        parent = child;
      }
    }

    // key children がなければ追加
    if (!Nan::Has(parent, key_children).FromJust())
    {
      // std::cout << "!FromJust" << std::endl;
      Local<Array> children = Nan::New<Array>();
      parent->Set(key_children, children);
    }

    bool found = false;
    Local<Array> children = Local<Array>::Cast(Nan::Get(parent, key_children).ToLocalChecked());
    int length = children->Length();

    Local<String> mine = String::NewFromUtf8(isolate, x.path().filename().string<char>().c_str());
    for (int i = 0; i < length; i++)
    {
      Local<Object> v = children->Get(i)->ToObject();
      Local<String> filename = Nan::Get(v, key_filename).ToLocalChecked()->ToString();
      if (Nan::Equals(filename, mine).FromJust())
      {
        // std::cout << "FromJust" << std::endl;
        found = true;
        break;
      }
    }

    if (!found)
    {
      // std::cout << "not found" << std::endl;
      Local<Object> child = Nan::New<Object>();
      Nan::Set(child, key_path, String::NewFromUtf8(isolate, x.path().string<char>().c_str()));
      Nan::Set(child, key_parent_path, String::NewFromUtf8(isolate, x.path().parent_path().string<char>().c_str()));
      Nan::Set(child, key_filename, String::NewFromUtf8(isolate, x.path().filename().string<char>().c_str()));
      Nan::Set(child, key_extension, String::NewFromUtf8(isolate, x.path().extension().string<char>().c_str()));
      Nan::Set(child, key_is_directory, Nan::New(x.is_directory()));
      Nan::Set(children, length, child);
    }
  }

  std::cout << "write " << " start " << "..." << std::endl;
  Nan::JSON NanJSON;
  Nan::MaybeLocal<v8::String> result_document = NanJSON.Stringify(document);
  v8::Local<v8::String> stringified = result_document.ToLocalChecked();
  String::Utf8Value hoge(isolate, stringified);
  std::string out_hoge(*hoge);
  std::cout << "stringified " << " end " << "..." << std::endl;
  std::cout << out_hoge << std::endl;

  std::string output = "output.json";
  std::ofstream writing_file;
  writing_file.open(output, std::ios::out);
  std::cout << "writing " << output << "..." << std::endl;
  writing_file << out_hoge;
  writing_file.close();
  std::cout << "JSON file has been saved " << " end " << "..." << std::endl;

  info.GetReturnValue().Set(document);
}

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "hello", Method);
}

NODE_MODULE(addon, init)
