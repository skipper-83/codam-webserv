#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <istream>
#include <map>
#include <functional>

// void     CPPLog(std::Str)

template <char C>
std::istream &expect(std::istream &s) {
    if (s.flags() & std::ios_base::skipws) s >> std::ws;
    if (s.peek() == C)
        s.ignore();
    else
        s.setstate(std::ios_base::failbit);
    return (s);
}

struct ServerConfig
{

};

struct MainConfig
{
    std::vector<ServerConfig> servers;
    bool autoIndex;
    std::vector<std::string> serverNames;
};

std::istream& operator>>(std::istream& is, ServerConfig &lhs)
{
    std::cout <<"server!";
}

std::istream& operator>>(std::istream& is, MainConfig &lhs)
{
    (void)lhs;
    (void)is;
    std::cout << "called!!\n";
    std::string word;
    std::cout << word;
    std::map<std::string, std::function<void (std::istream &)> > sub_parsers = 
    {
        {"server", [&lhs](std::istream& is){ ServerConfig newServer; is >> newServer; lhs.servers.push_back(newServer); }
        {"server", [&lhs](std::istream& is){ ServerConfig newServer; is >> newServer; lhs.servers.push_back(newServer); }.
    }

    while (is >> word)
    {

    }
    return is;
}

int main(int argc, char** argv)
{
    std::fstream file;
    std::string line;
    std::string::size_type pos;
    std::stringstream ss;

    if (argc != 2)
        return 1;
    file.open(argv[1]);
    if (!file)
        return 1;
    while (getline(file, line))
    {
        if ((pos = line.find('#')) != std::string::npos)
            line.erase(pos);
        if (std::all_of(line.begin(), line.end(), [](char c){ return std::isspace(static_cast<unsigned char>(c));} ))
            continue;
        ss << line << "\n";
    }
    // std::cout << ss.str();
    MainConfig test;
    ss >> test;
    // std::cout << "BOOH";
}