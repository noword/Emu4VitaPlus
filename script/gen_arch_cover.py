from cairosvg import svg2png
from PIL import Image
from io import BytesIO
from cores import *
import os

WIDTH, HEIGHT = 285, 461
CONTROLLER_WIDTH = 176
SPACE = 10
SVGS = 'svgs'
ROTATE_270 = ('ARC', 'ATARI7800')


def load_svg(svg, width):
    out = BytesIO()
    svg2png(url=svg, write_to=out, output_width=width)
    return Image.open(out).convert('RGBA')


def gen(console_name):
    console = load_svg(f'{SVGS}/{console_name}/console.svg', WIDTH)
    console_w, console_h = console.size

    logo = load_svg(f'{SVGS}/{console_name}/logo.svg', WIDTH)
    logo_w, logo_h = logo.size

    ctrl_name = f'{SVGS}/{console_name}/controller.svg'
    if os.path.exists(ctrl_name):
        ctrl = load_svg(ctrl_name, CONTROLLER_WIDTH)
        ctrl_w, ctrl_h = ctrl.size
        if ctrl_w < ctrl_h:
            ctrl = ctrl.transpose(Image.ROTATE_270 if console_name in ROTATE_270 else Image.ROTATE_90)
            ctrl.thumbnail((CONTROLLER_WIDTH, CONTROLLER_WIDTH))
            ctrl_w, ctrl_h = ctrl.size
        top_h = console_h + ctrl_h + SPACE * 3
    else:
        ctrl = None
        ctrl_w = ctrl_h = 0
        top_h = console_h + SPACE * 2

    max_top_h = HEIGHT - SPACE - logo_h
    if top_h > max_top_h:
        ratio = max_top_h / top_h
        console_w = int(console_w * ratio)
        console_h = int(console_h * ratio)
        console = console.resize((console_w, console_h))
        if ctrl:
            ctrl_w = int(ctrl_w * ratio)
            ctrl_h = int(ctrl_h * ratio)
            ctrl = ctrl.resize((ctrl_w, ctrl_h))

    im = Image.new('RGBA', (WIDTH, HEIGHT))
    im.paste(logo, ((WIDTH - logo_w) // 2, HEIGHT - SPACE - logo_h), logo)
    if ctrl:
        y = (max_top_h - console_h - ctrl_h) // 3
        im.paste(console, ((WIDTH - console_w) // 2, y), console)
        im.paste(ctrl, ((WIDTH - ctrl_w) // 2, y * 2 + console_h), ctrl)
    else:
        im.paste(console, ((WIDTH - console_w) // 2, (max_top_h - console_h) // 2), console)

    # im.save(f'{console_name}.png')
    im.save(f'../arch/pkg/data/{console_name}/console.png')


for console in CORES.keys():
    print(console)
    gen(console)
