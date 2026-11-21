#include "RGBControllerAccess.h"
#include "RGBController.h"
#include "RGBController_Virtual.h"
#include <cstring>

namespace RGBControllerAccess
{
    std::string GetName(RGBController* c) { return c ? c->GetName() : ""; }
    std::string GetVendor(RGBController* c) { return c ? c->GetVendor() : ""; }
    std::string GetDescription(RGBController* c) { return c ? c->GetDescription() : ""; }
    std::string GetVersion(RGBController* c) { return c ? c->GetVersion() : ""; }
    std::string GetSerial(RGBController* c) { return c ? c->GetSerial() : ""; }
    std::string GetLocation(RGBController* c) { return c ? c->GetLocation() : ""; }
    int GetDeviceType(RGBController* c) { return c ? (int)c->type : 0; }
    unsigned int GetFlags(RGBController* c) { return c ? c->flags : 0; }

    unsigned int GetLEDCount(RGBController* c) { return c ? (unsigned int)c->leds.size() : 0; }
    std::string GetLEDName(RGBController* c, unsigned int led) { return c ? c->GetLEDName(led) : ""; }
    unsigned int GetLEDValue(RGBController* c, unsigned int led) { return (c && led < c->leds.size()) ? c->leds[led].value : 0; }
    unsigned int* GetColorsPointer(RGBController* c) { return (c && !c->colors.empty()) ? c->colors.data() : nullptr; }
    unsigned int GetColor(RGBController* c, unsigned int led) { return (c && led < c->colors.size()) ? c->colors[led] : 0; }
    void SetColor(RGBController* c, unsigned int led, unsigned int color) { if (c) c->SetLED(led, color); }
    void SetAllColors(RGBController* c, unsigned int color) { if (c) c->SetAllLEDs(color); }
    void SetAllZoneColors(RGBController* c, int zone, unsigned int color) { if (c) c->SetAllZoneLEDs(zone, color); }

    void UpdateLEDs(RGBController* c) { if (c) c->UpdateLEDs(); }
    void UpdateZoneLEDs(RGBController* c, int zone) { if (c) c->UpdateZoneLEDs(zone); }
    void UpdateSingleLED(RGBController* c, int led) { if (c) c->UpdateSingleLED(led); }
    void UpdateMode(RGBController* c) { if (c) c->UpdateMode(); }
    void SaveMode(RGBController* c) { if (c) c->SaveMode(); }

    unsigned int GetModeCount(RGBController* c) { return c ? (unsigned int)c->modes.size() : 0; }
    ModeAccessData GetMode(RGBController* c, unsigned int mode_idx)
    {
        ModeAccessData d;
        if (!c || mode_idx >= c->modes.size()) return d;
        const auto& m = c->modes[mode_idx];
        d.name = m.name;
        d.value = m.value;
        d.flags = m.flags;
        d.speed_min = m.speed_min;
        d.speed_max = m.speed_max;
        d.brightness_min = m.brightness_min;
        d.brightness_max = m.brightness_max;
        d.colors_min = m.colors_min;
        d.colors_max = m.colors_max;
        d.speed = m.speed;
        d.brightness = m.brightness;
        d.direction = m.direction;
        d.color_mode = m.color_mode;
        d.colors = m.colors;
        return d;
    }
    int GetActiveMode(RGBController* c) { return c ? c->GetMode() : 0; }
    void SetActiveMode(RGBController* c, int mode) { if (c) c->SetMode(mode); }
    void SetCustomMode(RGBController* c) { if (c) c->SetCustomMode(); }
    void SetMode(RGBController* c, unsigned int mode_idx, const ModeAccessData& d)
    {
        if (!c || mode_idx >= c->modes.size()) return;
        auto& m = c->modes[mode_idx];
        m.name = d.name;
        m.value = d.value;
        m.flags = d.flags;
        m.speed_min = d.speed_min;
        m.speed_max = d.speed_max;
        m.brightness_min = d.brightness_min;
        m.brightness_max = d.brightness_max;
        m.colors_min = d.colors_min;
        m.colors_max = d.colors_max;
        m.speed = d.speed;
        m.brightness = d.brightness;
        m.direction = d.direction;
        m.color_mode = d.color_mode;
        m.colors = d.colors;
    }

    unsigned int GetZoneCount(RGBController* c) { return c ? (unsigned int)c->zones.size() : 0; }
    ZoneAccessData GetZone(RGBController* c, unsigned int zone_idx)
    {
        ZoneAccessData d;
        if (!c || zone_idx >= c->zones.size()) return d;
        const auto& z = c->zones[zone_idx];
        d.name = z.name;
        d.type = (int)z.type;
        d.start_idx = z.start_idx;
        d.leds_count = z.leds_count;
        d.leds_min = z.leds_min;
        d.leds_max = z.leds_max;
        d.flags = z.flags;
        if (z.matrix_map != nullptr)
        {
            d.matrix_map_height = z.matrix_map->height;
            d.matrix_map_width = z.matrix_map->width;
            if (z.matrix_map->map != nullptr && z.matrix_map->height > 0 && z.matrix_map->width > 0)
            {
                size_t sz = (size_t)z.matrix_map->height * (size_t)z.matrix_map->width;
                d.matrix_map_data.assign(z.matrix_map->map, z.matrix_map->map + sz);
            }
        }
        else
        {
            d.matrix_map_height = 0;
            d.matrix_map_width = 0;
        }
        for (const auto& s : z.segments)
        {
            ZoneAccessData::SegmentAccessData sd;
            sd.name = s.name;
            sd.type = (int)s.type;
            sd.start_idx = s.start_idx;
            sd.leds_count = s.leds_count;
            d.segments.push_back(sd);
        }
        return d;
    }
    void ResizeZone(RGBController* c, int zone, int new_size) { if (c) c->ResizeZone(zone, new_size); }

    void RegisterUpdateCallback(RGBController* c, void (*cb)(void*, unsigned int, void*), void* arg)
    {
        if (c) c->RegisterUpdateCallback((RGBControllerCallback)cb, arg);
    }
    void UnregisterUpdateCallback(RGBController* c, void* arg)
    {
        if (c) c->UnregisterUpdateCallback(arg);
    }
    void ClearCallbacks(RGBController* c)
    {
        if (c) c->ClearCallbacks();
    }
    void SignalUpdate(RGBController* c, unsigned int /*reason*/)
    {
        if (c) c->SignalUpdate();
    }

    RGBController* CreateVirtualRGBController(const VirtualSetupData& setup)
    {
        RGBController_Virtual* v = new RGBController_Virtual();
        UpdateVirtualRGBController(v, setup);
        return v;
    }

    void UpdateVirtualRGBController(RGBController* ctrl, const VirtualSetupData& setup)
    {
        RGBController_Virtual* v = dynamic_cast<RGBController_Virtual*>(ctrl);
        if (!v) return;

        v->name = setup.name;
        v->description = setup.description;
        v->vendor = setup.vendor;
        v->location = setup.location;
        v->version = setup.version;
        v->serial = setup.serial;
        v->type = (device_type)setup.type;
        v->flags = setup.flags;
        v->active_mode = setup.active_mode;

        v->object_ptr = setup.object_ptr;
        v->DeviceUpdateLEDs_ptr = setup.DeviceUpdateLEDs;
        v->DeviceUpdateZoneLEDs_ptr = setup.DeviceUpdateZoneLEDs;
        v->DeviceUpdateSingleLED_ptr = setup.DeviceUpdateSingleLED;
        v->DeviceUpdateMode_ptr = setup.DeviceUpdateMode;
        v->DeviceConfigureZone_ptr = setup.DeviceConfigureZone;

        v->modes.clear();
        for (const auto& md : setup.modes)
        {
            mode m;
            m.name = md.name;
            m.value = md.value;
            m.flags = md.flags;
            m.speed_min = md.speed_min;
            m.speed_max = md.speed_max;
            m.brightness_min = md.brightness_min;
            m.brightness_max = md.brightness_max;
            m.colors_min = md.colors_min;
            m.colors_max = md.colors_max;
            m.speed = md.speed;
            m.brightness = md.brightness;
            m.direction = md.direction;
            m.color_mode = md.color_mode;
            m.colors = md.colors;
            v->modes.push_back(m);
        }

        v->zones.clear();
        for (const auto& zd : setup.zones)
        {
            zone z;
            z.name = zd.name;
            z.type = (zone_type)zd.type;
            z.start_idx = zd.start_idx;
            z.leds_count = zd.leds_count;
            z.leds_min = zd.leds_min;
            z.leds_max = zd.leds_max;
            z.flags = zd.flags;
            if (zd.matrix_map_height > 0 && zd.matrix_map_width > 0 && !zd.matrix_map_data.empty())
            {
                z.matrix_map = new matrix_map_type();
                z.matrix_map->height = zd.matrix_map_height;
                z.matrix_map->width = zd.matrix_map_width;
                size_t sz = (size_t)zd.matrix_map_height * (size_t)zd.matrix_map_width;
                z.matrix_map->map = new unsigned int[sz];
                std::memcpy(z.matrix_map->map, zd.matrix_map_data.data(), sz * sizeof(unsigned int));
            }
            else
            {
                z.matrix_map = nullptr;
            }

            for (const auto& sd : zd.segments)
            {
                segment s;
                s.name = sd.name;
                s.type = (zone_type)sd.type;
                s.start_idx = sd.start_idx;
                s.leds_count = sd.leds_count;
                z.segments.push_back(s);
            }
            v->zones.push_back(z);
        }

        v->leds.clear();
        for (const auto& ld : setup.leds)
        {
            led l;
            l.name = ld.name;
            l.value = ld.value;
            v->leds.push_back(l);
        }

        v->SetupColors();
    }
}
