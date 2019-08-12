#include <nan.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/reader.h"
#include "rapidjson/pointer.h"

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

static void output(rapidjson::Document document, fs::path output_file)
{
  FILE *fp = fopen("output.json", "wb"); // non-Windows use "w"
  char writeBuffer[65536];
  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
  document.Accept(writer);
  fclose(fp);
}

static rapidjson::Document tree(fs::path root_path)
{
  rapidjson::Document document;
  document.SetObject();
  rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

  document.AddMember("path", rapidjson::StringRef(root_path.u8string().c_str()), allocator);
  document.AddMember("parent_path", rapidjson::StringRef(root_path.parent_path().u8string().c_str()), allocator);
  document.AddMember("filename", rapidjson::StringRef(root_path.filename().u8string().c_str()), allocator);
  document.AddMember("extension", rapidjson::StringRef(root_path.extension().u8string().c_str()), allocator);
  document.AddMember("is_directory", true, allocator);
  document.AddMember("children", rapidjson::Value{rapidjson::kArrayType}, allocator);

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

    std::string pointer = "/children";
    const rapidjson::Value &document_children = document["children"];
    size_t length = document_children.Size();

    for (auto it = parent_path.begin(), e = parent_path.end(); it != e; ++it)
    {
      pointer += "/";
      pointer += std::to_string(length - 1);
      const rapidjson::Value &parent = rapidjson::GetValueByPointerWithDefault(document, rapidjson::Pointer(pointer.c_str()), rapidjson::Value{rapidjson::kObjectType});
      const rapidjson::Value &children = parent["children"];
      length = children.Size();
      pointer += "/children";
    }

    rapidjson::Value &child = rapidjson::Value{rapidjson::kObjectType};
    child.AddMember("path", rapidjson::StringRef(entry.path().u8string().c_str()), allocator);
    child.AddMember("parent_path", rapidjson::StringRef(entry.path().parent_path().u8string().c_str()), allocator);
    child.AddMember("filename", rapidjson::StringRef(entry.path().filename().u8string().c_str()), allocator);
    child.AddMember("extension", rapidjson::StringRef(entry.path().extension().u8string().c_str()), allocator);
    child.AddMember("is_directory", entry.is_directory(), allocator);
    child.AddMember("children", rapidjson::Value{rapidjson::kArrayType}, allocator);

    rapidjson::Value &children = rapidjson::GetValueByPointerWithDefault(document, rapidjson::Pointer(pointer.c_str()), rapidjson::Value{rapidjson::kArrayType});
    children.PushBack(child, allocator);
  }

  rapidjson::StringBuffer ws;
  rapidjson::Writer<rapidjson::StringBuffer> writer(ws);
  document.Accept(writer);
  const char *result = ws.GetString();
  std::cout << result << std::endl;

  return document;
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
    std::cout << "Execute start" << std::endl;
    rapidjson::Document result = tree(root_path);
    std::cout << "Execute end" << std::endl;
  }

  void HandleOKCallback()
  {
    std::cout << "HandleOKCallback start" << std::endl;
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = {/* result */};
    callback->Call(argc, argv);
    std::cout << "HandleOKCallback end" << std::endl;
  }

private:
  fs::path root_path;
  fs::path output_file;
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
  // if (info.Length() != 1 && info.Length() != 2)
  // {
  //   // 出力なし(root: string)
  //   // 出力あり(root: string, output: string)
  //   Nan::ThrowRangeError("Wrong number of arguments");
  //   return;
  // }
  // if ((info.Length() == 1 && (!info[0]->IsString())) || (info.Length() == 2 && (!info[0]->IsString() || !info[1]->IsString())))
  // {
  //   Nan::ThrowTypeError("Wrong arguments");
  //   return;
  // }

  // v8::Isolate *isolate = info.GetIsolate();
  // const auto arg0 = v8::String::Utf8Value{isolate, info[0]};
  // const auto arg1 = v8::String::Utf8Value{isolate, info[1]};
  // const fs::path root_path = fs::u8path(*arg0);
  // const fs::path output_file = fs::u8path(*arg1);

  // is_valid_path(root_path);
  // is_valid_path(output_file.parent_path());

  // const auto json_object = tree(root_path);
  // output(json_object, output_file);

  // info.GetReturnValue().Set(json_object);
}

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "tree", tree);
  Nan::SetMethod(target, "treeSync", treeSync);
}

NODE_MODULE(addon, init)
