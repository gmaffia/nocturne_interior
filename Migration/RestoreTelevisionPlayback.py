"""Restore the known-good video material while the distortion graph is repaired."""
import unreal

material = unreal.load_object(None, "/NocturneTelevision/Materials/M_NocturneTelevisionScreen.M_NocturneTelevisionScreen")
if material is None:
    raise RuntimeError("The known-good television screen material is missing.")
map_path = "/Game/StylizedKitchen/Levels/NocturneInterior"
world = unreal.EditorLoadingAndSavingUtils.load_map(map_path)
if world is None:
    raise RuntimeError("Could not load the interior map.")
count = 0
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if actor.get_class().get_name() == "NocturneTelevisionScreen":
        actor.set_editor_property("screen_material", material)
        unreal.log(f"Restored video material on {actor.get_actor_label()}; video source={actor.get_editor_property('video_source')}")
        count += 1
if count == 0:
    raise RuntimeError("No television screen actor was found.")
if not unreal.EditorLoadingAndSavingUtils.save_map(world, map_path):
    raise RuntimeError("Could not save the restored television material assignment.")
unreal.log(f"NOCTURNE_PLAYBACK_MATERIAL_RESTORED: {count} television actor(s), video settings preserved.")
