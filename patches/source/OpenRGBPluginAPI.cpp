#include "OpenRGBPluginAPI.h"
#include "MasterRGBPluginBridge.h"
#include "RGBControllerAccess.h"
#include "RGBControllerAdapter.h"
#include <cstdarg>
#include <thread>
#include <algorithm>

OpenRGBPluginAPI::OpenRGBPluginAPI()
{
}

OpenRGBPluginAPI::~OpenRGBPluginAPI()
{
    for (RGBControllerAdapter* adapter : managed_adapters)
    {
        delete adapter;
    }
    managed_adapters.clear();
}

void OpenRGBPluginAPI::LogEntry(const char* filename, int line, unsigned int level, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    MasterRGBPluginBridge::LogEntry(filename, line, level, fmt, va);
    va_end(va);
}

RGBControllerInterface* OpenRGBPluginAPI::CreateVirtualRGBController(RGBController_Setup* setup)
{
    VirtualSetupData vsd;
    vsd.name = setup->name;
    vsd.description = setup->description;
    vsd.vendor = setup->vendor;
    vsd.location = setup->location;
    vsd.version = setup->version;
    vsd.serial = setup->serial;
    vsd.type = (int)setup->type;
    vsd.flags = setup->flags;
    vsd.active_mode = setup->active_mode;
    vsd.object_ptr = setup->object_ptr;
    vsd.DeviceUpdateLEDs = setup->DeviceUpdateLEDs;
    vsd.DeviceUpdateZoneLEDs = setup->DeviceUpdateZoneLEDs;
    vsd.DeviceUpdateSingleLED = setup->DeviceUpdateSingleLED;
    vsd.DeviceUpdateMode = setup->DeviceUpdateMode;
    vsd.DeviceConfigureZone = setup->DeviceConfigureZone;

    for (const auto& m : setup->modes)
    {
        ModeAccessData md;
        md.name = m.name;
        md.value = m.value;
        md.flags = m.flags;
        md.speed_min = m.speed_min;
        md.speed_max = m.speed_max;
        md.brightness_min = m.brightness_min;
        md.brightness_max = m.brightness_max;
        md.colors_min = m.colors_min;
        md.colors_max = m.colors_max;
        md.speed = m.speed;
        md.brightness = m.brightness;
        md.direction = m.direction;
        md.color_mode = m.color_mode;
        md.colors = m.colors;
        vsd.modes.push_back(md);
    }

    for (const auto& z : setup->zones)
    {
        ZoneAccessData zd;
        zd.name = z.name;
        zd.type = (int)z.type;
        zd.start_idx = z.start_idx;
        zd.leds_count = z.leds_count;
        zd.leds_min = z.leds_min;
        zd.leds_max = z.leds_max;
        zd.flags = z.flags;
        zd.matrix_map_height = z.matrix_map.height;
        zd.matrix_map_width = z.matrix_map.width;
        zd.matrix_map_data = z.matrix_map.map;
        for (const auto& s : z.segments)
        {
            ZoneAccessData::SegmentAccessData sd;
            sd.name = s.name;
            sd.type = (int)s.type;
            sd.start_idx = s.start_idx;
            sd.leds_count = s.leds_count;
            zd.segments.push_back(sd);
        }
        vsd.zones.push_back(zd);
    }

    for (const auto& l : setup->leds)
    {
        vsd.leds.push_back({l.name, l.value});
    }

    RGBController* raw_virt = RGBControllerAccess::CreateVirtualRGBController(vsd);
    RGBControllerAdapter* adapter = new RGBControllerAdapter(raw_virt);
    managed_adapters.push_back(adapter);
    created_controllers.push_back(adapter);
    return adapter;
}

void OpenRGBPluginAPI::DeleteVirtualRGBController(RGBControllerInterface* rgb_controller)
{
    UnregisterVirtualRGBController(rgb_controller);

    auto it = std::find(created_controllers.begin(), created_controllers.end(), rgb_controller);
    if (it != created_controllers.end())
    {
        created_controllers.erase(it);
    }

    RGBControllerAdapter* adapter = dynamic_cast<RGBControllerAdapter*>(rgb_controller);
    if (adapter)
    {
        auto ait = std::find(managed_adapters.begin(), managed_adapters.end(), adapter);
        if (ait != managed_adapters.end())
        {
            managed_adapters.erase(ait);
        }
        RGBController* raw = adapter->GetRawController();
        delete adapter;
        MasterRGBPluginBridge::DeleteController(raw);
    }
}

void OpenRGBPluginAPI::RegisterVirtualRGBController(RGBControllerInterface* rgb_controller)
{
    auto it = std::find(created_controllers.begin(), created_controllers.end(), rgb_controller);
    if (it != created_controllers.end())
    {
        RGBControllerAdapter* adapter = dynamic_cast<RGBControllerAdapter*>(rgb_controller);
        if (adapter)
        {
            RGBController* raw = adapter->GetRawController();
            rgb_controllers.push_back(rgb_controller);
            MasterRGBPluginBridge::RegisterVirtualRGBController(raw);
            MasterRGBPluginBridge::UpdateDeviceList();
        }
    }
}

static void CallRegisterVirtualRGBController(OpenRGBPluginAPI* this_ptr, RGBControllerInterface* rgb_controller)
{
    this_ptr->RegisterVirtualRGBController(rgb_controller);
}

void OpenRGBPluginAPI::RegisterVirtualRGBControllerInThread(RGBControllerInterface* rgb_controller)
{
    std::thread t(CallRegisterVirtualRGBController, this, rgb_controller);
    t.detach();
}

void OpenRGBPluginAPI::UnregisterVirtualRGBController(RGBControllerInterface* rgb_controller)
{
    auto it = std::find(rgb_controllers.begin(), rgb_controllers.end(), rgb_controller);
    if (it != rgb_controllers.end())
    {
        rgb_controllers.erase(it);
        RGBControllerAdapter* adapter = dynamic_cast<RGBControllerAdapter*>(rgb_controller);
        if (adapter)
        {
            RGBController* raw = adapter->GetRawController();
            MasterRGBPluginBridge::UnregisterVirtualRGBController(raw);
            MasterRGBPluginBridge::UpdateDeviceList();
        }
    }
}

void OpenRGBPluginAPI::UpdateVirtualRGBController(RGBControllerInterface* rgb_controller, RGBController_Setup* setup)
{
    RGBControllerAdapter* adapter = dynamic_cast<RGBControllerAdapter*>(rgb_controller);
    if (!adapter) return;

    VirtualSetupData vsd;
    vsd.name = setup->name;
    vsd.description = setup->description;
    vsd.vendor = setup->vendor;
    vsd.location = setup->location;
    vsd.version = setup->version;
    vsd.serial = setup->serial;
    vsd.type = (int)setup->type;
    vsd.flags = setup->flags;
    vsd.active_mode = setup->active_mode;
    vsd.object_ptr = setup->object_ptr;
    vsd.DeviceUpdateLEDs = setup->DeviceUpdateLEDs;
    vsd.DeviceUpdateZoneLEDs = setup->DeviceUpdateZoneLEDs;
    vsd.DeviceUpdateSingleLED = setup->DeviceUpdateSingleLED;
    vsd.DeviceUpdateMode = setup->DeviceUpdateMode;
    vsd.DeviceConfigureZone = setup->DeviceConfigureZone;

    for (const auto& m : setup->modes)
    {
        ModeAccessData md;
        md.name = m.name;
        md.value = m.value;
        md.flags = m.flags;
        md.speed_min = m.speed_min;
        md.speed_max = m.speed_max;
        md.brightness_min = m.brightness_min;
        md.brightness_max = m.brightness_max;
        md.colors_min = m.colors_min;
        md.colors_max = m.colors_max;
        md.speed = m.speed;
        md.brightness = m.brightness;
        md.direction = m.direction;
        md.color_mode = m.color_mode;
        md.colors = m.colors;
        vsd.modes.push_back(md);
    }

    for (const auto& z : setup->zones)
    {
        ZoneAccessData zd;
        zd.name = z.name;
        zd.type = (int)z.type;
        zd.start_idx = z.start_idx;
        zd.leds_count = z.leds_count;
        zd.leds_min = z.leds_min;
        zd.leds_max = z.leds_max;
        zd.flags = z.flags;
        zd.matrix_map_height = z.matrix_map.height;
        zd.matrix_map_width = z.matrix_map.width;
        zd.matrix_map_data = z.matrix_map.map;
        for (const auto& s : z.segments)
        {
            ZoneAccessData::SegmentAccessData sd;
            sd.name = s.name;
            sd.type = (int)s.type;
            sd.start_idx = s.start_idx;
            sd.leds_count = s.leds_count;
            zd.segments.push_back(sd);
        }
        vsd.zones.push_back(zd);
    }

    for (const auto& l : setup->leds)
    {
        vsd.leds.push_back({l.name, l.value});
    }

    RGBControllerAccess::UpdateVirtualRGBController(adapter->GetRawController(), vsd);
    MasterRGBPluginBridge::UpdateDeviceList();
}

static void CallUnregisterVirtualRGBController(OpenRGBPluginAPI* this_ptr, RGBControllerInterface* rgb_controller)
{
    this_ptr->UnregisterVirtualRGBController(rgb_controller);
}

void OpenRGBPluginAPI::UnregisterVirtualRGBControllerInThread(RGBControllerInterface* rgb_controller)
{
    std::thread t(CallUnregisterVirtualRGBController, this, rgb_controller);
    t.detach();
}

void OpenRGBPluginAPI::ClearActiveProfile()
{
}

std::vector<std::string> OpenRGBPluginAPI::GetProfileList()
{
    return MasterRGBPluginBridge::GetProfileList();
}

bool OpenRGBPluginAPI::LoadProfile(std::string profile_name)
{
    return MasterRGBPluginBridge::LoadProfile(profile_name);
}

bool OpenRGBPluginAPI::SaveProfileFromPlugin(std::string profile_name, std::string plugin_name, nlohmann::json plugin_data)
{
    return MasterRGBPluginBridge::SaveProfileFromPlugin(profile_name, plugin_name, plugin_data);
}

filesystem::path OpenRGBPluginAPI::GetConfigurationDirectory()
{
    return MasterRGBPluginBridge::GetConfigurationDirectory();
}

bool OpenRGBPluginAPI::GetDetectionEnabled()
{
    return MasterRGBPluginBridge::GetDetectionEnabled();
}

unsigned int OpenRGBPluginAPI::GetDetectionPercent()
{
    return MasterRGBPluginBridge::GetDetectionPercent();
}

std::string OpenRGBPluginAPI::GetDetectionString()
{
    return MasterRGBPluginBridge::GetDetectionString();
}

void OpenRGBPluginAPI::RescanDevices()
{
    MasterRGBPluginBridge::RescanDevices();
}

void OpenRGBPluginAPI::WaitForDetection()
{
    MasterRGBPluginBridge::WaitForDetection();
}

std::vector<RGBControllerInterface*> OpenRGBPluginAPI::GetRGBControllers()
{
    std::vector<RGBController*> raw_list = MasterRGBPluginBridge::GetRGBControllers();
    std::vector<RGBControllerInterface*> result;

    for (RGBController* raw : raw_list)
    {
        bool found = false;
        for (RGBControllerAdapter* adapter : managed_adapters)
        {
            if (adapter->GetRawController() == raw)
            {
                result.push_back(adapter);
                found = true;
                break;
            }
        }
        if (!found)
        {
            RGBControllerAdapter* adapter = new RGBControllerAdapter(raw);
            managed_adapters.push_back(adapter);
            result.push_back(adapter);
        }
    }

    for (RGBControllerInterface* virt : rgb_controllers)
    {
        if (std::find(result.begin(), result.end(), virt) == result.end())
        {
            result.push_back(virt);
        }
    }

    return result;
}

nlohmann::json OpenRGBPluginAPI::GetLEDDescriptionJSON(led l)
{
    nlohmann::json j;
    j["name"] = l.name;
    j["value"] = l.value;
    return j;
}

nlohmann::json OpenRGBPluginAPI::GetMatrixMapDescriptionJSON(matrix_map_type m)
{
    nlohmann::json j;
    j["height"] = m.height;
    j["width"] = m.width;
    j["map"] = m.map;
    return j;
}

nlohmann::json OpenRGBPluginAPI::GetModeDescriptionJSON(mode m)
{
    nlohmann::json j;
    j["name"] = m.name;
    j["value"] = m.value;
    j["flags"] = m.flags;
    j["speed_min"] = m.speed_min;
    j["speed_max"] = m.speed_max;
    j["brightness_min"] = m.brightness_min;
    j["brightness_max"] = m.brightness_max;
    j["colors_min"] = m.colors_min;
    j["colors_max"] = m.colors_max;
    j["speed"] = m.speed;
    j["brightness"] = m.brightness;
    j["direction"] = m.direction;
    j["color_mode"] = m.color_mode;
    j["colors"] = m.colors;
    return j;
}

nlohmann::json OpenRGBPluginAPI::GetSegmentDescriptionJSON(segment s)
{
    nlohmann::json j;
    j["name"] = s.name;
    j["type"] = s.type;
    j["start_idx"] = s.start_idx;
    j["leds_count"] = s.leds_count;
    j["flags"] = s.flags;
    j["matrix_map"] = GetMatrixMapDescriptionJSON(s.matrix_map);
    return j;
}

nlohmann::json OpenRGBPluginAPI::GetZoneDescriptionJSON(zone z)
{
    nlohmann::json j;
    j["name"] = z.name;
    j["display_name"] = z.display_name;
    j["type"] = z.type;
    j["leds_count"] = z.leds_count;
    j["leds_min"] = z.leds_min;
    j["leds_max"] = z.leds_max;
    j["flags"] = z.flags;
    j["active_mode"] = z.active_mode;
    j["matrix_map"] = GetMatrixMapDescriptionJSON(z.matrix_map);
    for (size_t i = 0; i < z.segments.size(); i++)
    {
        j["segments"].push_back(GetSegmentDescriptionJSON(z.segments[i]));
    }
    for (size_t i = 0; i < z.modes.size(); i++)
    {
        j["modes"].push_back(GetModeDescriptionJSON(z.modes[i]));
    }
    return j;
}

nlohmann::json OpenRGBPluginAPI::GetDeviceDescriptionJSON(RGBControllerInterface* controller)
{
    nlohmann::json j;
    if (!controller) return j;
    j["name"] = controller->GetName();
    j["vendor"] = controller->GetVendor();
    j["description"] = controller->GetDescription();
    j["version"] = controller->GetVersion();
    j["serial"] = controller->GetSerial();
    j["location"] = controller->GetLocation();
    j["type"] = controller->GetDeviceType();
    j["flags"] = controller->GetFlags();
    j["active_mode"] = controller->GetActiveMode();
    unsigned int mc = controller->GetModeCount();
    for (unsigned int i = 0; i < mc; i++)
    {
        mode m;
        m.name = controller->GetModeName(i);
        m.flags = controller->GetModeFlags(i);
        m.speed_min = controller->GetModeSpeedMin(i);
        m.speed_max = controller->GetModeSpeedMax(i);
        m.brightness_min = controller->GetModeBrightnessMin(i);
        m.brightness_max = controller->GetModeBrightnessMax(i);
        m.colors_min = controller->GetModeColorsMin(i);
        m.colors_max = controller->GetModeColorsMax(i);
        m.speed = controller->GetModeSpeed(i);
        m.brightness = controller->GetModeBrightness(i);
        m.direction = controller->GetModeDirection(i);
        m.color_mode = controller->GetModeColorMode(i);
        unsigned int colors_count = controller->GetModeColorsCount(i);
        for (unsigned int c = 0; c < colors_count; c++)
        {
            m.colors.push_back(controller->GetModeColor(i, c));
        }
        j["modes"].push_back(GetModeDescriptionJSON(m));
    }
    unsigned int zc = controller->GetZoneCount();
    for (unsigned int i = 0; i < zc; i++)
    {
        j["zones"].push_back(GetZoneDescriptionJSON(controller->GetZone(i)));
    }
    unsigned int lc = controller->GetLEDCount();
    for (unsigned int i = 0; i < lc; i++)
    {
        led l;
        l.name = controller->GetLEDName(i);
        l.value = 0;
        j["leds"].push_back(GetLEDDescriptionJSON(l));
        j["colors"].push_back(controller->GetColor(i));
    }
    return j;
}

led OpenRGBPluginAPI::SetLEDDescriptionJSON(nlohmann::json j)
{
    led l;
    if (j.contains("name")) l.name = j["name"].get<std::string>();
    if (j.contains("value")) l.value = j["value"].get<unsigned int>();
    return l;
}

matrix_map_type OpenRGBPluginAPI::SetMatrixMapDescriptionJSON(nlohmann::json j)
{
    matrix_map_type m;
    if (j.contains("height")) m.height = j["height"].get<unsigned int>();
    if (j.contains("width")) m.width = j["width"].get<unsigned int>();
    if (j.contains("map") && j["map"].is_array()) m.map = j["map"].get<std::vector<unsigned int>>();
    return m;
}

mode OpenRGBPluginAPI::SetModeDescriptionJSON(nlohmann::json j)
{
    mode m;
    if (j.contains("name")) m.name = j["name"].get<std::string>();
    if (j.contains("value")) m.value = j["value"].get<int>();
    if (j.contains("flags")) m.flags = j["flags"].get<unsigned int>();
    if (j.contains("speed_min")) m.speed_min = j["speed_min"].get<unsigned int>();
    if (j.contains("speed_max")) m.speed_max = j["speed_max"].get<unsigned int>();
    if (j.contains("brightness_min")) m.brightness_min = j["brightness_min"].get<unsigned int>();
    if (j.contains("brightness_max")) m.brightness_max = j["brightness_max"].get<unsigned int>();
    if (j.contains("colors_min")) m.colors_min = j["colors_min"].get<unsigned int>();
    if (j.contains("colors_max")) m.colors_max = j["colors_max"].get<unsigned int>();
    if (j.contains("speed")) m.speed = j["speed"].get<unsigned int>();
    if (j.contains("brightness")) m.brightness = j["brightness"].get<unsigned int>();
    if (j.contains("direction")) m.direction = j["direction"].get<unsigned int>();
    if (j.contains("color_mode")) m.color_mode = j["color_mode"].get<unsigned int>();
    if (j.contains("colors") && j["colors"].is_array()) m.colors = j["colors"].get<std::vector<RGBColor>>();
    return m;
}

segment OpenRGBPluginAPI::SetSegmentDescriptionJSON(nlohmann::json j)
{
    segment s;
    if (j.contains("name")) s.name = j["name"].get<std::string>();
    if (j.contains("type")) s.type = j["type"].get<zone_type>();
    if (j.contains("start_idx")) s.start_idx = j["start_idx"].get<unsigned int>();
    if (j.contains("leds_count")) s.leds_count = j["leds_count"].get<unsigned int>();
    if (j.contains("flags")) s.flags = j["flags"].get<segment_flags>();
    if (j.contains("matrix_map")) s.matrix_map = SetMatrixMapDescriptionJSON(j["matrix_map"]);
    return s;
}

zone OpenRGBPluginAPI::SetZoneDescriptionJSON(nlohmann::json j)
{
    zone z;
    if (j.contains("name")) z.name = j["name"].get<std::string>();
    if (j.contains("display_name")) z.display_name = j["display_name"].get<std::string>();
    if (j.contains("type")) z.type = j["type"].get<zone_type>();
    if (j.contains("leds_count")) z.leds_count = j["leds_count"].get<unsigned int>();
    if (j.contains("leds_min")) z.leds_min = j["leds_min"].get<unsigned int>();
    if (j.contains("leds_max")) z.leds_max = j["leds_max"].get<unsigned int>();
    if (j.contains("flags")) z.flags = j["flags"].get<zone_flags>();
    if (j.contains("active_mode")) z.active_mode = j["active_mode"].get<int>();
    if (j.contains("matrix_map")) z.matrix_map = SetMatrixMapDescriptionJSON(j["matrix_map"]);
    if (j.contains("segments") && j["segments"].is_array())
    {
        for (auto& sj : j["segments"]) z.segments.push_back(SetSegmentDescriptionJSON(sj));
    }
    if (j.contains("modes") && j["modes"].is_array())
    {
        for (auto& mj : j["modes"]) z.modes.push_back(SetModeDescriptionJSON(mj));
    }
    return z;
}

RGBControllerInterface* OpenRGBPluginAPI::SetDeviceDescriptionJSON(nlohmann::json j)
{
    RGBController_Setup setup;
    if (j.contains("name")) setup.name = j["name"].get<std::string>();
    if (j.contains("vendor")) setup.vendor = j["vendor"].get<std::string>();
    if (j.contains("description")) setup.description = j["description"].get<std::string>();
    if (j.contains("version")) setup.version = j["version"].get<std::string>();
    if (j.contains("serial")) setup.serial = j["serial"].get<std::string>();
    if (j.contains("location")) setup.location = j["location"].get<std::string>();
    if (j.contains("type")) setup.type = j["type"].get<device_type>();
    if (j.contains("flags")) setup.flags = j["flags"].get<controller_flags>();
    if (j.contains("active_mode")) setup.active_mode = j["active_mode"].get<int>();
    if (j.contains("modes") && j["modes"].is_array())
    {
        for (auto& mj : j["modes"]) setup.modes.push_back(SetModeDescriptionJSON(mj));
    }
    if (j.contains("zones") && j["zones"].is_array())
    {
        for (auto& zj : j["zones"]) setup.zones.push_back(SetZoneDescriptionJSON(zj));
    }
    if (j.contains("leds") && j["leds"].is_array())
    {
        for (auto& lj : j["leds"]) setup.leds.push_back(SetLEDDescriptionJSON(lj));
    }
    return CreateVirtualRGBController(&setup);
}

bool OpenRGBPluginAPI::CompareControllers(RGBControllerInterface* c1, RGBControllerInterface* c2)
{
    if (!c1 || !c2) return false;
    return (c1->GetName() == c2->GetName() &&
            c1->GetLocation() == c2->GetLocation() &&
            c1->GetSerial() == c2->GetSerial());
}

std::string OpenRGBPluginAPI::DeviceTypeToString(device_type type)
{
    switch(type)
    {
        case DEVICE_TYPE_MOTHERBOARD:   return "Motherboard";
        case DEVICE_TYPE_DRAM:          return "DRAM";
        case DEVICE_TYPE_GPU:           return "GPU";
        case DEVICE_TYPE_COOLER:        return "Cooler";
        case DEVICE_TYPE_LEDSTRIP:      return "LED Strip";
        case DEVICE_TYPE_KEYBOARD:      return "Keyboard";
        case DEVICE_TYPE_MOUSE:         return "Mouse";
        case DEVICE_TYPE_MOUSEMAT:      return "Mousemat";
        case DEVICE_TYPE_HEADSET:       return "Headset";
        case DEVICE_TYPE_HEADSET_STAND: return "Headset Stand";
        case DEVICE_TYPE_GAMEPAD:       return "Gamepad";
        case DEVICE_TYPE_LIGHT:         return "Light";
        case DEVICE_TYPE_SPEAKER:       return "Speaker";
        case DEVICE_TYPE_VIRTUAL:       return "Virtual";
        case DEVICE_TYPE_STORAGE:       return "Storage";
        case DEVICE_TYPE_CASE:          return "Case";
        case DEVICE_TYPE_MICROPHONE:    return "Microphone";
        case DEVICE_TYPE_ACCESSORY:     return "Accessory";
        case DEVICE_TYPE_KEYPAD:        return "Keypad";
        case DEVICE_TYPE_LAPTOP:        return "Laptop";
        case DEVICE_TYPE_MONITOR:       return "Monitor";
        default:                        return "Unknown";
    }
}

bool OpenRGBPluginAPI::SetModeValuesFromMode(mode& dest, mode& src)
{
    dest.speed = src.speed;
    dest.brightness = src.brightness;
    dest.direction = src.direction;
    dest.color_mode = src.color_mode;
    dest.colors = src.colors;
    return true;
}

nlohmann::json OpenRGBPluginAPI::GetSettings(std::string settings_key)
{
    return MasterRGBPluginBridge::GetSettings(settings_key);
}

void OpenRGBPluginAPI::SaveSettings()
{
    MasterRGBPluginBridge::SaveSettings();
}

void OpenRGBPluginAPI::SetSettings(std::string settings_key, nlohmann::json new_settings)
{
    MasterRGBPluginBridge::SetSettings(settings_key, new_settings);
}
