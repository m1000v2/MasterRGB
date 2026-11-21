#pragma once
#include "RGBControllerInterface.h"

class RGBController;

class RGBControllerAdapter : public RGBControllerInterface
{
public:
    RGBControllerAdapter(RGBController* ctrl);
    virtual ~RGBControllerAdapter();

    RGBController* GetRawController() const { return controller; }

    /* Identification */
    std::string GetName() override;
    std::string GetVendor() override;
    std::string GetDescription() override;
    std::string GetVersion() override;
    std::string GetSerial() override;
    std::string GetLocation() override;
    std::string GetDisplayName() override;
    device_type GetDeviceType() override;
    controller_flags GetFlags() override;
    bool GetHidden() override;
    void SetHidden(bool hidden) override;

    /* Zones */
    zone GetZone(unsigned int zone_idx) override;
    int GetZoneActiveMode(unsigned int zone) override;
    RGBColor GetZoneColor(unsigned int zone, unsigned int color_index) override;
    RGBColor* GetZoneColorsPointer(unsigned int zone) override;
    unsigned int GetZoneCount() override;
    std::string GetZoneDisplayName(unsigned int zone) override;
    zone_flags GetZoneFlags(unsigned int zone) override;
    unsigned int GetZoneLEDsCount(unsigned int zone) override;
    unsigned int GetZoneLEDsMax(unsigned int zone) override;
    unsigned int GetZoneLEDsMin(unsigned int zone) override;
    matrix_map_type GetZoneMatrixMap(unsigned int zone) override;
    const unsigned int* GetZoneMatrixMapData(unsigned int zone) override;
    unsigned int GetZoneMatrixMapHeight(unsigned int zone) override;
    unsigned int GetZoneMatrixMapWidth(unsigned int zone) override;
    unsigned int GetZoneModeCount(unsigned int zone) override;
    unsigned int GetZoneModeBrightness(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeBrightnessMax(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeBrightnessMin(unsigned int zone, unsigned int mode) override;
    RGBColor GetZoneModeColor(unsigned int zone, unsigned int mode, unsigned int color_index) override;
    unsigned int GetZoneModeColorMode(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeColorsCount(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeColorsMax(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeColorsMin(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeDirection(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeFlags(unsigned int zone, unsigned int mode) override;
    std::string GetZoneModeName(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeSpeed(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeSpeedMax(unsigned int zone, unsigned int mode) override;
    unsigned int GetZoneModeSpeedMin(unsigned int zone, unsigned int mode) override;
    std::string GetZoneName(unsigned int zone) override;
    unsigned int GetZoneSegmentCount(unsigned int zone) override;
    segment_flags GetZoneSegmentFlags(unsigned int zone, unsigned int segment) override;
    unsigned int GetZoneSegmentLEDsCount(unsigned int zone, unsigned int segment) override;
    matrix_map_type GetZoneSegmentMatrixMap(unsigned int zone, unsigned int segment) override;
    const unsigned int *GetZoneSegmentMatrixMapData(unsigned int zone, unsigned int segment) override;
    unsigned int GetZoneSegmentMatrixMapHeight(unsigned int zone, unsigned int segment) override;
    unsigned int GetZoneSegmentMatrixMapWidth(unsigned int zone, unsigned int segment) override;
    std::string GetZoneSegmentName(unsigned int zone, unsigned int segment) override;
    unsigned int GetZoneSegmentStartIndex(unsigned int zone, unsigned int segment) override;
    unsigned int GetZoneSegmentType(unsigned int zone, unsigned int segment) override;
    unsigned int GetZoneStartIndex(unsigned int zone) override;
    zone_type GetZoneType(unsigned int zone) override;
    unsigned int GetLEDsInZone(unsigned int zone) override;
    void SetZoneActiveMode(unsigned int zone, int mode) override;
    void SetZoneColor(unsigned int zone, unsigned int color_index, RGBColor color) override;
    void SetZoneModeBrightness(unsigned int zone, unsigned int mode, unsigned int brightness) override;
    void SetZoneModeColor(unsigned int zone, unsigned int mode, unsigned int color_index, RGBColor color) override;
    void SetZoneModeColorMode(unsigned int zone, unsigned int mode, unsigned int color_mode) override;
    void SetZoneModeColorsCount(unsigned int zone, unsigned int mode, unsigned int count) override;
    void SetZoneModeDirection(unsigned int zone, unsigned int mode, unsigned int direction) override;
    void SetZoneModeSpeed(unsigned int zone, unsigned int mode, unsigned int speed) override;
    bool SupportsPerZoneModes() override;

    /* Modes */
    RGBColor GetModeColor(unsigned int mode, unsigned int color_index) override;
    unsigned int GetModeColorMode(unsigned int mode) override;
    unsigned int GetModeCount() override;
    unsigned int GetModeBrightness(unsigned int mode) override;
    unsigned int GetModeBrightnessMax(unsigned int mode) override;
    unsigned int GetModeBrightnessMin(unsigned int mode) override;
    unsigned int GetModeColorsCount(unsigned int mode) override;
    unsigned int GetModeColorsMax(unsigned int mode) override;
    unsigned int GetModeColorsMin(unsigned int mode) override;
    unsigned int GetModeDirection(unsigned int mode) override;
    unsigned int GetModeFlags(unsigned int mode) override;
    std::string GetModeName(unsigned int mode) override;
    unsigned int GetModeSpeed(unsigned int mode) override;
    unsigned int GetModeSpeedMax(unsigned int mode) override;
    unsigned int GetModeSpeedMin(unsigned int mode) override;
    void SetModeBrightness(unsigned int mode, unsigned int brightness) override;
    void SetModeColor(unsigned int mode, unsigned int color_index, RGBColor color) override;
    void SetModeColorMode(unsigned int mode, unsigned int color_mode) override;
    void SetModeColorsCount(unsigned int mode, unsigned int count) override;
    void SetModeDirection(unsigned int mode, unsigned int direction) override;
    void SetModeSpeed(unsigned int mode, unsigned int speed) override;
    int GetActiveMode() override;
    void SetActiveMode(int mode) override;
    void SetCustomMode() override;

    /* LEDs */
    unsigned int GetLEDCount() override;
    std::string GetLEDName(unsigned int led) override;
    std::string GetLEDDisplayName(unsigned int led) override;

    /* Colors */
    RGBColor GetColor(unsigned int led) override;
    RGBColor* GetColorsPointer() override;
    void SetColor(unsigned int led, RGBColor color) override;
    void SetAllColors(RGBColor color) override;
    void SetAllZoneColors(int zone, RGBColor color) override;

    /* Device-Specific Configuration */
    nlohmann::json GetDeviceSpecificConfigurationSchema() override;
    nlohmann::json GetDeviceSpecificConfiguration() override;
    void SetDeviceSpecificConfiguration(nlohmann::json configuration_json) override;
    nlohmann::json GetDeviceSpecificZoneConfigurationSchema(int zone) override;
    nlohmann::json GetDeviceSpecificZoneConfiguration(int zone) override;
    void SetDeviceSpecificZoneConfiguration(int zone, nlohmann::json configuration_json) override;

    /* Callbacks */
    void RegisterUpdateCallback(RGBControllerCallback new_callback, void * new_callback_arg) override;
    void UnregisterUpdateCallback(void * callback_arg) override;
    void ClearCallbacks() override;
    void SignalUpdate(unsigned int update_reason) override;

    /* Updates */
    void UpdateLEDs() override;
    void UpdateZoneLEDs(int zone) override;
    void UpdateSingleLED(int led) override;
    void UpdateMode() override;
    void UpdateZoneMode(int zone) override;
    void SaveMode() override;
    void ClearSegments(int zone) override;
    void AddSegment(int zone, segment new_segment) override;
    void ConfigureZone(int zone_idx, zone new_zone) override;
    void ResizeZone(int zone, int new_size) override;
    void ConfigureDevice(controller_flags new_flags, std::string new_name) override;

private:
    RGBController* controller;
    nlohmann::json dev_config;
};
