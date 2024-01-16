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

	if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
	{
		std::cerr << "Index of " << path << "\n";
		for (const auto& it : std::filesystem::directory_iterator(path))
			entries.push_back(it.path());
			// std::cerr << it.path().filename() << "\n";
		std::sort(entries.begin(), entries.end(), compareDirectoryContents);
		for (const auto& it : entries)
		{
			std::cerr << it.filename().string() ;
			if (std::filesystem::is_directory(it))
				std::cerr << "/";
			else
				std::cerr << "\t" << std::filesystem::file_size(it);
			std::cerr << "\n";
		}
	}
	else
		std::cerr << path << " is not a dir\n";
}