#include <string.h>
#include "disk_control.h"
#include "log.h"

DiskControl::DiskControl(const retro_disk_control_ext_callback *callback)
{
    memcpy(&_callbacks, callback, sizeof(retro_disk_control_ext_callback));
}

DiskControl::DiskControl(const retro_disk_control_callback *callback)
{
    _callbacks.set_eject_state = callback->set_eject_state;
    _callbacks.get_eject_state = callback->get_eject_state;
    _callbacks.get_image_index = callback->get_image_index;
    _callbacks.set_image_index = callback->set_image_index;
    _callbacks.get_num_images = callback->get_num_images;
    _callbacks.replace_image_index = callback->replace_image_index;
    _callbacks.add_image_index = callback->add_image_index;
    _callbacks.set_initial_image = nullptr;
    _callbacks.get_image_path = nullptr;
    _callbacks.get_image_label = nullptr;
}

DiskControl::~DiskControl()
{
}

bool DiskControl::ReplaceImageIndex(unsigned index)
{
    if (index >= GetNumImages())
    {
        return false;
    }

    _callbacks.set_eject_state(true);
    _callbacks.set_image_index(index);
    return _callbacks.set_eject_state(false);
}

std::string DiskControl::GetImagePath(size_t index)
{
    LogFunctionName;
    char path[256];
    if (_callbacks.get_image_path && _callbacks.get_image_path(index, path, 256))
    {
        return path;
    }
    else
    {
        return "";
    }
}