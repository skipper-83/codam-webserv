#include <ostream>
#include <iomanip>

#include "config.hpp"

std::ostream& operator<<(std::ostream& os, const AutoIndex& rhs) {
    os << "autoindex " << (rhs.on ? "on" : "off") << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, const BodySize& rhs) {
    os << "client_max_body_size " << std::fixed << std::setprecision(2);
    if (rhs.value >= 1000000000)
        os  << static_cast<float>(rhs.value) / 1000000000 << "g";
    else if (rhs.value >= 1000000)
        os << static_cast<float>(rhs.value) / 1000000 << "m";
    else if (rhs.value >= 1000)
        os << static_cast<float>(rhs.value)  / 1000 << "k";
	else
		os << static_cast<float>(rhs.value) << "b";
    os << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Location& rhs) {
    os << "\tlocation" << rhs.ref << " {\n\t\troot " << rhs.root << ";\n";
    if (rhs.index_vec.size()) {
        os << "\t\tindex";
        for (auto it : rhs.index_vec)
            os << " " << it;
    }
    os << ";\n\t}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ErrorPage& rhs) {
    os << "\terror_page";
    for (auto it : rhs.errorNumbers)
        os << " " << it;
    os << " " << rhs.page << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, const AllowedMethods& rhs) {
    os << "allowed_methods";
    for (auto it : rhs.methods) {
        if (it.second)
            os << " " << it.first;
    }
    os << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ServerConfig& rhs) {
    if (rhs.names.name_vec.size() > 0) {
        os << "\tserver_name";
        for (auto it : rhs.names.name_vec)
            os << " " << it;
        os << ";\n";
    }
    for (auto it : rhs.ports)
        os << "\tlisten " << it.value << ";\n";
    os << "\t" << rhs.allowed << "\n";
    if (rhs.locations.size() > 0) {
        for (auto it : rhs.locations)
            os << it << "\n";
    }
    if (rhs.errorPages.size() > 0) {
        for (auto it : rhs.errorPages)
            os << it << "\n";
    }
    os << "\t" << rhs.autoIndex << "\n"
       << "\t" << rhs.clientMaxBodySize << "\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const MainConfig& rhs) {
    os << rhs._autoIndex << "\n" << rhs.clientMaxBodySize << "\n" << rhs._allowed << "\n";
    for (auto it : rhs._servers)
        os << "\nserver { #" << "rank: " << it.rank << "\n" << it << "}\n";
    os << std::endl;
    return os;
}
