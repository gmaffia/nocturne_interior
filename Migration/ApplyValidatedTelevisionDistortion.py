"""Repair the Substrate output and restore television distortion in the main map."""
from pathlib import Path
import unreal

material_path = "/NocturneTelevision/Materials/M_NocturneTelevisionDistorted.M_NocturneTelevisionDistorted"
material = unreal.load_object(None, material_path)
if material is None:
    raise RuntimeError("The television distortion material is missing.")
library = unreal.MaterialEditingLibrary
emissive = library.get_material_property_input_node(material, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
front = library.get_material_property_input_node(material, unreal.MaterialProperty.MP_FRONT_MATERIAL)
if emissive is None or front is None:
    raise RuntimeError("The television material is missing its emissive or Substrate output.")
input_name = next(name for name in library.get_material_expression_input_names(front) if name.lower().startswith("emissive"))
if not library.connect_material_expressions(emissive, "", front, input_name):
    raise RuntimeError("Could not connect distortion to the Substrate emissive input.")
library.recompile_material(material)
validation_path = Path(__file__).with_name("ValidateTelevisionDistortion.py")
validation_source = validation_path.read_text(encoding="utf-8").replace(
    "/NocturneTelevision/Materials/M_NocturneTVAnalog54.M_NocturneTVAnalog54", material_path)
exec(compile(validation_source, str(validation_path), "exec"), {"__file__": str(validation_path)})
if not unreal.EditorAssetLibrary.save_loaded_asset(material):
    raise RuntimeError("Could not save the repaired television material.")
map_path = "/Game/StylizedKitchen/Levels/NocturneInterior"
world = unreal.EditorLoadingAndSavingUtils.load_map(map_path)
if world is None:
    raise RuntimeError("Could not load the interior map.")
count = 0
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if actor.get_class().get_name() == "NocturneTelevisionScreen":
        actor.set_editor_property("screen_material", material)
        actor.set_editor_property("enable_distortion", True)
        unreal.log(f"Restored distortion on {actor.get_actor_label()}; video source={actor.get_editor_property('video_source')}")
        count += 1
if count == 0:
    raise RuntimeError("No television screen actor was found.")
if not unreal.EditorLoadingAndSavingUtils.save_map(world, map_path):
    raise RuntimeError("Could not save the television material assignment.")
unreal.log(f"NOCTURNE_DISTORTION_APPLIED: {count} television actor(s); video settings preserved.")
