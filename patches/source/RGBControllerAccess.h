#pragma once
#include <string>
#include <vector>

class RGBController;

struct ZoneAccessData
{
    std::string name;
    int type = 0;
    unsigned int start_idx = 0;
    unsigned int leds_count = 0;
    unsigned int leds_min = 0;
    unsigned int leds_max = 0;
    unsigned int flags = 0;
    unsigned int matrix_map_height = 0;
    unsigned int matrix_map_width = 0;
    std::vector<unsigned int> matrix_map_data;

    struct SegmentAccessData
    {
        std::string name;
        int type = 0;
        unsigned int start_idx = 0;
        unsigned int leds_count = 0;
    };
    std::vector<SegmentAccessData> segments;
};

struct ModeAccessData
{
    std::string name;
    int value = 0;
    unsigned int flags = 0;
    unsigned int speed_min = 0;
    unsigned int speed_max = 0;
    unsigned int brightness_min = 0;
    unsigned int brightness_max = 0;
    unsigned int colors_min = 0;
    unsigned int colors_max = 0;
    unsigned int speed = 0;
    unsigned int brightness = 0;
    unsigned int direction = 0;
    unsigned int color_mode = 0;
    std::vector<unsigned int> colors;
};

struct VirtualSetupData
{
    std::string description;
    std::string location;
    std::string name;
    std::string serial;
    std::string vendor;
    std::string version;
    int active_mode = 0;
    unsigned int flags = 0;
    int type = 0;

    std::vector<ModeAccessData> modes;
    std::vector<ZoneAccessData> zones;
    struct LEDData { std::string name; unsigned int value; };
    std::vector<LEDData> leds;

    void* object_ptr = nullptr;
    void (*DeviceUpdateLEDs)(void*) = nullptr;
    void (*DeviceUpdateZoneLEDs)(void*, int) = nullptr;
    void (*DeviceUpdateSingleLED)(void*, int) = nullptr;
    void (*DeviceUpdateMode)(void*) = nullptr;
    void (*DeviceConfigureZone)(void*, int) = nullptr;
};

namespace RGBControllerAccess
{
    std::string GetName(RGBController* c);
    std::string GetVendor(RGBController* c);
    std::string GetDescription(RGBController* c);
    std::string GetVersion(RGBController* c);
    std::string GetSerial(RGBController* c);
    std::string GetLocation(RGBController* c);
    int GetDeviceType(RGBController* c);
    unsigned int GetFlags(RGBController* c);

    unsigned int GetLEDCount(RGBController* c);
    std::string GetLEDName(RGBController* c, unsigned int led);
    unsigned int GetLEDValue(RGBController* c, unsigned int led);
    unsigned int* GetColorsPointer(RGBController* c);
    unsigned int GetColor(RGBController* c, unsigned int led);
    void SetColor(RGBController* c, unsigned int led, unsigned int color);
    void SetAllColors(RGBController* c, unsigned int color);
    void SetAllZoneColors(RGBController* c, int zone, unsigned int color);

    void UpdateLEDs(RGBController* c);
    void UpdateZoneLEDs(RGBController* c, int zone);
    void UpdateSingleLED(RGBController* c, int led);
    void UpdateMode(RGBController* c);
    void SaveMode(RGBController* c);

    unsigned int GetModeCount(RGBController* c);
    ModeAccessData GetMode(RGBController* c, unsigned int mode_idx);
    int GetActiveMode(RGBController* c);
    void SetActiveMode(RGBController* c, int mode);
    void SetCustomMode(RGBController* c);
    void SetMode(RGBController* c, unsigned int mode_idx, const ModeAccessData& mode_data);

    unsigned int GetZoneCount(RGBController* c);
    ZoneAccessData GetZone(RGBController* c, unsigned int zone_idx);
    void ResizeZone(RGBController* c, int zone, int new_size);

    void RegisterUpdateCallback(RGBController* c, void (*cb)(void*, unsigned int, void*), void* arg);
    void UnregisterUpdateCallback(RGBController* c, void* arg);
    void ClearCallbacks(RGBController* c);
    void SignalUpdate(RGBController* c, unsigned int reason);

    RGBController* CreateVirtualRGBController(const VirtualSetupData& setup);
    void UpdateVirtualRGBController(RGBController* c, const VirtualSetupData& setup);
}
