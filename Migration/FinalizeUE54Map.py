import unreal


SOURCE_MAP = "/Game/StylizedKitchen/Levels/NocturneInterior54"
FINAL_MAP = "/Game/StylizedKitchen/Levels/NocturneInterior"


def finalize_map() -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(FINAL_MAP):
        unreal.log(f"Final UE 5.4 map already exists: {FINAL_MAP}")
        return
    if not unreal.EditorAssetLibrary.does_asset_exist(SOURCE_MAP):
        raise RuntimeError(f"Migrated map does not exist: {SOURCE_MAP}")
    world = unreal.EditorLoadingAndSavingUtils.load_map(SOURCE_MAP)
    if world is None:
        raise RuntimeError(f"Could not load migrated map: {SOURCE_MAP}")
    if not unreal.EditorLoadingAndSavingUtils.save_map(world, FINAL_MAP):
        raise RuntimeError(f"Could not save {SOURCE_MAP} as {FINAL_MAP}")
    unreal.log(f"Finalized native UE 5.4 map: {FINAL_MAP}")


finalize_map()
