"""Restore the UE 5.4 television material without changing the level."""

import unreal


MATERIAL_PATH = "/NocturneTelevision/Materials/M_NocturneTelevisionDistorted"
PARAMETERS = {
    "EmissiveIntensity": 2.5,
    "DistortionEnabled": 1.0,
    "DistortionAmount": 0.8,
    "ScanlineIntensity": 0.22,
    "ScanlineDensity": 280.0,
    "NoiseIntensity": 0.12,
    "HorizontalJitter": 0.012,
    "VerticalRollSpeed": 0.015,
    "ChromaticAberration": 0.004,
    "FlickerIntensity": 0.06,
    "VignetteIntensity": 0.45,
    "TrackingBarIntensity": 0.18,
}

SHADER_CODE = r"""
float strength = saturate(DistortionEnabled) * saturate(DistortionAmount);
float3 original = Original;
if (strength <= 0.0) return original * EmissiveIntensity;
float frame = floor(ElapsedTime * 30.0);
float3 color = float3(Red.r, Green.g, Blue.b);
float scanline = 0.5 + 0.5 * cos(UV.y * max(ScanlineDensity, 1.0) * 6.2831853);
color *= 1.0 - saturate(ScanlineIntensity) * scanline;
float noise = frac(sin(dot(floor(UV * float2(640.0, 480.0)), float2(12.9898, 78.233)) + frame * 19.19) * 43758.5453);
color += (noise - 0.5) * NoiseIntensity;
float flicker = 0.5 + 0.5 * sin(ElapsedTime * 47.0 + sin(ElapsedTime * 13.0));
color *= 1.0 - saturate(FlickerIntensity) * flicker;
float2 centered = UV * 2.0 - 1.0;
color *= 1.0 - saturate(VignetteIntensity) * saturate(dot(centered, centered) * 0.5);
float trackingDistance = abs(frac(UV.y - ElapsedTime * 0.17 + 0.5) - 0.5);
float trackingBand = 1.0 - smoothstep(0.015, 0.065, trackingDistance);
color *= 1.0 - saturate(TrackingBarIntensity) * trackingBand;
return lerp(original, max(color, 0.0), strength) * EmissiveIntensity;
"""


def restore_material():
    material = unreal.load_object(None, MATERIAL_PATH + ".M_NocturneTelevisionDistorted")
    if material is None:
        raise RuntimeError("The migrated television material is missing.")
    library = unreal.MaterialEditingLibrary
    library.delete_all_material_expressions(material)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("two_sided", True)
    shader = library.create_material_expression(material, unreal.MaterialExpressionCustom, 0, 0)
    shader.set_editor_property("code", SHADER_CODE)
    shader.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    shader.set_editor_property("description", "Animated analog television distortion")
    input_names = ["Original", "Red", "Green", "Blue", "UV", "ElapsedTime"] + list(PARAMETERS)
    inputs = []
    for name in input_names:
        custom_input = unreal.CustomInput()
        custom_input.set_editor_property("input_name", name)
        inputs.append(custom_input)
    shader.set_editor_property("inputs", inputs)
    uv = library.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -600, -150)
    time = library.create_material_expression(material, unreal.MaterialExpressionTime, -600, 0)
    sources = {"UV": uv, "ElapsedTime": time}
    for index, (name, value) in enumerate(PARAMETERS.items()):
        node = library.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -600, 150 + index * 80)
        node.set_editor_property("parameter_name", name)
        node.set_editor_property("default_value", value)
        sources[name] = node
    default_texture = unreal.load_object(None, "/Engine/EngineResources/DefaultTexture.DefaultTexture")
    for index, channel in enumerate(("Original", "Red", "Green", "Blue")):
        sample = library.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -300, -500 - index * 180)
        sample.set_editor_property("parameter_name", "VideoTexture")
        sample.set_editor_property("texture", default_texture)
        if channel == "Original":
            if not library.connect_material_expressions(uv, "", sample, ""):
                raise RuntimeError("Could not connect original video coordinates.")
        else:
            coordinate = library.create_material_expression(material, unreal.MaterialExpressionCustom, -900, -500 - index * 180)
            coordinate.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT2)
            coordinate.set_editor_property("description", f"{channel} video sampling coordinates")
            coordinate_names = ["UV", "ElapsedTime", "DistortionEnabled", "DistortionAmount", "ScanlineDensity", "HorizontalJitter", "VerticalRollSpeed", "ChromaticAberration"]
            coordinate_inputs = []
            for name in coordinate_names:
                custom_input = unreal.CustomInput()
                custom_input.set_editor_property("input_name", name)
                coordinate_inputs.append(custom_input)
            coordinate.set_editor_property("inputs", coordinate_inputs)
            offset_sign = {"Red": "1.0", "Green": "0.0", "Blue": "-1.0"}[channel]
            coordinate.set_editor_property("code", """float strength = saturate(DistortionEnabled) * saturate(DistortionAmount);
float frame = floor(ElapsedTime * 30.0);
float row = floor(UV.y * max(ScanlineDensity, 1.0));
float jitter = frac(sin(row * 12.9898 + frame * 78.233) * 43758.5453) * 2.0 - 1.0;
float2 warped = UV;
warped.x = saturate(UV.x + strength * (jitter * HorizontalJitter + OFFSET * ChromaticAberration));
warped.y = frac(UV.y + strength * ElapsedTime * VerticalRollSpeed);
return strength > 0.0 ? warped : UV;
""".replace("OFFSET", offset_sign))
            for name in coordinate_names:
                if not library.connect_material_expressions(sources[name], "", coordinate, name):
                    raise RuntimeError(f"Could not connect coordinate input: {name}")
            if not library.connect_material_expressions(coordinate, "", sample, ""):
                raise RuntimeError("Could not connect native video sampling coordinates.")
        if not library.connect_material_expressions(sample, "RGB", shader, channel):
            raise RuntimeError(f"Could not connect native video sample: {channel}")
    for name, node in sources.items():
        if not library.connect_material_expressions(node, "", shader, name):
            raise RuntimeError(f"Could not connect shader input: {name}")
    if not library.connect_material_property(shader, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR):
        raise RuntimeError("Could not connect television emissive output.")
    library.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError("Could not save the restored television material.")
    unreal.log("NOCTURNE_DISTORTION_RESTORED: all television parameters are connected.")


restore_material()
