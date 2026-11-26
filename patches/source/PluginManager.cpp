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

    // Restore plugin enabled states from settings
    nlohmann::json plugins_settings = MasterRGBPluginBridge::GetSettings("Plugins");
    nlohmann::json pm_settings      = MasterRGBPluginBridge::GetSettings("PluginManager");

    for (std::size_t plugin_idx = 0; plugin_idx < ActivePlugins.size(); plugin_idx++)
    {
        bool is_enabled = false;

        // Check OpenRGB standard "Plugins" section by plugin name
        if (plugins_settings.contains("plugins") && plugins_settings["plugins"].is_array())
        {
            for (const auto& item : plugins_settings["plugins"])
            {
                if (item.contains("name") && item["name"] == ActivePlugins[plugin_idx].info.Name)
                {
                    if (item.contains("enabled"))
                    {
                        is_enabled = item["enabled"].get<bool>();
                    }
                    break;
                }
            }
        }

        // Also check "PluginManager" section by path or filename
        if (!is_enabled && pm_settings.contains("plugins"))
        {
            for (auto it = pm_settings["plugins"].begin(); it != pm_settings["plugins"].end(); ++it)
            {
                filesystem::path p1 = it.key();
                filesystem::path p2 = ActivePlugins[plugin_idx].path;
                if (p1 == p2 || p1.string() == p2.string() || p1.generic_u8string() == p2.generic_u8string() || p1.filename() == p2.filename())
                {
                    if (it.value().contains("enabled"))
                    {
                        is_enabled = it.value()["enabled"].get<bool>();
                    }
                    break;
                }
            }
        }

        ActivePlugins[plugin_idx].enabled = is_enabled;
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
    filesystem::path p_target = path;
    for(unsigned int i = 0; i < ActivePlugins.size(); i++)
    {
        filesystem::path existing_p = ActivePlugins[i].path;
        if(existing_p == p_target || existing_p.filename() == p_target.filename())
        {
            return;
        }
    }

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

            for(unsigned int i = 0; i < ActivePlugins.size(); i++)
            {
                if(!plugin_entry.info.Name.empty() && ActivePlugins[i].info.Name == plugin_entry.info.Name)
                {
                    delete plugin_entry.loader;
                    return;
                }
            }
        }
    }

    ActivePlugins.push_back(plugin_entry);
}

void PluginManager::RemovePlugin(const filesystem::path& path)
{
    filesystem::path target = path;
    unsigned int plugin_idx;

    for(plugin_idx = 0; plugin_idx < ActivePlugins.size(); plugin_idx++)
    {
        filesystem::path entry_p = ActivePlugins[plugin_idx].path;
        if(target == entry_p || target.string() == entry_p.string() || target.generic_u8string() == entry_p.generic_u8string() || target.filename() == entry_p.filename())
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
    filesystem::path target = path;
    unsigned int plugin_idx;

    for(plugin_idx = 0; plugin_idx < ActivePlugins.size(); plugin_idx++)
    {
        filesystem::path entry_p = ActivePlugins[plugin_idx].path;
        if(target == entry_p || target.string() == entry_p.string() || target.generic_u8string() == entry_p.generic_u8string() || target.filename() == entry_p.filename())
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

    // Save to PluginManager settings
    nlohmann::json pm_settings = MasterRGBPluginBridge::GetSettings("PluginManager");
    pm_settings["plugins"][ActivePlugins[plugin_idx].path]["enabled"] = true;
    MasterRGBPluginBridge::SetSettings("PluginManager", pm_settings);

    // Also sync to Plugins settings
    nlohmann::json plugins_settings = MasterRGBPluginBridge::GetSettings("Plugins");
    bool found = false;
    if (plugins_settings.contains("plugins") && plugins_settings["plugins"].is_array())
    {
        for (auto& item : plugins_settings["plugins"])
        {
            if (item.contains("name") && item["name"] == ActivePlugins[plugin_idx].info.Name)
            {
                item["enabled"] = true;
                found = true;
                break;
            }
        }
    }
    if (!found)
    {
        nlohmann::json item;
        item["name"] = ActivePlugins[plugin_idx].info.Name;
        item["description"] = ActivePlugins[plugin_idx].info.Description;
        item["enabled"] = true;
        plugins_settings["plugins"].push_back(item);
    }
    MasterRGBPluginBridge::SetSettings("Plugins", plugins_settings);
    MasterRGBPluginBridge::SaveSettings();
}

void PluginManager::DisablePlugin(const filesystem::path& path)
{
    filesystem::path target = path;
    unsigned int plugin_idx;

    for(plugin_idx = 0; plugin_idx < ActivePlugins.size(); plugin_idx++)
    {
        filesystem::path entry_p = ActivePlugins[plugin_idx].path;
        if(target == entry_p || target.string() == entry_p.string() || target.generic_u8string() == entry_p.generic_u8string() || target.filename() == entry_p.filename())
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

    // Save to PluginManager settings
    nlohmann::json pm_settings = MasterRGBPluginBridge::GetSettings("PluginManager");
    pm_settings["plugins"][ActivePlugins[plugin_idx].path]["enabled"] = false;
    MasterRGBPluginBridge::SetSettings("PluginManager", pm_settings);

    // Also sync to Plugins settings
    nlohmann::json plugins_settings = MasterRGBPluginBridge::GetSettings("Plugins");
    if (plugins_settings.contains("plugins") && plugins_settings["plugins"].is_array())
    {
        for (auto& item : plugins_settings["plugins"])
        {
            if (item.contains("name") && item["name"] == ActivePlugins[plugin_idx].info.Name)
            {
                item["enabled"] = false;
                break;
            }
        }
    }
    MasterRGBPluginBridge::SetSettings("Plugins", plugins_settings);
    MasterRGBPluginBridge::SaveSettings();
}

void PluginManager::LoadPlugin(OpenRGBPluginEntry* plugin_entry)
{
    if(plugin_entry->incompatible)
    {
        return;
    }

    if(plugin_entry->plugin != nullptr && plugin_entry->widget != nullptr)
    {
        return;
    }

    if(!plugin_entry->loader->isLoaded())
    {
        plugin_entry->loader->load();
    }

    QObject* instance = plugin_entry->loader->instance();

    if(instance)
    {
        OpenRGBPluginInterface* plugin = qobject_cast<OpenRGBPluginInterface*>(instance);

        if(plugin)
        {
            if(plugin->GetPluginAPIVersion() <= OPENRGB_PLUGIN_API_VERSION)
            {
                plugin_entry->plugin = plugin;
                if(!plugin_entry->api)
                {
                    plugin_entry->api = new OpenRGBPluginAPI();
                }

                try
                {
                    plugin->Load(plugin_entry->api);
                    plugin_entry->widget = plugin->GetWidget();
                    plugin_entry->traymenu = plugin->GetTrayMenu();

                    if(AddPluginCallbackArg != nullptr && AddPluginCallbackVal != nullptr)
                    {
                        AddPluginCallbackVal(AddPluginCallbackArg, plugin_entry);
                    }

                    MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 3, "[PluginManager] Successfully loaded and initialized plugin: %s", plugin_entry->info.Name.c_str());
                }
                catch(const std::exception& e)
                {
                    MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 1, "[PluginManager] Exception loading plugin %s: %s", plugin_entry->info.Name.c_str(), e.what());
                }
                catch(...)
                {
                    MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 1, "[PluginManager] Unknown exception loading plugin %s", plugin_entry->info.Name.c_str());
                }
            }
            else
            {
                MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 2, "[PluginManager] Plugin %s API version %d > %d", plugin_entry->info.Name.c_str(), plugin->GetPluginAPIVersion(), OPENRGB_PLUGIN_API_VERSION);
            }
        }
        else
        {
            MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 2, "[PluginManager] Failed to cast instance to OpenRGBPluginInterface for: %s", plugin_entry->path.c_str());
        }
    }
    else
    {
        MasterRGBPluginBridge::Log("PluginManager.cpp", __LINE__, 2, "[PluginManager] Failed to load plugin instance for: %s (error: %s)", plugin_entry->path.c_str(), plugin_entry->loader->errorString().toStdString().c_str());
    }
}

void PluginManager::UnloadPlugin(OpenRGBPluginEntry* plugin_entry)
{
    if(plugin_entry->plugin)
    {
        try
        {
            plugin_entry->plugin->Unload();
        }
        catch(...) {}
    }

    if(RemovePluginCallbackVal != nullptr && RemovePluginCallbackArg != nullptr)
    {
        RemovePluginCallbackVal(RemovePluginCallbackArg, plugin_entry);
    }

    if(plugin_entry->api != nullptr)
    {
        delete plugin_entry->api;
        plugin_entry->api = nullptr;
    }

    plugin_entry->widget = nullptr;
    plugin_entry->traymenu = nullptr;
    plugin_entry->plugin = nullptr;

    if(plugin_entry->loader->isLoaded())
    {
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
    if (plugin_idx < ActivePlugins.size())
    {
        return ActivePlugins[plugin_idx].info.Description;
    }
    return "";
}

std::string PluginManager::GetPluginName(unsigned int plugin_idx)
{
    if (plugin_idx < ActivePlugins.size())
    {
        return ActivePlugins[plugin_idx].info.Name;
    }
    return "";
}

unsigned int PluginManager::GetPluginProtocolVersion(unsigned int plugin_idx)
{
    if (plugin_idx < ActivePlugins.size())
    {
        return ActivePlugins[plugin_idx].info.ProtocolVersion;
    }
    return 0;
}

std::string PluginManager::GetPluginVersion(unsigned int plugin_idx)
{
    if (plugin_idx < ActivePlugins.size())
    {
        return ActivePlugins[plugin_idx].info.Version;
    }
    return "";
}
