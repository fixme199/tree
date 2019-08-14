#include <nan.h>

#include <fstream>
#include <filesystem>

#include <picojson.h>

namespace fs = std::filesystem;

static void is_valid_directory(const fs::path path)
{
  std::string message = path.u8string();

  std::error_code ec;
  const auto entry = fs::directory_entry{path, ec};
  if (ec)
  {
    Nan::ThrowError("unexpected error.");
    return;
  }

  ec.clear();
  entry.status(ec);
  if (ec)
  {
    message += " cannot read file status.";
    Nan::ThrowError(message.c_str());
    return;
  }

  ec.clear();
  if (!entry.exists(ec) || ec)
  {
    message += " not exist.";
    Nan::ThrowError(message.c_str());
    return;
  }

  ec.clear();
  if (!entry.is_directory(ec) || ec)
  {
    message += " is not directory.";
    Nan::ThrowError(message.c_str());
    return;
  }
}

static void output(std::string json_string, fs::path output_file)
{
  std::ofstream writing_file;
  writing_file.open(output_file.u8string(), std::ios::out);
  writing_file << json_string;
  writing_file.close();
}

class AsyncTreeWorker : public Nan::AsyncWorker
{
private:
  fs::path root_path;
  fs::path output_file;
  std::string json_string;

public:
  AsyncTreeWorker(Nan::Callback *callback,
                  fs::path root_path,
                  fs::path output_file)
      : Nan::AsyncWorker(callback),
        root_path(root_path),
        output_file(output_file) {}
  ~AsyncTreeWorker() {}

  void Execute()
  {
    json_string = tree(root_path);
    if (!output_file.empty())
    {
      output(json_string, output_file);
    }
  }

  void HandleOKCallback()
  {
    v8::Local<v8::String> json = v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), json_string.c_str());
    Nan::JSON NanJSON;
    v8::Local<v8::Value> result = NanJSON.Parse(json).ToLocalChecked();
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = {result};
    callback->Call(argc, argv);
  }

private:
  static void entry_to_object(const fs::directory_entry &entry, picojson::object &object)
  {
    object.insert(std::make_pair("path", picojson::value(entry.path().u8string().c_str())));
    object.insert(std::make_pair("parent_path", picojson::value(entry.path().parent_path().u8string().c_str())));
    object.insert(std::make_pair("filename", picojson::value(entry.path().filename().u8string().c_str())));
    object.insert(std::make_pair("extension", picojson::value(entry.path().extension().u8string().c_str())));
    object.insert(std::make_pair("is_directory", picojson::value(entry.is_directory())));
    picojson::array children;
    object.insert(std::make_pair("children", picojson::value(children)));
  }

  static picojson::object &find_parent(picojson::object &object, int distance)
  {
    if (distance == 0)
    {
      return object;
    }
    else
    {
      return find_parent(object["children"].get<picojson::array>().back().get<picojson::object>(), distance - 1);
    }
  }

  static std::string tree(fs::path root_path)
  {
    std::error_code ec;
    const fs::directory_entry &document_entry = fs::directory_entry{root_path, ec};

    picojson::object document;
    entry_to_object(document_entry, document);

    root_path /= "";
    const std::string root = root_path.string<char>().c_str();
    const std::size_t n = root.size();

    ec.clear();
    const auto iterator = fs::recursive_directory_iterator{root, fs::directory_options::skip_permission_denied, ec};
    if (ec)
    {
      return picojson::value(document).serialize();
    }

    for (const fs::directory_entry &child_entry : iterator)
    {
      ec.clear();
      fs::file_status status = child_entry.status(ec);
      if (ec)
      {
        continue;
      }

      const fs::path path = child_entry.path().string<char>().replace(0, n, "");
      const fs::path parent_path = path.parent_path();

      auto distance = std::distance(parent_path.begin(), parent_path.end());
      picojson::object &parent = find_parent(document, distance);
      picojson::array &children = parent["children"].get<picojson::array>();

      picojson::object child;
      entry_to_object(child_entry, child);
      children.push_back(picojson::value(child));
    }

    return picojson::value(document).serialize();
  }
};

class SyncTreeWorker
{
private:
  v8::Local<v8::String> KEY_PATH, KEY_PARENT_PATH, KEY_FILENAME, KEY_EXTENSION, KEY_IS_DIRECTORY, KEY_CHILDREN;
  fs::path root_path;
  fs::path output_file;

public:
  SyncTreeWorker(fs::path root_path,
                 fs::path output_file)
      : root_path(root_path),
        output_file(output_file)
  {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    KEY_PATH = v8::String::NewFromUtf8(isolate, "path");
    KEY_PARENT_PATH = v8::String::NewFromUtf8(isolate, "parent_path");
    KEY_FILENAME = v8::String::NewFromUtf8(isolate, "filename");
    KEY_EXTENSION = v8::String::NewFromUtf8(isolate, "extension");
    KEY_IS_DIRECTORY = v8::String::NewFromUtf8(isolate, "is_directory");
    KEY_CHILDREN = v8::String::NewFromUtf8(isolate, "children");
  }
  ~SyncTreeWorker() {}

  v8::Local<v8::Object> Execute()
  {
    v8::Local<v8::Object> json_object = tree(root_path);
    if (!output_file.empty())
    {
      Nan::JSON NanJSON;
      Nan::MaybeLocal<v8::String> json_maybe_string = NanJSON.Stringify(json_object);
      const v8::Local<v8::String> json_local_string = json_maybe_string.ToLocalChecked();
      const auto json_v8_string = v8::String::Utf8Value{v8::Isolate::GetCurrent(), json_local_string};
      const auto json_string = std::string{*json_v8_string};
      output(json_string, output_file);
    }
    return json_object;
  }

private:
  void entry_to_object(const fs::directory_entry &entry, v8::Local<v8::Object> &object)
  {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Nan::Set(object, KEY_PATH, v8::String::NewFromUtf8(isolate, entry.path().u8string().c_str()));
    Nan::Set(object, KEY_PARENT_PATH, v8::String::NewFromUtf8(isolate, entry.path().parent_path().u8string().c_str()));
    Nan::Set(object, KEY_FILENAME, v8::String::NewFromUtf8(isolate, entry.path().filename().u8string().c_str()));
    Nan::Set(object, KEY_EXTENSION, v8::String::NewFromUtf8(isolate, entry.path().extension().u8string().c_str()));
    Nan::Set(object, KEY_IS_DIRECTORY, Nan::New(entry.is_directory()));
    Nan::Set(object, KEY_CHILDREN, Nan::New<v8::Array>());
  }

  v8::Local<v8::Object> tree(fs::path root_path)
  {
    std::error_code ec;
    const fs::directory_entry &document_entry = fs::directory_entry{root_path, ec};

    v8::Local<v8::Object> document = Nan::New<v8::Object>();
    entry_to_object(document_entry, document);

    root_path /= "";
    const std::string root = root_path.string<char>().c_str();
    const std::size_t n = root.size();

    ec.clear();
    const auto iterator = fs::recursive_directory_iterator{root, fs::directory_options::skip_permission_denied, ec};
    if (ec)
    {
      return document;
    }

    for (const fs::directory_entry &child_entry : iterator)
    {
      ec.clear();
      fs::file_status status = child_entry.status(ec);
      if (ec)
      {
        continue;
      }

      const fs::path path = child_entry.path().string<char>().replace(0, n, "");
      const fs::path parent_path = path.parent_path();

      v8::Local<v8::Array> children = v8::Local<v8::Array>::Cast(Nan::Get(document, KEY_CHILDREN).ToLocalChecked());
      int length = children->Length();
      for (auto it = parent_path.begin(), e = parent_path.end(); it != e; ++it)
      {
        const auto parent = children->Get(length - 1)->ToObject();
        children = v8::Local<v8::Array>::Cast(Nan::Get(parent, KEY_CHILDREN).ToLocalChecked());
        length = children->Length();
      }

      v8::Local<v8::Object> child = Nan::New<v8::Object>();
      entry_to_object(child_entry, child);
      Nan::Set(children, children->Length(), child);
    }

    return document;
  }
};

NAN_METHOD(tree)
{
  if (info.Length() != 2 && info.Length() != 3)
  {
    Nan::ThrowRangeError("Wrong number of arguments");
    return;
  }
  if ((!info[0]->IsString()) || (info.Length() == 2 && !info[1]->IsFunction()) || (info.Length() == 3 && (!info[1]->IsString() || !info[2]->IsFunction())))
  {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  v8::Isolate *isolate = info.GetIsolate();

  const auto arg0 = v8::String::Utf8Value{isolate, info[0]};
  fs::path root_path = fs::u8path(*arg0); // root path 省略不可
  is_valid_directory(root_path);

  fs::path output_file = fs::path{}; // output file 省略可
  if (info.Length() == 3)
  {
    const auto arg1 = v8::String::Utf8Value{isolate, info[1]};
    output_file = fs::u8path(*arg1);
    is_valid_directory(output_file.parent_path());
  }

  Nan::Callback *callback = info.Length() == 2 // callback 省略不可
                                ? new Nan::Callback(info[1].As<v8::Function>())
                                : new Nan::Callback(info[2].As<v8::Function>());
  Nan::AsyncQueueWorker(new AsyncTreeWorker(callback, root_path, output_file));
}

NAN_METHOD(treeSync)
{
  if (info.Length() != 1 && info.Length() != 2)
  {
    Nan::ThrowRangeError("Wrong number of arguments");
    return;
  }
  if ((!info[0]->IsString()) || (info.Length() == 2 && !info[1]->IsString()))
  {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  v8::Isolate *isolate = info.GetIsolate();

  const auto arg0 = v8::String::Utf8Value{isolate, info[0]};
  fs::path root_path = fs::u8path(*arg0); // root path 省略不可
  is_valid_directory(root_path);

  fs::path output_file = fs::path{}; // output file 省略可
  if (info.Length() == 2)
  {
    const auto arg1 = v8::String::Utf8Value{isolate, info[1]};
    output_file = fs::u8path(*arg1);
    is_valid_directory(output_file.parent_path());
  }

  SyncTreeWorker worker = SyncTreeWorker{root_path, output_file};
  v8::Local<v8::Object> result = worker.Execute();
  info.GetReturnValue().Set(result);
}

NAN_MODULE_INIT(init)
{
  Nan::SetMethod(target, "tree", tree);
  Nan::SetMethod(target, "treeSync", treeSync);
}

NODE_MODULE(addon, init)
