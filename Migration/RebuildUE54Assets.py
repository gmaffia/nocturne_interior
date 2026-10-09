import os

import unreal


TARGET_MAP = "/Game/StylizedKitchen/Levels/NocturneInterior54"
FINAL_MAP = "/Game/StylizedKitchen/Levels/NocturneInterior"
VIDEO_MATERIAL_PATH = "/NocturneTelevision/Materials/M_NocturneTelevisionDistorted"
SIMPLE_VIDEO_MATERIAL_PATH = "/NocturneTelevision/Materials/M_NocturneTelevisionScreen"
VIDEO_SOURCE_PATH = "/Game/Movies/LivingroomTelevision"
SUBJECT_MATERIAL_PATH = "/Game/Nocturne/Materials/M_SubjectVoidBlack"
NIGHT_SKY_MATERIAL_PATH = "/Game/NocturneLook/Materials/MI_Nocturne_NightSky"


def create_asset(asset_path, asset_class, factory):
    package_path, asset_name = asset_path.rsplit("/", 1)
    existing_asset = unreal.load_object(None, f"{asset_path}.{asset_name}")
    if existing_asset is not None:
        return existing_asset

    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name,
        package_path,
        asset_class,
        factory,
    )
    if asset is None:
        raise RuntimeError(f"Could not create asset: {asset_path}")
    return asset


def create_scalar_parameter(material, name, value, x, y):
    expression = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionScalarParameter,
        x,
        y,
    )
    expression.set_editor_property("parameter_name", name)
    expression.set_editor_property("default_value", value)
    return expression


def build_video_material(asset_path):
    material = create_asset(asset_path, unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("two_sided", True)

    video_texture = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionTextureSampleParameter2D,
        -700,
        -100,
    )
    video_texture.set_editor_property("parameter_name", "VideoTexture")
    video_texture.set_editor_property(
        "texture", unreal.EditorAssetLibrary.load_asset("/Engine/EngineResources/DefaultTexture")
    )

    emissive_intensity = create_scalar_parameter(
        material, "EmissiveIntensity", 2.5, -700, 100
    )
    emissive_multiply = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionMultiply,
        -350,
        -50,
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(
        video_texture, "RGB", emissive_multiply, "A"
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(
        emissive_intensity, "", emissive_multiply, "B"
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        emissive_multiply,
        "",
        unreal.MaterialProperty.MP_EMISSIVE_COLOR,
    )

    auxiliary_parameters = (
        ("DistortionEnabled", 1.0),
        ("DistortionAmount", 0.8),
        ("ScanlineIntensity", 0.22),
        ("ScanlineDensity", 280.0),
        ("NoiseIntensity", 0.12),
        ("HorizontalJitter", 0.012),
        ("VerticalRollSpeed", 0.015),
        ("ChromaticAberration", 0.004),
        ("FlickerIntensity", 0.06),
        ("VignetteIntensity", 0.45),
        ("TrackingBarIntensity", 0.18),
    )
    for index, (name, value) in enumerate(auxiliary_parameters):
        create_scalar_parameter(material, name, value, -1000, 250 + index * 75)

    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    return material


def build_unlit_color_material(asset_path, color, skeletal_mesh=False):
    material = create_asset(asset_path, unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("two_sided", True)
    if skeletal_mesh:
        material.set_editor_property("used_with_skeletal_mesh", True)

    color_expression = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant3Vector,
        -300,
        0,
    )
    color_expression.set_editor_property("constant", color)
    unreal.MaterialEditingLibrary.connect_material_property(
        color_expression,
        "",
        unreal.MaterialProperty.MP_EMISSIVE_COLOR,
    )
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    return material


def rebuild_assets():
    video_material = build_video_material(VIDEO_MATERIAL_PATH)
    build_video_material(SIMPLE_VIDEO_MATERIAL_PATH)
    build_unlit_color_material(
        SUBJECT_MATERIAL_PATH,
        unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
        skeletal_mesh=True,
    )
    night_sky_material = build_unlit_color_material(
        NIGHT_SKY_MATERIAL_PATH,
        unreal.LinearColor(0.001, 0.002, 0.006, 1.0),
    )

    video_source = create_asset(
        VIDEO_SOURCE_PATH,
        unreal.FileMediaSource,
        unreal.FileMediaSourceFactoryNew(),
    )
    video_file = os.path.join(
        unreal.Paths.project_content_dir(), "Movies", "betty_boop_snow_white.mp4"
    )
    video_source.set_file_path(video_file)
    unreal.EditorAssetLibrary.save_loaded_asset(video_source, only_if_is_dirty=False)

    world = unreal.EditorLoadingAndSavingUtils.load_map(TARGET_MAP)
    if world is None:
        raise RuntimeError(f"Could not load migrated map: {TARGET_MAP}")

    television_count = 0
    sky_count = 0
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        class_name = actor.get_class().get_name()
        if class_name == "NocturneTelevisionScreen":
            actor.set_editor_property("video_source", video_source)
            actor.set_editor_property("screen_material", video_material)
            television_count += 1
        elif actor.get_actor_label() == "SkySphere":
            mesh_component = actor.get_component_by_class(unreal.StaticMeshComponent)
            if mesh_component is not None:
                mesh_component.set_material(0, night_sky_material)
                sky_count += 1

    if television_count == 0:
        raise RuntimeError("No television actor was found in the migrated map.")
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError(f"Could not save repaired map: {TARGET_MAP}")

    if unreal.EditorAssetLibrary.does_asset_exist(FINAL_MAP):
        raise RuntimeError(f"Final map path is already occupied: {FINAL_MAP}")
    if not unreal.EditorAssetLibrary.rename_asset(TARGET_MAP, FINAL_MAP):
        raise RuntimeError(f"Could not finalize migrated map as: {FINAL_MAP}")

    unreal.log(
        f"Rebuilt UE 5.4 assets and repaired {television_count} television actor(s) "
        f"and {sky_count} sky actor(s). Final map: {FINAL_MAP}"
    )


rebuild_assets()
