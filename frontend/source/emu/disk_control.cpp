#include <string.h>
#include "disk_contorl.h"

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
}

DiskControl::~DiskControl()
{
}