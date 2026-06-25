from cairosvg import svg2png
from PIL import Image
from io import BytesIO
from cores import *

WIDTH, HEIGHT = 285, 461
BOTTOM = 10


def gen(console_name):
    out = BytesIO()
    svg2png(url=f'svgs/{console_name}/console.svg', write_to=out, output_width=WIDTH)
    console = Image.open(out).convert('RGBA')

    out = BytesIO()
    svg2png(url=f'svgs/{console_name}/logo.svg', write_to=out, output_width=WIDTH)
    logo = Image.open(out).convert('RGBA')

    w, h = console.size
    logo_w, logo_h = logo.size
    max_h = HEIGHT - BOTTOM - logo_h
    if h > max_h:
        console = console.resize((w * max_h // h, max_h))
        w, h = console.size

    im = Image.new('RGBA', (WIDTH, HEIGHT))
    im.paste(console, ((WIDTH - w) // 2, (HEIGHT - BOTTOM - logo_h - h) // 2), console)
    im.paste(logo, ((WIDTH - logo_w) // 2, HEIGHT - BOTTOM - logo_h), logo)

    im.save(f'{console_name}.png')


for console in CORES.keys():
    print(console)
    gen(console)
