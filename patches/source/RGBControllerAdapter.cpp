#include "RGBControllerAdapter.h"
#include "RGBControllerAccess.h"

RGBControllerAdapter::RGBControllerAdapter(RGBController* ctrl)
    : controller(ctrl)
{
}

RGBControllerAdapter::~RGBControllerAdapter()
{
}

std::string RGBControllerAdapter::GetName() { return RGBControllerAccess::GetName(controller); }
std::string RGBControllerAdapter::GetVendor() { return RGBControllerAccess::GetVendor(controller); }
std::string RGBControllerAdapter::GetDescription() { return RGBControllerAccess::GetDescription(controller); }
std::string RGBControllerAdapter::GetVersion() { return RGBControllerAccess::GetVersion(controller); }
std::string RGBControllerAdapter::GetSerial() { return RGBControllerAccess::GetSerial(controller); }
std::string RGBControllerAdapter::GetLocation() { return RGBControllerAccess::GetLocation(controller); }
std::string RGBControllerAdapter::GetDisplayName() { return RGBControllerAccess::GetName(controller); }
device_type RGBControllerAdapter::GetDeviceType() { return (device_type)RGBControllerAccess::GetDeviceType(controller); }
controller_flags RGBControllerAdapter::GetFlags() { return (controller_flags)RGBControllerAccess::GetFlags(controller); }
bool RGBControllerAdapter::GetHidden() { return false; }
void RGBControllerAdapter::SetHidden(bool /*hidden*/) {}

zone RGBControllerAdapter::GetZone(unsigned int zone_idx)
{
    ZoneAccessData zad = RGBControllerAccess::GetZone(controller, zone_idx);
    zone z;
    z.name = zad.name;
    z.display_name = zad.name;
    z.type = (zone_type)zad.type;
    z.start_idx = zad.start_idx;
    z.leds_count = zad.leds_count;
    z.leds_min = zad.leds_min;
    z.leds_max = zad.leds_max;
    z.flags = zad.flags;
    z.matrix_map.height = zad.matrix_map_height;
    z.matrix_map.width = zad.matrix_map_width;
    z.matrix_map.map = zad.matrix_map_data;
    for (const auto& s : zad.segments)
    {
        segment seg;
        seg.name = s.name;
        seg.type = (zone_type)s.type;
        seg.start_idx = s.start_idx;
        seg.leds_count = s.leds_count;
        z.segments.push_back(seg);
    }
    return z;
}

int RGBControllerAdapter::GetZoneActiveMode(unsigned int /*zone*/) { return -1; }
RGBColor RGBControllerAdapter::GetZoneColor(unsigned int zone, unsigned int color_index)
{
    unsigned int start = RGBControllerAccess::GetZone(controller, zone).start_idx;
    return RGBControllerAccess::GetColor(controller, start + color_index);
}
RGBColor* RGBControllerAdapter::GetZoneColorsPointer(unsigned int zone)
{
    unsigned int* ptr = RGBControllerAccess::GetColorsPointer(controller);
    if (!ptr) return nullptr;
    unsigned int start = RGBControllerAccess::GetZone(controller, zone).start_idx;
    return ptr + start;
}
unsigned int RGBControllerAdapter::GetZoneCount() { return RGBControllerAccess::GetZoneCount(controller); }
std::string RGBControllerAdapter::GetZoneDisplayName(unsigned int zone) { return RGBControllerAccess::GetZone(controller, zone).name; }
zone_flags RGBControllerAdapter::GetZoneFlags(unsigned int zone) { return RGBControllerAccess::GetZone(controller, zone).flags; }
unsigned int RGBControllerAdapter::GetZoneLEDsCount(unsigned int zone) { return RGBControllerAccess::GetZone(controller, zone).leds_count; }
unsigned int RGBControllerAdapter::GetZoneLEDsMax(unsigned int zone) { return RGBControllerAccess::GetZone(controller, zone).leds_max; }
unsigned int RGBControllerAdapter::GetZoneLEDsMin(unsigned int zone) { return RGBControllerAccess::GetZone(controller, zone).leds_min; }
matrix_map_type RGBControllerAdapter::GetZoneMatrixMap(unsigned int zone)
{
    ZoneAccessData zad = RGBControllerAccess::GetZone(controller, zone);
    matrix_map_type m;
    m.height = zad.matrix_map_height;
    m.width = zad.matrix_map_width;
    m.map = zad.matrix_map_data;
    return m;
}
const unsigned int* RGBControllerAdapter::GetZoneMatrixMapData(unsigned int zone)
{
    static thread_local std::vector<unsigned int> tls_map;
    tls_map = RGBControllerAccess::GetZone(controller, zone).matrix_map_data;
    return tls_map.empty() ? nullptr : tls_map.data();
}
unsigned int RGBControllerAdapter::GetZoneMatrixMapHeight(unsigned int zone) { return RGBControllerAccess::GetZone(controller, zone).matrix_map_height; }
unsigned int RGBControllerAdapter::GetZoneMatrixMapWidth(unsigned int zone) { return RGBControllerAccess::GetZone(controller, zone).matrix_map_width; }
unsigned int RGBControllerAdapter::GetZoneModeCount(unsigned int /*zone*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeBrightness(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeBrightnessMax(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeBrightnessMin(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
RGBColor RGBControllerAdapter::GetZoneModeColor(unsigned int /*zone*/, unsigned int /*mode*/, unsigned int /*color_index*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeColorMode(unsigned int /*zone*/, unsigned int mode) { return GetModeColorMode(mode); }
unsigned int RGBControllerAdapter::GetZoneModeColorsCount(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeColorsMax(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeColorsMin(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeDirection(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeFlags(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
std::string RGBControllerAdapter::GetZoneModeName(unsigned int /*zone*/, unsigned int /*mode*/) { return ""; }
unsigned int RGBControllerAdapter::GetZoneModeSpeed(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeSpeedMax(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneModeSpeedMin(unsigned int /*zone*/, unsigned int /*mode*/) { return 0; }
std::string RGBControllerAdapter::GetZoneName(unsigned int zone) { return RGBControllerAccess::GetZone(controller, zone).name; }
unsigned int RGBControllerAdapter::GetZoneSegmentCount(unsigned int zone)
{
    ZoneAccessData zad = RGBControllerAccess::GetZone(controller, zone);
    return (unsigned int)zad.segments.size();
}
segment_flags RGBControllerAdapter::GetZoneSegmentFlags(unsigned int /*zone*/, unsigned int /*segment*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneSegmentLEDsCount(unsigned int zone, unsigned int segment)
{
    ZoneAccessData zad = RGBControllerAccess::GetZone(controller, zone);
    if (segment < zad.segments.size()) return zad.segments[segment].leds_count;
    return 0;
}
matrix_map_type RGBControllerAdapter::GetZoneSegmentMatrixMap(unsigned int /*zone*/, unsigned int /*segment*/) { return matrix_map_type(); }
const unsigned int *RGBControllerAdapter::GetZoneSegmentMatrixMapData(unsigned int /*zone*/, unsigned int /*segment*/) { return nullptr; }
unsigned int RGBControllerAdapter::GetZoneSegmentMatrixMapHeight(unsigned int /*zone*/, unsigned int /*segment*/) { return 0; }
unsigned int RGBControllerAdapter::GetZoneSegmentMatrixMapWidth(unsigned int /*zone*/, unsigned int /*segment*/) { return 0; }
std::string RGBControllerAdapter::GetZoneSegmentName(unsigned int zone, unsigned int segment)
{
    ZoneAccessData zad = RGBControllerAccess::GetZone(controller, zone);
    if (segment < zad.segments.size()) return zad.segments[segment].name;
    return "";
}
unsigned int RGBControllerAdapter::GetZoneSegmentStartIndex(unsigned int zone, unsigned int segment)
{
    ZoneAccessData zad = RGBControllerAccess::GetZone(controller, zone);
    if (segment < zad.segments.size()) return zad.segments[segment].start_idx;
    return 0;
}
unsigned int RGBControllerAdapter::GetZoneSegmentType(unsigned int zone, unsigned int segment)
{
    ZoneAccessData zad = RGBControllerAccess::GetZone(controller, zone);
    if (segment < zad.segments.size()) return (unsigned int)zad.segments[segment].type;
    return 0;
}
unsigned int RGBControllerAdapter::GetZoneStartIndex(unsigned int zone)
{
    return RGBControllerAccess::GetZone(controller, zone).start_idx;
}
zone_type RGBControllerAdapter::GetZoneType(unsigned int zone) { return (zone_type)RGBControllerAccess::GetZone(controller, zone).type; }
unsigned int RGBControllerAdapter::GetLEDsInZone(unsigned int zone)
{
    return RGBControllerAccess::GetZone(controller, zone).leds_count;
}
void RGBControllerAdapter::SetZoneActiveMode(unsigned int /*zone*/, int /*mode*/) {}
void RGBControllerAdapter::SetZoneColor(unsigned int zone, unsigned int color_index, RGBColor color)
{
    unsigned int start = RGBControllerAccess::GetZone(controller, zone).start_idx;
    RGBControllerAccess::SetColor(controller, start + color_index, color);
}
void RGBControllerAdapter::SetZoneModeBrightness(unsigned int /*zone*/, unsigned int /*mode*/, unsigned int /*brightness*/) {}
void RGBControllerAdapter::SetZoneModeColor(unsigned int /*zone*/, unsigned int /*mode*/, unsigned int /*color_index*/, RGBColor /*color*/) {}
void RGBControllerAdapter::SetZoneModeColorMode(unsigned int /*zone*/, unsigned int /*mode*/, unsigned int /*color_mode*/) {}
void RGBControllerAdapter::SetZoneModeColorsCount(unsigned int /*zone*/, unsigned int /*mode*/, unsigned int /*count*/) {}
void RGBControllerAdapter::SetZoneModeDirection(unsigned int /*zone*/, unsigned int /*mode*/, unsigned int /*direction*/) {}
void RGBControllerAdapter::SetZoneModeSpeed(unsigned int /*zone*/, unsigned int /*mode*/, unsigned int /*speed*/) {}
bool RGBControllerAdapter::SupportsPerZoneModes() { return false; }

RGBColor RGBControllerAdapter::GetModeColor(unsigned int mode, unsigned int color_index)
{
    ModeAccessData md = RGBControllerAccess::GetMode(controller, mode);
    if (color_index < md.colors.size()) return md.colors[color_index];
    return 0;
}
unsigned int RGBControllerAdapter::GetModeColorMode(unsigned int mode)
{
    return RGBControllerAccess::GetMode(controller, mode).color_mode;
}
unsigned int RGBControllerAdapter::GetModeCount() { return RGBControllerAccess::GetModeCount(controller); }
unsigned int RGBControllerAdapter::GetModeBrightness(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).brightness; }
unsigned int RGBControllerAdapter::GetModeBrightnessMax(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).brightness_max; }
unsigned int RGBControllerAdapter::GetModeBrightnessMin(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).brightness_min; }
unsigned int RGBControllerAdapter::GetModeColorsCount(unsigned int mode) { return (unsigned int)RGBControllerAccess::GetMode(controller, mode).colors.size(); }
unsigned int RGBControllerAdapter::GetModeColorsMax(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).colors_max; }
unsigned int RGBControllerAdapter::GetModeColorsMin(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).colors_min; }
unsigned int RGBControllerAdapter::GetModeDirection(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).direction; }
unsigned int RGBControllerAdapter::GetModeFlags(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).flags; }
std::string RGBControllerAdapter::GetModeName(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).name; }
unsigned int RGBControllerAdapter::GetModeSpeed(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).speed; }
unsigned int RGBControllerAdapter::GetModeSpeedMax(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).speed_max; }
unsigned int RGBControllerAdapter::GetModeSpeedMin(unsigned int mode) { return RGBControllerAccess::GetMode(controller, mode).speed_min; }

void RGBControllerAdapter::SetModeBrightness(unsigned int mode, unsigned int brightness)
{
    ModeAccessData md = RGBControllerAccess::GetMode(controller, mode);
    md.brightness = brightness;
    RGBControllerAccess::SetMode(controller, mode, md);
}
void RGBControllerAdapter::SetModeColor(unsigned int mode, unsigned int color_index, RGBColor color)
{
    ModeAccessData md = RGBControllerAccess::GetMode(controller, mode);
    if (color_index < md.colors.size())
    {
        md.colors[color_index] = color;
        RGBControllerAccess::SetMode(controller, mode, md);
    }
}
void RGBControllerAdapter::SetModeColorMode(unsigned int mode, unsigned int color_mode)
{
    ModeAccessData md = RGBControllerAccess::GetMode(controller, mode);
    md.color_mode = color_mode;
    RGBControllerAccess::SetMode(controller, mode, md);
}
void RGBControllerAdapter::SetModeColorsCount(unsigned int mode, unsigned int count)
{
    ModeAccessData md = RGBControllerAccess::GetMode(controller, mode);
    md.colors.resize(count);
    RGBControllerAccess::SetMode(controller, mode, md);
}
void RGBControllerAdapter::SetModeDirection(unsigned int mode, unsigned int direction)
{
    ModeAccessData md = RGBControllerAccess::GetMode(controller, mode);
    md.direction = direction;
    RGBControllerAccess::SetMode(controller, mode, md);
}
void RGBControllerAdapter::SetModeSpeed(unsigned int mode, unsigned int speed)
{
    ModeAccessData md = RGBControllerAccess::GetMode(controller, mode);
    md.speed = speed;
    RGBControllerAccess::SetMode(controller, mode, md);
}
int RGBControllerAdapter::GetActiveMode() { return RGBControllerAccess::GetActiveMode(controller); }
void RGBControllerAdapter::SetActiveMode(int mode) { RGBControllerAccess::SetActiveMode(controller, mode); }
void RGBControllerAdapter::SetCustomMode() { RGBControllerAccess::SetCustomMode(controller); }

unsigned int RGBControllerAdapter::GetLEDCount() { return RGBControllerAccess::GetLEDCount(controller); }
std::string RGBControllerAdapter::GetLEDName(unsigned int led) { return RGBControllerAccess::GetLEDName(controller, led); }
std::string RGBControllerAdapter::GetLEDDisplayName(unsigned int led) { return RGBControllerAccess::GetLEDName(controller, led); }

RGBColor RGBControllerAdapter::GetColor(unsigned int led) { return RGBControllerAccess::GetColor(controller, led); }
RGBColor* RGBControllerAdapter::GetColorsPointer() { return RGBControllerAccess::GetColorsPointer(controller); }
void RGBControllerAdapter::SetColor(unsigned int led, RGBColor color) { RGBControllerAccess::SetColor(controller, led, color); }
void RGBControllerAdapter::SetAllColors(RGBColor color) { RGBControllerAccess::SetAllColors(controller, color); }
void RGBControllerAdapter::SetAllZoneColors(int zone, RGBColor color) { RGBControllerAccess::SetAllZoneColors(controller, zone, color); }

nlohmann::json RGBControllerAdapter::GetDeviceSpecificConfigurationSchema() { return nlohmann::json::object(); }
nlohmann::json RGBControllerAdapter::GetDeviceSpecificConfiguration() { return dev_config; }
void RGBControllerAdapter::SetDeviceSpecificConfiguration(nlohmann::json configuration_json) { dev_config = configuration_json; }
nlohmann::json RGBControllerAdapter::GetDeviceSpecificZoneConfigurationSchema(int /*zone*/) { return nlohmann::json::object(); }
nlohmann::json RGBControllerAdapter::GetDeviceSpecificZoneConfiguration(int /*zone*/) { return nlohmann::json::object(); }
void RGBControllerAdapter::SetDeviceSpecificZoneConfiguration(int /*zone*/, nlohmann::json /*configuration_json*/) {}

void RGBControllerAdapter::RegisterUpdateCallback(RGBControllerCallback new_callback, void * new_callback_arg)
{
    RGBControllerAccess::RegisterUpdateCallback(controller, new_callback, new_callback_arg);
}
void RGBControllerAdapter::UnregisterUpdateCallback(void * callback_arg)
{
    RGBControllerAccess::UnregisterUpdateCallback(controller, callback_arg);
}
void RGBControllerAdapter::ClearCallbacks()
{
    RGBControllerAccess::ClearCallbacks(controller);
}
void RGBControllerAdapter::SignalUpdate(unsigned int update_reason)
{
    RGBControllerAccess::SignalUpdate(controller, update_reason);
}

void RGBControllerAdapter::UpdateLEDs() { RGBControllerAccess::UpdateLEDs(controller); }
void RGBControllerAdapter::UpdateZoneLEDs(int zone) { RGBControllerAccess::UpdateZoneLEDs(controller, zone); }
void RGBControllerAdapter::UpdateSingleLED(int led) { RGBControllerAccess::UpdateSingleLED(controller, led); }
void RGBControllerAdapter::UpdateMode() { RGBControllerAccess::UpdateMode(controller); }
void RGBControllerAdapter::UpdateZoneMode(int /*zone*/) { RGBControllerAccess::UpdateMode(controller); }
void RGBControllerAdapter::SaveMode() { RGBControllerAccess::SaveMode(controller); }
void RGBControllerAdapter::ClearSegments(int /*zone*/) {}
void RGBControllerAdapter::AddSegment(int /*zone*/, segment /*new_segment*/) {}
void RGBControllerAdapter::ConfigureZone(int /*zone_idx*/, zone /*new_zone*/) {}
void RGBControllerAdapter::ResizeZone(int zone, int new_size) { RGBControllerAccess::ResizeZone(controller, zone, new_size); }
void RGBControllerAdapter::ConfigureDevice(controller_flags /*new_flags*/, std::string /*new_name*/) {}
