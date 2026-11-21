#pragma once
#include <string>
#include <vector>
#include <cstdarg>
#include <nlohmann/json.hpp>
#include "filesystem.h"

class RGBController;

typedef void (*BridgeDeviceListChangeCallback)(void*);

namespace MasterRGBPluginBridge
{
    filesystem::path GetConfigurationDirectory();
    bool GetDetectionEnabled();
    unsigned int GetDetectionPercent();
    std::string GetDetectionString();
    void RescanDevices();
    void WaitForDetection();

    void RegisterDeviceListChangeCallback(BridgeDeviceListChangeCallback callback, void* arg);
    void UnregisterDeviceListChangeCallback(BridgeDeviceListChangeCallback callback, void* arg);

    std::vector<RGBController*> GetRGBControllers();
    void RegisterVirtualRGBController(RGBController* controller);
    void UnregisterVirtualRGBController(RGBController* controller);
    void DeleteController(RGBController* controller);
    void UpdateDeviceList();

    nlohmann::json GetSettings(const std::string& key);
    void SetSettings(const std::string& key, const nlohmann::json& val);
    void SaveSettings();

    std::vector<std::string> GetProfileList();
    bool LoadProfile(const std::string& profile_name);
    bool SaveProfileFromPlugin(const std::string& profile_name, const std::string& plugin_name, const nlohmann::json& plugin_data);

    void Log(const char* filename, int line, unsigned int level, const char* fmt, ...);
    void LogEntry(const char* filename, int line, unsigned int level, const char* fmt, va_list va);
}
