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
			// insert server with port. Insert does not overwrite, so the first server with a port will be the one stored.
            this->_portsToServers.insert({it_ports.value, &this->_servers[i]});
            if (std::find(_ports.begin(), _ports.end(), it_ports.value) == _ports.end())
                _ports.push_back(it_ports.value);
            for (auto it_names : this->_servers[i].names.name_vec) {
				// insert server with port and name. Insert does not overwrite, so the first server with a port and name will be the one stored.
                this->_portsNamesToServers.insert({{it_ports.value, it_names}, &this->_servers[i]});
            }
        }
    }
}

/**
 * @brief Return the server that is listening on the given port.
 * 
 * @param port 
 * @return const ServerConfig* 
 */
const ServerConfig *MainConfig::getServerFromPort(uint16_t port) { 
    auto pos = this->_portsToServers.find(port);
    if (pos != this->_portsToServers.end())
        return pos->second;
    return nullptr;
}

/**
 * @brief Return the server that is listening on the given port and name.
 * 
 * @param port 
 * @param name 
 * @return const ServerConfig* 
 */
const ServerConfig *MainConfig::getServerFromPortAndName(uint16_t port, std::string name) {
    auto pos = this->_portsNamesToServers.find({port, name});
    if (pos != this->_portsNamesToServers.end())
        return pos->second;
    return nullptr;
}

/**
 * @brief Return the server that is listening on the given port and name.
 * 
 * @param port 
 * @param name 
 * @return const ServerConfig* 
 */
const ServerConfig *MainConfig::getServer(uint16_t port, std::string name) {
    const ServerConfig *ret;
	// if a name was given, try to find a server with the given name.
    if (!name.empty() && (ret = getServerFromPortAndName(port, name)))
        return ret;
	// if no server was found with the given name, return the first server that listens on the given port.
    if ((ret = getServerFromPort(port)))
        return ret;
    return nullptr;
}

const std::vector<uint16_t> &MainConfig::getPorts(void) {
    return (_ports);
}

std::string ServerConfig::getErrorPage(int errorCode) const {
    for (auto it : this->errorPages) {
        for (size_t i = 0; i < it.errorNumbers.size(); ++i) {
			if (it.errorNumbers[i] == errorCode)
				return it.page;
        }
    }
    return std::string();
}

std::string ServerConfig::getCgiExectorFromPath(std::string path) const {
	size_t extensionPos = path.find_last_of('.');
	if (this->cgis.empty() || extensionPos == std::string::npos)
		return std::string();
	std::string extension = path.substr(extensionPos);
	for (auto it : this->cgis) {
		for (size_t i = 0; i < it.extensions.size(); ++i) {
			if (path.find(it.extensions[i]) != std::string::npos)
				return it.executor;
		}
	}
    return std::string();
}

void ServerConfig::sortLocations(void) {
	std::sort(this->locations.begin(), this->locations.end(), [](const Location &a, const Location &b) {
		return a.ref.size() > b.ref.size();
	});
}
