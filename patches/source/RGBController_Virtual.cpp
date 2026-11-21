#include "RGBController_Virtual.h"

RGBController_Virtual::RGBController_Virtual()
{
}

RGBController_Virtual::~RGBController_Virtual()
{
    for (auto& z : zones)
    {
        if (z.matrix_map != nullptr)
        {
            if (z.matrix_map->map != nullptr)
            {
                delete[] z.matrix_map->map;
            }
            delete z.matrix_map;
            z.matrix_map = nullptr;
        }
    }
}

void RGBController_Virtual::DeviceUpdateLEDs()
{
    if (DeviceUpdateLEDs_ptr)
    {
        DeviceUpdateLEDs_ptr(object_ptr);
    }
}

void RGBController_Virtual::UpdateZoneLEDs(int zone)
{
    if (DeviceUpdateZoneLEDs_ptr)
    {
        DeviceUpdateZoneLEDs_ptr(object_ptr, zone);
    }
}

void RGBController_Virtual::UpdateSingleLED(int led)
{
    if (DeviceUpdateSingleLED_ptr)
    {
        DeviceUpdateSingleLED_ptr(object_ptr, led);
    }
}

void RGBController_Virtual::DeviceUpdateMode()
{
    if (DeviceUpdateMode_ptr)
    {
        DeviceUpdateMode_ptr(object_ptr);
    }
}
