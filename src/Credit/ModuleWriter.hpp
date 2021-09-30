#pragma once

#include <algorithm>
#include <sstream>
#include "../plugin.hpp"

struct PluginInfo {
    std::string name, brand, url;
    std::vector<std::string> modules;

    PluginInfo(const Plugin* plugin) {
        name = plugin->name;
        brand = plugin->brand;
        url = plugin->pluginUrl;
    }

    /**Adds a module to list if not already present */
    void addModule(const std::string& moduleName) {
        if(std::find(modules.begin(), modules.end(), moduleName) != modules.end()) // already added
            return;
        
        modules.push_back(moduleName);
    }

    /** Prints plugin info to a string */
    std::string print(bool printURL) const {
        std::stringstream ss; // { brand + ": " + name + " (" + url + ")" + '\n' };
        ss << brand << ": " << name;
        
        if(! url.empty() && printURL)
            ss << " (" << url << ")";
        ss << '\n';

        for(const auto& mod : modules)
            ss << "    " << mod << '\n';

        return ss.str();
    }
};

struct ModuleWriter {
    std::vector<PluginInfo> plugins;
    bool writeURLs = true;

    /** Returns the plugin info object wth this name, or nullptr if plugin hasn't been loaded */
    PluginInfo* getPlugin(const std::string& name) {
        auto pluginIter = std::find_if(plugins.begin(), plugins.end(), [=] (const PluginInfo& p) { return p.name == name; });
        if(pluginIter != plugins.end())
            return &(*pluginIter);

        return nullptr; // hasn't been loaded yet
    }

    /** Loads plugin info from current Rack session */
    void loadPlugins() {
        auto* moduleContainer = APP->scene->rack->getModuleContainer();

        for (widget::Widget* w : moduleContainer->children) {
	    	ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
	    	assert(moduleWidget);

            auto plugin = getPlugin(moduleWidget->model->plugin->name);
            if(plugin == nullptr) { // plugin hasn't been loaded yet
                plugins.push_back(PluginInfo { moduleWidget->model->plugin });
                plugin = &plugins.back();
            }

            plugin->addModule(moduleWidget->model->name);
	    }
    }
    
    /** Writes session modules to a file */
    void operator()(FILE* file) {
        loadPlugins();

        for(const auto& p : plugins) {
            auto pStr = p.print(writeURLs);
            fprintf(file, pStr.c_str());
        }
    }
};
