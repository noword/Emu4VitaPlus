#pragma once
#include <libretro.h>
#include <string>

class DiskControl
{
public:
    DiskControl(const retro_disk_control_ext_callback *callback);
    DiskControl(const retro_disk_control_callback *callback);
    virtual ~DiskControl();

    bool GetEjectState() { return _callbacks.get_eject_state(); };
    unsigned GetNumImages() { return _callbacks.get_num_images(); };
    unsigned GetImageIndex() { return _callbacks.get_image_index(); };
    bool ReplaceImageIndex(unsigned index);
    std::string GetImagePath(size_t index);

private:
    retro_disk_control_ext_callback _callbacks;
};