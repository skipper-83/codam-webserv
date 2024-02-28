#include <filesystem>
#include <iostream>
#include <vector>

bool compareDirectoryContents(const std::filesystem::path &one, const std::filesystem::path &two)
{
	bool oneIsDir = std::filesystem::is_directory(one);
	bool twoIsDir = std::filesystem::is_directory(two);

	if (oneIsDir == twoIsDir)
		return one.filename().string() < two.filename().string();
	return oneIsDir > twoIsDir;
}

void checkFile(std::string path)
{
	std::vector<std::filesystem::path> entries;
	std::string ret_string, title, up;

	if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
	{
		title = "Index of " + path;
		up = path.substr(0, path.substr(0, path.length() - 1).find_last_of('/') + 1);
		ret_string.append("<html><head><title>").append(title).append("</title></head><body>\n");
		ret_string.append("<h1>" + title + "</h1><hr><pre>\n");
		ret_string.append("up: " + up + "\n");
		for (const auto& it : std::filesystem::directory_iterator(path))
			entries.push_back(it.path());
		std::sort(entries.begin(), entries.end(), compareDirectoryContents);
		for (const auto& it : entries)
		{
			ret_string.append(it.filename().string());
			if (std::filesystem::is_directory(it))
				ret_string.append("/");
			else
				ret_string.append("\t" + std::to_string(std::filesystem::file_size(it)));
			ret_string.append("\n");
		}
		ret_string.append("</pre><hr></body></html>");
		std::cerr << ret_string;
	}
	else
		std::cerr << path << " is not a dir\n";
}