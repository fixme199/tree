#include <nan.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

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
  std::string rootPath = *arg0;
  fs::path document_path = fs::u8path(rootPath);
  document_path /= "";
  rootPath = document_path.string<char>().c_str();
  std::size_t n = rootPath.size();

  const auto key_path = v8::String::NewFromUtf8(isolate, "path");
  const auto key_parent_path = v8::String::NewFromUtf8(isolate, "parent_path");
  const auto key_filename = v8::String::NewFromUtf8(isolate, "filename");
  const auto key_extension = v8::String::NewFromUtf8(isolate, "extension");
  const auto key_is_directory = v8::String::NewFromUtf8(isolate, "is_directory");
  const auto key_children = v8::String::NewFromUtf8(isolate, "children");

  v8::Local<v8::Object> document = Nan::New<v8::Object>();
  v8::Local<v8::Array> document_children = Nan::New<v8::Array>();
  document->Set(key_path, v8::String::NewFromUtf8(isolate, document_path.u8string().c_str()));
  document->Set(key_parent_path, v8::String::NewFromUtf8(isolate, document_path.parent_path().u8string().c_str()));
  document->Set(key_filename, v8::String::NewFromUtf8(isolate, document_path.filename().u8string().c_str()));
  document->Set(key_extension, v8::String::NewFromUtf8(isolate, document_path.extension().u8string().c_str()));
  document->Set(key_is_directory, Nan::New(true));
  document->Set(key_children, document_children);

  std::error_code ec;
  for (const fs::directory_entry &x : fs::recursive_directory_iterator(rootPath,
      fs::directory_options::skip_permission_denied, ec))
  {
    fs::path path = x.path().string<char>().replace(0, n, "");

    v8::Local<v8::Object> parent = document;
    auto parent_path = --path.end();
    for (auto element = path.begin(); element != parent_path; ++element)
    {
      v8::Local<v8::String> mine = v8::String::NewFromUtf8(isolate, (*element).u8string().c_str());
      if (!Nan::Has(parent, key_children).FromJust())
      {
        v8::Local<v8::Array> children = Nan::New<v8::Array>();
        parent->Set(key_children, children);
      }

      v8::Local<v8::Array> children = v8::Local<v8::Array>::Cast(Nan::Get(parent, key_children).ToLocalChecked());
      int length = children->Length();
      bool found = false;
      for (int i = 0; i < length; i++)
      {
        v8::Local<v8::Object> v = children->Get(i)->ToObject();
        v8::Local<v8::String> filename = Nan::Get(v, key_filename).ToLocalChecked()->ToString();
        if (Nan::Equals(filename, mine).FromJust())
        {
          parent = v;
          found = true;
          break;
        }
      }

      if (!found)
      {
        v8::Local<v8::Object> child = Nan::New<v8::Object>();
        Nan::Set(child, key_path, mine);
        Nan::Set(child, key_parent_path, mine);
        Nan::Set(child, key_filename, mine);
        Nan::Set(child, key_extension, mine);
        Nan::Set(child, key_is_directory, mine);
        Nan::Set(children, length, child);
        parent = child;
      }
    }

    if (!Nan::Has(parent, key_children).FromJust())
    {
      v8::Local<v8::Array> children = Nan::New<v8::Array>();
      parent->Set(key_children, children);
    }

    bool found = false;
    v8::Local<v8::Array> children = v8::Local<v8::Array>::Cast(Nan::Get(parent, key_children).ToLocalChecked());
    int length = children->Length();

    v8::Local<v8::String> mine = v8::String::NewFromUtf8(isolate, x.path().filename().u8string().c_str());
    for (int i = 0; i < length; i++)
    {
      v8::Local<v8::Object> v = children->Get(i)->ToObject();
      v8::Local<v8::String> filename = Nan::Get(v, key_filename).ToLocalChecked()->ToString();
      if (Nan::Equals(filename, mine).FromJust())
      {
        found = true;
        break;
      }
    }

    if (!found)
    {
      v8::Local<v8::Object> child = Nan::New<v8::Object>();
      Nan::Set(child, key_path, v8::String::NewFromUtf8(isolate, x.path().u8string().c_str()));
      Nan::Set(child, key_parent_path, v8::String::NewFromUtf8(isolate, x.path().parent_path().u8string().c_str()));
      Nan::Set(child, key_filename, v8::String::NewFromUtf8(isolate, x.path().filename().u8string().c_str()));
      Nan::Set(child, key_extension, v8::String::NewFromUtf8(isolate, x.path().extension().u8string().c_str()));
      Nan::Set(child, key_is_directory, Nan::New(x.is_directory()));
      Nan::Set(children, length, child);
    }
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

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "hello", Method);
}

NODE_MODULE(addon, init)
