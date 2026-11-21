#pragma once

#include "OpenRGBPluginInterface.h"
#include <vector>

class RGBControllerAdapter;

class OpenRGBPluginAPI : public OpenRGBPluginAPIInterface
{
public:
    OpenRGBPluginAPI();
    virtual ~OpenRGBPluginAPI();

    /* LogManager APIs */
    void LogEntry(const char* filename, int line, unsigned int level, const char* fmt, ...) override;

    /* PluginManager APIs */
    RGBControllerInterface* CreateVirtualRGBController(RGBController_Setup* setup) override;
    void DeleteVirtualRGBController(RGBControllerInterface* rgb_controller) override;
    void RegisterVirtualRGBController(RGBControllerInterface* rgb_controller) override;
    void RegisterVirtualRGBControllerInThread(RGBControllerInterface* rgb_controller) override;
    void UnregisterVirtualRGBController(RGBControllerInterface* rgb_controller) override;
    void UpdateVirtualRGBController(RGBControllerInterface* rgb_controller, RGBController_Setup* setup) override;
    void UnregisterVirtualRGBControllerInThread(RGBControllerInterface* rgb_controller) override;

    /* ProfileManager APIs */
    void ClearActiveProfile() override;
    std::vector<std::string> GetProfileList() override;
    bool LoadProfile(std::string profile_name) override;
    bool SaveProfileFromPlugin(std::string profile_name, std::string plugin_name, nlohmann::json plugin_data) override;

    /* ResourceManager APIs */
    filesystem::path GetConfigurationDirectory() override;
    bool GetDetectionEnabled() override;
    unsigned int GetDetectionPercent() override;
    std::string GetDetectionString() override;
    void RescanDevices() override;
    void WaitForDetection() override;
    std::vector<RGBControllerInterface*> GetRGBControllers() override;

    /* RGBController JSON APIs */
    nlohmann::json GetDeviceDescriptionJSON(RGBControllerInterface* controller) override;
    nlohmann::json GetLEDDescriptionJSON(led led) override;
    nlohmann::json GetMatrixMapDescriptionJSON(matrix_map_type matrix_map) override;
    nlohmann::json GetModeDescriptionJSON(mode mode) override;
    nlohmann::json GetSegmentDescriptionJSON(segment segment) override;
    nlohmann::json GetZoneDescriptionJSON(zone zone) override;

    RGBControllerInterface* SetDeviceDescriptionJSON(nlohmann::json controller_json) override;
    led SetLEDDescriptionJSON(nlohmann::json led_json) override;
    matrix_map_type SetMatrixMapDescriptionJSON(nlohmann::json matrix_map_json) override;
    mode SetModeDescriptionJSON(nlohmann::json mode_json) override;
    segment SetSegmentDescriptionJSON(nlohmann::json segment_json) override;
    zone SetZoneDescriptionJSON(nlohmann::json zone_json) override;

    bool CompareControllers(RGBControllerInterface* controller_1, RGBControllerInterface* controller_2) override;
    std::string DeviceTypeToString(device_type type) override;
    bool SetModeValuesFromMode(mode& destination, mode& source) override;

    /* SettingsManager APIs */
    nlohmann::json GetSettings(std::string settings_key) override;
    void SaveSettings() override;
    void SetSettings(std::string settings_key, nlohmann::json new_settings) override;

    /* Registered controllers */
    std::vector<RGBControllerInterface*> rgb_controllers;
    std::vector<RGBControllerInterface*> created_controllers;

private:
    std::vector<RGBControllerAdapter*> managed_adapters;
};
