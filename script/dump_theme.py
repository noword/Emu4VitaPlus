import toml
import re
import json

# Download from https://github.com/Patitotective/ImThemes/blob/main/themes.toml

THEMES_TOML = 'themes.toml'
OUTPUT_JSON = 'themes.json'
STYLE_MAP = {  # The version of imgui-vita2d is relatively old and has fewer style attributes.
    "alpha": "Alpha",
    "windowPadding": "WindowPadding",
    "windowRounding": "WindowRounding",
    "windowBorderSize": "WindowBorderSize",
    "windowMinSize": "WindowMinSize",
    "windowTitleAlign": "WindowTitleAlign",
    "windowMenuButtonPosition": "WindowMenuButtonPosition",
    "childRounding": "ChildRounding",
    "childBorderSize": "ChildBorderSize",
    "popupRounding": "PopupRounding",
    "popupBorderSize": "PopupBorderSize",
    "framePadding": "FramePadding",
    "frameRounding": "FrameRounding",
    "frameBorderSize": "FrameBorderSize",
    "itemSpacing": "ItemSpacing",
    "itemInnerSpacing": "ItemInnerSpacing",
    "indentSpacing": "IndentSpacing",
    "columnsMinSpacing": "ColumnsMinSpacing",
    "scrollbarSize": "ScrollbarSize",
    "scrollbarRounding": "ScrollbarRounding",
    "grabMinSize": "GrabMinSize",
    "grabRounding": "GrabRounding",
    "tabRounding": "TabRounding",
    "tabBorderSize": "TabBorderSize",
    "tabMinWidthForCloseButton": "TabMinWidthForUnselectedCloseButton",
    "colorButtonPosition": "ColorButtonPosition",
    "buttonTextAlign": "ButtonTextAlign",
    "selectableTextAlign": "SelectableTextAlign",
}

COLORS = (
    "Text",
    "TextDisabled",
    "WindowBg",
    "ChildBg",
    "PopupBg",
    "Border",
    "BorderShadow",
    "FrameBg",
    "FrameBgHovered",
    "FrameBgActive",
    "TitleBg",
    "TitleBgActive",
    "TitleBgCollapsed",
    "MenuBarBg",
    "ScrollbarBg",
    "ScrollbarGrab",
    "ScrollbarGrabHovered",
    "ScrollbarGrabActive",
    "CheckMark",
    "SliderGrab",
    "SliderGrabActive",
    "Button",
    "ButtonHovered",
    "ButtonActive",
    "Header",
    "HeaderHovered",
    "HeaderActive",
    "Separator",
    "SeparatorHovered",
    "SeparatorActive",
    "ResizeGrip",
    "ResizeGripHovered",
    "ResizeGripActive",
    "Tab",
    "TabHovered",
    "TabActive",
    "TabUnfocused",
    "TabUnfocusedActive",
    "PlotLines",
    "PlotLinesHovered",
    "PlotHistogram",
    "PlotHistogramHovered",
    "TextSelectedBg",
    "DragDropTarget",
    "NavHighlight",
    "NavWindowingHighlight",
    "NavWindowingDimBg",
    "ModalWindowDimBg",
)


def rgba2floats(s):
    m = re.fullmatch(r'rgba\s*\(\s*([^,]+)\s*,\s*([^,]+)\s*,\s*([^,]+)\s*,\s*([^,\)]+)\s*\)', s)

    if not m:
        raise ValueError(f"Invalid rgba string: {s}")

    values = [x.strip() for x in m.groups()]

    nums = [float(x) for x in values]

    if all(re.fullmatch(r'[0-9]+', v) for v in values[:3]):
        # rgb are int
        nums[:3] = [v / 255.0 for v in nums[:3]]

    nums = [round(v, 3) for v in nums]
    return nums


data = toml.load(open(THEMES_TOML))['themes']

new_data = []
for d in data:
    styles = {}
    for k, v in STYLE_MAP.items():
        styles[v] = d['style'][k]

    colors = {}
    for c in COLORS:
        colors[c] = rgba2floats(d['style']['colors'][c])

    new_data.append({'name': d['name'], 'styles': styles, 'colors': colors})

json.dump(new_data, open(OUTPUT_JSON, 'w'), indent=2)
