#include "MasterRGBPluginBridge.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "ProfileManager.h"
#include "LogManager.h"
#include "RGBController.h"
#include <fstream>

namespace MasterRGBPluginBridge
{
    filesystem::path GetConfigurationDirectory()
    {
        return ResourceManager::get()->GetConfigurationDirectory();
    }

    bool GetDetectionEnabled()
    {
        return ResourceManager::get()->GetDetectionEnabled();
    }

    unsigned int GetDetectionPercent()
    {
        return ResourceManager::get()->GetDetectionPercent();
    }

    std::string GetDetectionString()
    {
        const char* str = ResourceManager::get()->GetDetectionString();
        return str ? std::string(str) : std::string();
    }

    void RescanDevices()
    {
        ResourceManager::get()->RescanDevices();
    }

    void WaitForDetection()
    {
        ResourceManager::get()->WaitForDeviceDetection();
    }

    void RegisterDeviceListChangeCallback(BridgeDeviceListChangeCallback callback, void* arg)
    {
        ResourceManager::get()->RegisterDeviceListChangeCallback(callback, arg);
    }

    void UnregisterDeviceListChangeCallback(BridgeDeviceListChangeCallback callback, void* arg)
    {
        ResourceManager::get()->UnregisterDeviceListChangeCallback(callback, arg);
    }

    std::vector<RGBController*> GetRGBControllers()
    {
        return ResourceManager::get()->GetRGBControllers();
    }

    void RegisterVirtualRGBController(RGBController* controller)
    {
        if (controller)
        {
            ResourceManager::get()->RegisterRGBController(controller);
        }
    }

    void UnregisterVirtualRGBController(RGBController* controller)
    {
        if (controller)
        {
            ResourceManager::get()->UnregisterRGBController(controller);
        }
    }

    void DeleteController(RGBController* controller)
    {
        delete controller;
    }

    void UpdateDeviceList()
    {
        ResourceManager::get()->UpdateDeviceList();
    }

    nlohmann::json GetSettings(const std::string& key)
    {
        SettingsManager* sm = ResourceManager::get()->GetSettingsManager();
        return sm ? sm->GetSettings(key) : nlohmann::json::object();
    }

    void SetSettings(const std::string& key, const nlohmann::json& val)
    {
        SettingsManager* sm = ResourceManager::get()->GetSettingsManager();
        if (sm)
        {
            sm->SetSettings(key, val);
        }
    }

    void SaveSettings()
    {
        SettingsManager* sm = ResourceManager::get()->GetSettingsManager();
        if (sm)
        {
            sm->SaveSettings();
        }
    }

    std::vector<std::string> GetProfileList()
    {
        ProfileManager* pm = ResourceManager::get()->GetProfileManager();
        return pm ? pm->profile_list : std::vector<std::string>();
    }

    bool LoadProfile(const std::string& profile_name)
    {
        ProfileManager* pm = ResourceManager::get()->GetProfileManager();
        return pm ? pm->LoadProfile(profile_name) : false;
    }

    bool SaveProfileFromPlugin(const std::string& profile_name, const std::string& plugin_name, const nlohmann::json& plugin_data)
    {
        if (profile_name.empty()) return false;
        filesystem::path path = ResourceManager::get()->GetConfigurationDirectory() / (profile_name + ".orp");
        nlohmann::json profile_json;
        if (filesystem::exists(path))
        {
            try
            {
                std::ifstream in(path);
                if (in.is_open())
                {
                    in >> profile_json;
                }
            }
            catch (...) {}
        }

        profile_json["profile_name"] = profile_name;
        profile_json["plugins"][plugin_name] = plugin_data;

        try
        {
            std::ofstream out(path);
            if (out.is_open())
            {
                out << profile_json.dump(4);
                return true;
            }
        }
        catch (...) {}
        return false;
    }

    void Log(const char* filename, int line, unsigned int level, const char* fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        LogEntry(filename, line, level, fmt, va);
        va_end(va);
    }

    void LogEntry(const char* filename, int line, unsigned int level, const char* fmt, va_list va)
    {
        LogManager* lm = LogManager::get();
        if (lm)
        {
            char buf[2048];
            vsnprintf(buf, sizeof(buf), fmt, va);
            lm->append(filename, line, level, "%s", buf);
        }
    }
}
