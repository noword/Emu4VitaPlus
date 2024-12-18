from gimpfu import *
from cores import CORES


def apply_filters():
    for console in CORES.keys():
        img_path = console + '.png'
        img = pdb.gimp_file_load(img_path, img_path)
        drawable = img.active_layer

        pdb.plug_in_cartoon(img, drawable, 7, 0.2)
        pdb.plug_in_softglow(img, drawable, 10, 0.75, 0.55)

        pdb.gimp_file_save(img, drawable, img_path, img_path)
        pdb.gimp_image_delete(img)
