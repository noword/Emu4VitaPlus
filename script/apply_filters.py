from pathlib import Path
import gi

gi.require_version("Gimp", "3.0")

from gi.repository import Gimp
from gi.repository import Gio


def process_image(img_path):
    file = Gio.File.new_for_path(img_path)

    image = Gimp.file_load(Gimp.RunMode.NONINTERACTIVE, file)

    drawable = image.get_layers()[0]

    #
    # Cartoon
    #
    cartoon = Gimp.DrawableFilter.new(drawable, "gegl:cartoon", "cartoon")

    cfg = cartoon.get_config()

    cfg.set_property("mask-radius", 7.0)
    cfg.set_property("pct-black", 0.2)

    drawable.append_filter(cartoon)

    #
    # Softglow
    #
    softglow = Gimp.DrawableFilter.new(drawable, "gegl:softglow", "softglow")

    cfg = softglow.get_config()

    cfg.set_property("glow-radius", 10.0)
    cfg.set_property("brightness", 0.3)
    cfg.set_property("sharpness", 0.85)

    drawable.append_filter(softglow)

    drawable.merge_filters()

    Gimp.file_save(Gimp.RunMode.NONINTERACTIVE, image, file, None)

    image.delete()


def apply_filters():
    for console in CORES.keys():
        img_path = console + '.png'
        img_path = str(Path(img_path).resolve())
        print(img_path)
        process_image(img_path)
        break


apply_filters()
