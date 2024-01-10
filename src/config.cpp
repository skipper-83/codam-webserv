#include "config.hpp"
#include <iostream>

/**
 * @brief Called after all servers are passed, overrides defaults when a global value is set,
 * 		and the server has no specific 'own' setting.
 *
 * @param rhs
 */
void MainConfig::_overrideDefaults() {
    for (auto &it : this->_servers) {
        if (!this->_autoIndex.defaultValue && it.autoIndex.defaultValue) {
            it.autoIndex.on = this->_autoIndex.on;
            it.autoIndex.defaultValue = false;
        }
        if (!this->clientMaxBodySize.defaultValue && it.clientMaxBodySize.defaultValue) {
            it.clientMaxBodySize.value = this->clientMaxBodySize.value;
            it.clientMaxBodySize.defaultValue = false;
        }
        if (!this->_allowed.defaultValue && it.allowed.defaultValue) {
            it.allowed.methods = this->_allowed.methods;
            it.allowed.defaultValue = false;
        }
    }
}

void MainConfig::_setServerNameAndPortArrays(void) {
    for (size_t i = 0; i < this->_servers.size(); ++i) {
        for (auto it_ports : this->_servers[i].ports) {
            this->_portsToServers.insert({it_ports.value, &this->_servers[i]});
            if (std::find(_ports.begin(), _ports.end(), it_ports.value) == _ports.end())
                _ports.push_back(it_ports.value);
            for (auto it_names : this->_servers[i].names.name_vec) {
                this->_portsNamesToServers.insert({{it_ports.value, it_names}, &this->_servers[i]});
            }
        }
    }
}

const ServerConfig *MainConfig::getServerFromPort(int port) {
    auto pos = this->_portsToServers.find(port);
    if (pos != this->_portsToServers.end())
        return pos->second;
    return nullptr;
}

const ServerConfig *MainConfig::getServerFromPortAndName(int port, std::string name) {
    auto pos = this->_portsNamesToServers.find({port, name});
    if (pos != this->_portsNamesToServers.end())
        return pos->second;
    return nullptr;
}

const ServerConfig *MainConfig::getServer(int port, std::string name) {
    const ServerConfig *ret;
    if (!name.empty() && (ret = getServerFromPortAndName(port, name)))
        return ret;
    if ((ret = getServerFromPort(port)))
        return ret;
    return nullptr;
}

const std::vector<uint16_t>& MainConfig::getPorts(void) {
    return (_ports);
}
