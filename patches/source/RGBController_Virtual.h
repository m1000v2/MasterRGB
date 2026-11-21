#pragma once
#include "RGBController.h"

class RGBController_Virtual : public RGBController
{
public:
    RGBController_Virtual();
    virtual ~RGBController_Virtual();

    void SetupZones() override {}
    void ResizeZone(int /*zone*/, int /*new_size*/) override {}

    void DeviceUpdateLEDs() override;
    void UpdateZoneLEDs(int zone) override;
    void UpdateSingleLED(int led) override;
    void DeviceUpdateMode() override;

    void* object_ptr = nullptr;
    void (*DeviceUpdateLEDs_ptr)(void*) = nullptr;
    void (*DeviceUpdateZoneLEDs_ptr)(void*, int) = nullptr;
    void (*DeviceUpdateSingleLED_ptr)(void*, int) = nullptr;
    void (*DeviceUpdateMode_ptr)(void*) = nullptr;
    void (*DeviceConfigureZone_ptr)(void*, int) = nullptr;
};
