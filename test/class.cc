#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <sstream>

#include <stdlib.h>

namespace fs = std::filesystem;

static const char* KEY_PATH = R"("path": ")";
static const char* KEY_PARENT_PATH = R"("parent_path": ")";
static const char* KEY_FILENAME = R"("filename": ")";
static const char* KEY_EXTENSION = R"("extension": ")";
static const char* KEY_IS_DIRECTORY = R"("is_directory": )";
static const char* KEY_CHILDREN = R"("children": ")";

class Entry
{
private:
	const std::string m_path;
	const std::string m_parent_path;
	const std::string m_filename;
	const std::string m_extension;
	const bool m_is_directory;
	std::vector<Entry>& m_children;

public:
	Entry(const fs::directory_entry& entry, std::vector<Entry> children)
		:m_path(entry.path().string<char>()),
		m_parent_path(entry.path().parent_path().string<char>()),
		m_filename(entry.path().filename().string<char>()),
		m_extension(entry.path().extension().string<char>()),
		m_is_directory(entry.is_directory()),
		m_children(children)
	{
	}

	~Entry()
	{
	}
public:
	void push_back(Entry child) {
		m_children.push_back(child);
	}

	Entry find_parent(int distance) {
		if (distance == 0) {
			return *this;
		}
		else {
			return m_children.back().find_parent(distance - 1);
		}
	}

	std::string to_string() {
		std::stringstream json;
		json << "{\n";
		json << KEY_PATH << m_path << "\",\n";
		json << KEY_PARENT_PATH << m_parent_path << "\",\n";
		json << KEY_FILENAME << m_filename << "\",\n";
		json << KEY_EXTENSION << m_extension << "\",\n";
		json << KEY_IS_DIRECTORY << m_is_directory;
		if (!m_children.empty()) {
			json << ",\n";
			bool first = true;
			json << KEY_CHILDREN << "[\n";
			for (Entry& entry : m_children) {
				if (!first)
					json << ",\n";
				json << entry.to_string();
				first = false;
			}
			json << "]\n";
		}
		json << "}\n";
		return json.str();
	}
};

int main() {
	fs::path root_path = R"(C:\Users\todo\Downloads)";
	std::cout << "root_path: " << root_path << std::endl;

	std::error_code ec;
	const fs::directory_entry& document_entry = fs::directory_entry{ root_path, ec };
	std::vector<Entry> children;
	Entry document = Entry{ document_entry , children };
	std::cout << "document: " << document.to_string() << std::endl;

	root_path /= "";
	const std::string root = root_path.string<char>().c_str();
	const std::size_t n = root.size();

	ec.clear();
	const auto iterator = fs::recursive_directory_iterator{ root, fs::directory_options::skip_permission_denied, ec };

	for (const fs::directory_entry& child_entry : iterator)
	{
		const fs::path path = child_entry.path().string<char>().replace(0, n, "");
		const fs::path parent_path = path.parent_path();

		auto distance = std::distance(parent_path.begin(), parent_path.end());
		Entry parent = document.find_parent(distance);

		std::vector<Entry> children;
		Entry child = Entry{ child_entry, children };
		parent.push_back(child);
	}

	std::cout << "result: " << document.to_string() << std::endl;
	system("pause");
}
