#pragma once
#include <libretro.h>

class DiskControl
{
public:
    DiskControl(const retro_disk_control_ext_callback *callback);
    DiskControl(const retro_disk_control_callback *callback);
    virtual ~DiskControl();

private:
    retro_disk_control_ext_callback _callbacks;
};