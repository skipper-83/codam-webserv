#include "config.hpp"

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
            this->_ports.insert({it_ports.value, &this->_servers[i]});
            for (auto it_names : this->_servers[i].names.name_vec) {
                this->_portsNames.insert({{it_ports.value, it_names}, &this->_servers[i]});
            }
        }
    }
}

const ServerConfig *MainConfig::getServerFromPort(int port) {
    auto pos = this->_ports.find(port);
    if (pos != this->_ports.end())
        return pos->second;
    return nullptr;
}

const ServerConfig *MainConfig::getServerFromPortAndName(int port, std::string name) {
    auto pos = this->_portsNames.find({port, name});
    if (pos != this->_portsNames.end())
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
