"""Isolate the television material output without saving any changes."""
import unreal

library = unreal.MaterialEditingLibrary
material = unreal.load_object(None, "/NocturneTelevision/Materials/M_NocturneTelevisionDistorted.M_NocturneTelevisionDistorted")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
target = unreal.RenderingLibrary.create_render_target2d(world, 16, 16, unreal.TextureRenderTargetFormat.RTF_RGBA16F)
white = unreal.load_object(None, "/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")
root = library.get_material_property_input_node(material, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
pending = [root]
visited = set()
sample = None
while pending:
    expression = pending.pop()
    if expression is None or expression.get_path_name() in visited:
        continue
    visited.add(expression.get_path_name())
    if isinstance(expression, unreal.MaterialExpressionTextureSampleParameter2D):
        sample = expression
    pending.extend(library.get_inputs_for_material_expression(material, expression))


def draw(label):
    dynamic = unreal.MaterialLibrary.create_dynamic_material_instance(world, material)
    dynamic.set_texture_parameter_value("VideoTexture", white)
    dynamic.set_scalar_parameter_value("EmissiveIntensity", 1.0)
    dynamic.set_scalar_parameter_value("DistortionEnabled", 0.0)
    unreal.log(f"{label} binding: {dynamic.get_texture_parameter_value('VideoTexture')}, emissive: {dynamic.get_scalar_parameter_value('EmissiveIntensity')}")
    unreal.RenderingLibrary.draw_material_to_render_target(world, target, dynamic)
    pixel = unreal.RenderingLibrary.read_render_target_raw_pixel(world, target, 8, 8, False)
    unreal.log(f"DIAGNOSTIC {label}: {pixel}")


draw("Full graph")
if sample is None:
    raise RuntimeError("No native texture sample was found.")
library.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
library.recompile_material(material)
draw("Texture sample only")
constant = library.create_material_expression(material, unreal.MaterialExpressionConstant3Vector)
constant.set_editor_property("constant", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
library.connect_material_property(constant, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
library.recompile_material(material)
draw("Constant white only")
