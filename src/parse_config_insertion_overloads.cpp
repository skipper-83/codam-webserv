#include <ostream>

#include "parse_config.hpp"

std::ostream& operator<<(std::ostream& os, AutoIndex& rhs) {
    os << "autoindex " << (rhs.on ? "on" : "off") << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, BodySize& rhs) {
    os << "client_max_body_size ";
    if (rhs.value >= 1000000)
        os << std::to_string(static_cast<float>(rhs.value) / 1000000) << "g";
    else if (rhs.value >= 1000)
        os << std::to_string(static_cast<float>(rhs.value) / 1000) << "m";
    else
        os << std::to_string(static_cast<float>(rhs.value)) << "k";
    os << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, Location& rhs) {
    os << "\tlocation" << rhs.ref << " {\n\t\troot " << rhs.root << ";\n";
    if (rhs.index_vec.size()) {
        os << "\t\tindex";
        for (auto it : rhs.index_vec)
            os << " " << it;
    }
    os << ";\n\t}";
    return os;
}

std::ostream& operator<<(std::ostream& os, ErrorPage& rhs) {
    os << "\terror_page";
    for (auto it : rhs.errorNumbers)
        os << " " << it;
    os << " " << rhs.page << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, AllowedMethods& rhs) {
    os << "allowed_methods";
    for (auto it : rhs.methods) {
        if (it.second)
            os << " " << it.first;
    }
    os << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, ServerConfig& rhs) {
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

std::ostream& operator<<(std::ostream& os, MainConfig& rhs) {
    os << rhs.autoIndex << "\n" << rhs.clientMaxBodySize << "\n" << rhs.allowed << "\n";
    for (auto it : rhs.servers)
        os << "\nserver {\n" << it << "}\n";
    os << std::endl;
    return os;
}
