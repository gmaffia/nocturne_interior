"""Build the analog television entirely with native UE 5.4 material nodes."""
import unreal

PATH = "/NocturneTelevision/Materials/M_NocturneTVAnalog54.M_NocturneTVAnalog54"
material = unreal.load_object(None, PATH)
if material is not None:
    raise RuntimeError("Candidate already exists; do not overwrite a validated material.")
working = unreal.load_object(None, "/NocturneTelevision/Materials/M_NocturneTelevisionScreen.M_NocturneTelevisionScreen")
material = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset("M_NocturneTVAnalog54", "/NocturneTelevision/Materials", working)
if material is None:
    raise RuntimeError("Could not duplicate the working television material.")
library = unreal.MaterialEditingLibrary
material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
material.set_editor_property("two_sided", True)
node_count = 0


def node(kind, **properties):
    global node_count
    result = library.create_material_expression(material, getattr(unreal, "MaterialExpression" + kind), -1800 + (node_count % 8) * 220, (node_count // 8) * 180)
    node_count += 1
    for key, value in properties.items():
        result.set_editor_property(key, value)
    return result


def connect(source, target, input_name="", output_name=""):
    if not library.connect_material_expressions(source, output_name, target, input_name):
        raise RuntimeError(f"Could not connect {source.get_name()} to {target.get_name()}:{input_name}")


def constant(value):
    return node("Constant", r=value)


def binary(kind, first, second):
    result = node(kind)
    connect(first, result, "A")
    connect(second, result, "B")
    return result


def unary(kind, source):
    result = node(kind)
    connect(source, result)
    return result


def scale(source, value):
    return binary("Multiply", source, constant(value))


def add(first, second):
    return binary("Add", first, second)


def multiply(first, second):
    return binary("Multiply", first, second)


def mask(source, channel):
    result = node("ComponentMask", r=channel == "r", g=channel == "g", b=channel == "b", a=False)
    connect(source, result)
    return result


def attenuation(value):
    return unary("OneMinus", unary("Saturate", value))


defaults = {
    "EmissiveIntensity": 2.5, "DistortionEnabled": 1.0, "DistortionAmount": 0.8,
    "ScanlineIntensity": 0.22, "ScanlineDensity": 280.0, "NoiseIntensity": 0.12,
    "HorizontalJitter": 0.012, "VerticalRollSpeed": 0.015,
    "ChromaticAberration": 0.004, "FlickerIntensity": 0.06,
    "VignetteIntensity": 0.45, "TrackingBarIntensity": 0.18,
}
parameters = {name: node("ScalarParameter", parameter_name=name, default_value=value) for name, value in defaults.items()}
strength = multiply(unary("Saturate", parameters["DistortionEnabled"]), unary("Saturate", parameters["DistortionAmount"]))
uv = node("TextureCoordinate")
time = node("Time")
x = mask(uv, "r")
y = mask(uv, "g")
row = multiply(y, parameters["ScanlineDensity"])
jitter = multiply(multiply(unary("Sine", add(row, scale(time, 30.0))), parameters["HorizontalJitter"]), strength)
roll = multiply(multiply(time, parameters["VerticalRollSpeed"]), strength)
shift = binary("AppendVector", jitter, roll)
warped = unary("Frac", add(uv, shift))
offset = binary("AppendVector", multiply(parameters["ChromaticAberration"], strength), constant(0.0))
default_texture = unreal.load_object(None, "/Engine/EngineResources/DefaultTexture.DefaultTexture")


def sample(coordinates):
    result = node("TextureSampleParameter2D", parameter_name="VideoTexture", texture=default_texture)
    connect(coordinates, result)
    return result


original = sample(uv)
red = mask(sample(add(warped, offset)), "r")
green = mask(sample(warped), "g")
blue = mask(sample(binary("Subtract", warped, offset)), "b")
color = binary("AppendVector", binary("AppendVector", red, green), blue)
scanline = add(scale(unary("Sine", row), 0.5), constant(0.5))
color = multiply(color, attenuation(multiply(scanline, parameters["ScanlineIntensity"])))
noise_phase = add(add(scale(x, 12.9898), scale(y, 78.233)), unary("Floor", scale(time, 30.0)))
noise = binary("Subtract", unary("Frac", scale(unary("Sine", noise_phase), 43758.5453)), constant(0.5))
color = add(color, multiply(noise, parameters["NoiseIntensity"]))
flicker = add(scale(unary("Sine", scale(time, 7.5)), 0.5), constant(0.5))
color = multiply(color, attenuation(multiply(flicker, parameters["FlickerIntensity"])))
center_x = binary("Subtract", x, constant(0.5))
center_y = binary("Subtract", y, constant(0.5))
radial = unary("Saturate", scale(add(multiply(center_x, center_x), multiply(center_y, center_y)), 2.0))
color = multiply(color, attenuation(multiply(radial, parameters["VignetteIntensity"])))
tracking_phase = unary("Frac", add(binary("Subtract", y, scale(time, 0.17)), constant(0.5)))
tracking_distance = unary("Abs", binary("Subtract", tracking_phase, constant(0.5)))
tracking_band = attenuation(scale(tracking_distance, 20.0))
color = multiply(color, attenuation(multiply(tracking_band, parameters["TrackingBarIntensity"])))
color = binary("Max", color, constant(0.0))
blend = node("LinearInterpolate")
connect(original, blend, "A", "RGB")
connect(color, blend, "B")
connect(strength, blend, "Alpha")
emissive = multiply(blend, parameters["EmissiveIntensity"])
if not library.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR):
    raise RuntimeError("Could not connect the native television emissive output.")
front = library.get_material_property_input_node(material, unreal.MaterialProperty.MP_FRONT_MATERIAL)
if front is None:
    raise RuntimeError("The working material has no Substrate front material.")
input_names = library.get_material_expression_input_names(front)
emissive_input = next((name for name in input_names if name.lower().startswith("emissive")), None)
if emissive_input is None:
    raise RuntimeError(f"Substrate emissive input is missing: {input_names}")
connect(emissive, front, emissive_input)
unreal.log(f"Connected television distortion to Substrate input: {emissive_input}")
library.recompile_material(material)
unreal.log(f"Native emissive root before save: {library.get_material_property_input_node(material, unreal.MaterialProperty.MP_EMISSIVE_COLOR)}")
from pathlib import Path
validation = Path(__file__).with_name("ValidateTelevisionDistortion.py")
exec(compile(validation.read_text(encoding="utf-8"), str(validation), "exec"), {"__file__": str(validation)})
if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
    raise RuntimeError("Could not save native television material.")
unreal.log(f"NOCTURNE_NATIVE_GRAPH_RESTORED: {node_count} native nodes, all effects connected.")
