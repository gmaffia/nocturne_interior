"""Render the television shader and verify its global bypass."""
import unreal

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
material = unreal.load_object(None, "/NocturneTelevision/Materials/M_NocturneTVAnalog54.M_NocturneTVAnalog54")
root = unreal.MaterialEditingLibrary.get_material_property_input_node(material, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
if root is None:
    raise RuntimeError("Candidate material has no emissive connection.")
unreal.log(f"Emissive root: {root}")
unreal.log(f"Shader inputs: {unreal.MaterialEditingLibrary.get_inputs_for_material_expression(material, root)}")
for name in ("EmissiveIntensity", "DistortionAmount", "DistortionEnabled"):
    unreal.log(f"Scalar default {name}: {unreal.MaterialEditingLibrary.get_material_default_scalar_parameter_value(material, name)}")
dynamic = unreal.MaterialLibrary.create_dynamic_material_instance(world, material)
dynamic.set_texture_parameter_value("VideoTexture", unreal.load_object(None, "/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"))
target = unreal.RenderingLibrary.create_render_target2d(world, 64, 64, unreal.TextureRenderTargetFormat.RTF_RGBA16F)
unreal.RenderingLibrary.draw_material_to_render_target(world, target, material)
dynamic.set_scalar_parameter_value("EmissiveIntensity", 1.0)
for name in ("blend_mode", "material_domain", "shading_model", "use_material_attributes"):
    unreal.log(f"Material setting {name}: {material.get_editor_property(name)}")
unreal.log(f"Texture parameters: {unreal.MaterialEditingLibrary.get_texture_parameter_names(material)}")
unreal.log(f"Dynamic texture: {dynamic.get_texture_parameter_value('VideoTexture')}")
unreal.log(f"Dynamic emissive: {dynamic.get_scalar_parameter_value('EmissiveIntensity')}")
control_material = unreal.load_object(None, "/NocturneTelevision/Materials/M_NocturneTelevisionScreen.M_NocturneTelevisionScreen")
control = unreal.MaterialLibrary.create_dynamic_material_instance(world, control_material)
control.set_texture_parameter_value("VideoTexture", unreal.load_object(None, "/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"))
control.set_scalar_parameter_value("EmissiveIntensity", 1.0)
unreal.RenderingLibrary.draw_material_to_render_target(world, target, control)
control_pixel = unreal.RenderingLibrary.read_render_target_raw_pixel(world, target, 32, 32, False)
unreal.log(f"Native control material render: {control_pixel}")


def render(enabled, amount):
    dynamic.set_scalar_parameter_value("DistortionEnabled", enabled)
    dynamic.set_scalar_parameter_value("DistortionAmount", amount)
    unreal.RenderingLibrary.draw_material_to_render_target(world, target, dynamic)
    pixels = [unreal.RenderingLibrary.read_render_target_raw_pixel(world, target, x, y, False) for y in (8, 24, 40, 56) for x in (8, 24, 40, 56)]
    return [(pixel.r, pixel.g, pixel.b) for pixel in pixels]


disabled = render(0.0, 1.0)
zero_amount = render(1.0, 0.0)
enabled = render(1.0, 1.0)
if len(disabled) != 16:
    raise RuntimeError(f"Unexpected render sample count: {len(disabled)}")


def difference(first, second):
    return sum(sum(abs(x - y) for x, y in zip(a, b)) for a, b in zip(first, second)) / len(first)


bypass_error = difference(disabled, zero_amount)
effect_difference = difference(disabled, enabled)
unreal.log(f"Render samples: disabled={disabled[5]}, enabled={enabled[5]}")
if bypass_error > 0.0001 or effect_difference < 0.001:
    raise RuntimeError(f"Distortion validation failed: bypass={bypass_error}, effect={effect_difference}")
unreal.log(f"NOCTURNE_DISTORTION_VALIDATED: bypass error={bypass_error}, rendered effect difference={effect_difference}")
