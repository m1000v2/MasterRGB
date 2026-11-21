/*---------------------------------------------------------*\
| PluginManager.cpp                                         |
|                                                           |
|   OpenRGB plugin manager                                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "PluginManager.h"
#include "OpenRGBPluginInterface.h"
#include "OpenRGBPluginAPI.h"
#include "MasterRGBPluginBridge.h"
#include "filesystem.h"

#ifdef _WIN32
#include <Windows.h>
#endif

static void PluginManagerDeviceListChangeCallback(void* this_ptr)
{
    PluginManager* this_obj = (PluginManager*)this_ptr;
    this_obj->ResourceManagerCallback(0);
}

PluginManager::PluginManager()
{
    AddPluginCallbackVal    = nullptr;
    AddPluginCallbackArg    = nullptr;
    RemovePluginCallbackVal = nullptr;
    RemovePluginCallbackArg = nullptr;

    filesystem::path plugins_dir = MasterRGBPluginBridge::GetConfigurationDirectory() / plugins_path;
    filesystem::create_directories(plugins_dir);

    MasterRGBPluginBridge::RegisterDeviceListChangeCallback(PluginManagerDeviceListChangeCallback, this);
}

PluginManager::~PluginManager()
{
    MasterRGBPluginBridge::UnregisterDeviceListChangeCallback(PluginManagerDeviceListChangeCallback, this);
    UnloadPlugins();
}

void PluginManager::RegisterAddPluginCallback(AddPluginCallback new_callback, void * new_callback_arg)
{
    AddPluginCallbackVal    = new_callback;
    AddPluginCallbackArg    = new_callback_arg;
}

void PluginManager::RegisterRemovePluginCallback(RemovePluginCallback new_callback, void * new_callback_arg)
{
    RemovePluginCallbackVal = new_callback;
    RemovePluginCallbackArg = new_callback_arg;
}

void PluginManager::ScanAndLoadPlugins()
{
    filesystem::path plugins_dir = MasterRGBPluginBridge::GetConfigurationDirectory() / plugins_path;
    ScanAndLoadPluginsFrom(plugins_dir, false);

#ifdef OPENRGB_SYSTEM_PLUGIN_DIRECTORY
    ScanAndLoadPluginsFrom(OPENRGB_SYSTEM_PLUGIN_DIRECTORY, true);
#endif

#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    filesystem::path exe_dir(path);
    exe_dir = exe_dir.remove_filename() / plugins_path;

    ScanAndLoadPluginsFrom(exe_dir, true);
#endif

    nlohmann::json plugin_settings = MasterRGBPluginBridge::GetSettings("PluginManager");
    if (plugin_settings.contains("plugins"))
    {
        for (std::size_t plugin_idx = 0; plugin_idx < ActivePlugins.size(); plugin_idx++)
        {
            if (plugin_settings["plugins"].contains(ActivePlugins[plugin_idx].path))
            {
                if (plugin_settings["plugins"][ActivePlugins[plugin_idx].path].contains("enabled"))
                {
                    ActivePlugins[plugin_idx].enabled = plugin_settings["plugins"][ActivePlugins[plugin_idx].path]["enabled"];
                }
            }
        }
    }

    LoadPlugins();
}

void PluginManager::ScanAndLoadPluginsFrom(const filesystem::path & plugins_dir, bool is_system)
{
    if(is_system)
    {
        MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 3, "[PluginManager] Scanning system plugin directory: %s", plugins_dir.generic_u8string().c_str());
    }
    else
    {
        MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 3, "[PluginManager] Scanning user plugin directory: %s", plugins_dir.generic_u8string().c_str());
    }

    if(!filesystem::is_directory(plugins_dir))
    {
        return;
    }

    for(const filesystem::directory_entry& entry: filesystem::directory_iterator(plugins_dir))
    {
        if(filesystem::is_directory(entry.path()))
        {
            continue;
        }

        filesystem::path p = entry.path();
        if(p.extension() == ".dll" || p.extension() == ".so" || p.extension() == ".dylib")
        {
            AddPlugin(p, is_system);
        }
    }
}

void PluginManager::AddPlugin(const filesystem::path& path, bool is_system)
{
    unsigned int plugin_idx;

    for(plugin_idx = 0; plugin_idx < ActivePlugins.size(); plugin_idx++)
    {
        if(path == ActivePlugins[plugin_idx].path)
        {
            break;
        }
    }

    if(plugin_idx == ActivePlugins.size())
    {
        OpenRGBPluginEntry plugin_entry;

        plugin_entry.path = path.string();
        plugin_entry.loader = new QPluginLoader(QString::fromStdString(plugin_entry.path));
        plugin_entry.plugin = nullptr;
        plugin_entry.api = nullptr;
        plugin_entry.widget = nullptr;
        plugin_entry.traymenu = nullptr;
        plugin_entry.enabled = false;
        plugin_entry.incompatible = true;
        plugin_entry.is_system = is_system;
        plugin_entry.api_version = 0;

        QObject* instance = plugin_entry.loader->instance();

        if(instance)
        {
            OpenRGBPluginInterface* plugin = qobject_cast<OpenRGBPluginInterface*>(instance);

            if(plugin)
            {
                plugin_entry.info = plugin->GetPluginInfo();
                plugin_entry.api_version = plugin->GetPluginAPIVersion();

                if(plugin_entry.api_version <= OPENRGB_PLUGIN_API_VERSION)
                {
                    plugin_entry.incompatible = false;
                }
            }
        }

        ActivePlugins.push_back(plugin_entry);
    }
}

void PluginManager::RemovePlugin(const filesystem::path& path)
{
    unsigned int plugin_idx;

    for(plugin_idx = 0; plugin_idx < ActivePlugins.size(); plugin_idx++)
    {
        if(path == ActivePlugins[plugin_idx].path)
        {
            break;
        }
    }

    if(plugin_idx == ActivePlugins.size())
    {
        return;
    }

    UnloadPlugin(&ActivePlugins[plugin_idx]);
    delete ActivePlugins[plugin_idx].loader;
    ActivePlugins.erase(ActivePlugins.begin() + plugin_idx);
}

void PluginManager::EnablePlugin(const filesystem::path& path)
{
    unsigned int plugin_idx;

    for(plugin_idx = 0; plugin_idx < ActivePlugins.size(); plugin_idx++)
    {
        if(path == ActivePlugins[plugin_idx].path)
        {
            break;
        }
    }

    if(plugin_idx == ActivePlugins.size())
    {
        return;
    }

    ActivePlugins[plugin_idx].enabled = true;
    LoadPlugin(&ActivePlugins[plugin_idx]);

    nlohmann::json plugin_settings = MasterRGBPluginBridge::GetSettings("PluginManager");
    plugin_settings["plugins"][ActivePlugins[plugin_idx].path]["enabled"] = true;
    MasterRGBPluginBridge::SetSettings("PluginManager", plugin_settings);
    MasterRGBPluginBridge::SaveSettings();
}

void PluginManager::LoadPlugin(OpenRGBPluginEntry* plugin_entry)
{
    if(plugin_entry->incompatible)
    {
        return;
    }

    if(!plugin_entry->loader->isLoaded())
    {
        plugin_entry->loader->load();

        QObject* instance = plugin_entry->loader->instance();

        if(instance)
        {
            OpenRGBPluginInterface* plugin = qobject_cast<OpenRGBPluginInterface*>(instance);

            if(plugin)
            {
                if(plugin->GetPluginAPIVersion() <= OPENRGB_PLUGIN_API_VERSION)
                {
                    plugin_entry->plugin = plugin;
                    plugin_entry->api = new OpenRGBPluginAPI();

                    plugin->Load(plugin_entry->api);
                    plugin_entry->widget = plugin->GetWidget();
                    plugin_entry->traymenu = plugin->GetTrayMenu();

                    if(AddPluginCallbackArg != nullptr)
                    {
                        AddPluginCallbackVal(AddPluginCallbackArg, plugin_entry);
                    }
                }
            }
        }
    }
}

void PluginManager::DisablePlugin(const filesystem::path& path)
{
    unsigned int plugin_idx;

    for(plugin_idx = 0; plugin_idx < ActivePlugins.size(); plugin_idx++)
    {
        if(path == ActivePlugins[plugin_idx].path)
        {
            break;
        }
    }

    if(plugin_idx == ActivePlugins.size())
    {
        return;
    }

    ActivePlugins[plugin_idx].enabled = false;
    UnloadPlugin(&ActivePlugins[plugin_idx]);

    nlohmann::json plugin_settings = MasterRGBPluginBridge::GetSettings("PluginManager");
    plugin_settings["plugins"][ActivePlugins[plugin_idx].path]["enabled"] = false;
    MasterRGBPluginBridge::SetSettings("PluginManager", plugin_settings);
    MasterRGBPluginBridge::SaveSettings();
}

void PluginManager::UnloadPlugin(OpenRGBPluginEntry* plugin_entry)
{
    if(plugin_entry->loader->isLoaded())
    {
        if (plugin_entry->plugin)
        {
            plugin_entry->plugin->Unload();
        }

        if(RemovePluginCallbackVal != nullptr)
        {
            RemovePluginCallbackVal(RemovePluginCallbackArg, plugin_entry);
        }

        if (plugin_entry->api != nullptr)
        {
            delete plugin_entry->api;
            plugin_entry->api = nullptr;
        }

        bool unloaded = plugin_entry->loader->unload();

        if(!unloaded)
        {
            MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 2, "[PluginManager] Plugin %s cannot be unloaded", plugin_entry->path.c_str());
        }
        else
        {
            MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 3, "[PluginManager] Plugin %s successfully unloaded", plugin_entry->path.c_str());
        }
    }
}

void PluginManager::LoadPlugins()
{
    for(OpenRGBPluginEntry& plugin_entry: ActivePlugins)
    {
        if(plugin_entry.enabled)
        {
            LoadPlugin(&plugin_entry);
        }
    }
}

void PluginManager::UnloadPlugins()
{
    for(OpenRGBPluginEntry& plugin_entry: ActivePlugins)
    {
        UnloadPlugin(&plugin_entry);
    }
}

void PluginManager::OnProfileAboutToLoad()
{
    for (auto& entry : ActivePlugins)
    {
        if (entry.enabled && entry.loader->isLoaded() && entry.plugin)
        {
            entry.plugin->OnProfileAboutToLoad();
        }
    }
}

void PluginManager::OnProfileLoad(nlohmann::json profile_data)
{
    for (auto& entry : ActivePlugins)
    {
        if (entry.enabled && entry.loader->isLoaded() && entry.plugin)
        {
            if (profile_data.contains(entry.info.Name))
            {
                entry.plugin->OnProfileLoad(profile_data[entry.info.Name]);
            }
        }
    }
}

nlohmann::json PluginManager::OnProfileSave()
{
    nlohmann::json j;
    for (auto& entry : ActivePlugins)
    {
        if (entry.enabled && entry.loader->isLoaded() && entry.plugin)
        {
            j[entry.info.Name] = entry.plugin->OnProfileSave();
        }
    }
    return j;
}

void PluginManager::ProfileManagerCallback(unsigned int update_reason)
{
    for (auto& entry : ActivePlugins)
    {
        if (entry.enabled && entry.loader->isLoaded() && entry.plugin)
        {
            entry.plugin->ProfileManagerUpdated(update_reason);
        }
    }
}

void PluginManager::ResourceManagerCallback(unsigned int update_reason)
{
    for (auto& entry : ActivePlugins)
    {
        if (entry.enabled && entry.loader->isLoaded() && entry.plugin)
        {
            entry.plugin->ResourceManagerUpdated(update_reason);
        }
    }
}

void PluginManager::SettingsManagerCallback(unsigned int update_reason)
{
    for (auto& entry : ActivePlugins)
    {
        if (entry.enabled && entry.loader->isLoaded() && entry.plugin)
        {
            entry.plugin->SettingsManagerUpdated(update_reason);
        }
    }
}

unsigned int PluginManager::GetPluginCount()
{
    return (unsigned int)ActivePlugins.size();
}

std::string PluginManager::GetPluginDescription(unsigned int plugin_idx)
{
    if (plugin_idx < ActivePlugins.size() && ActivePlugins[plugin_idx].plugin)
    {
        return ActivePlugins[plugin_idx].info.Description;
    }
    return "";
}

std::string PluginManager::GetPluginName(unsigned int plugin_idx)
{
    if (plugin_idx < ActivePlugins.size() && ActivePlugins[plugin_idx].plugin)
    {
        return ActivePlugins[plugin_idx].info.Name;
    }
    return "";
}

unsigned int PluginManager::GetPluginProtocolVersion(unsigned int plugin_idx)
{
    if (plugin_idx < ActivePlugins.size() && ActivePlugins[plugin_idx].plugin)
    {
        return ActivePlugins[plugin_idx].info.ProtocolVersion;
    }
    return 0;
}

std::string PluginManager::GetPluginVersion(unsigned int plugin_idx)
{
    if (plugin_idx < ActivePlugins.size() && ActivePlugins[plugin_idx].plugin)
    {
        return ActivePlugins[plugin_idx].info.Version;
    }
    return "";
}
