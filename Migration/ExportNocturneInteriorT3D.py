import os

import unreal


SOURCE_MAP = "/Game/StylizedKitchen/Levels/NocturneInterior"
OUTPUT_DIRECTORY = os.path.join(unreal.Paths.project_dir(), "Migration")
OUTPUT_FILE = os.path.join(OUTPUT_DIRECTORY, "NocturneInterior_UE58.t3d")


def export_level() -> None:
    os.makedirs(OUTPUT_DIRECTORY, exist_ok=True)

    world = unreal.EditorLoadingAndSavingUtils.load_map(SOURCE_MAP)
    if world is None:
        raise RuntimeError(f"Could not load source map: {SOURCE_MAP}")

    export_task = unreal.AssetExportTask()
    export_task.set_editor_property("object", world)
    export_task.set_editor_property("filename", OUTPUT_FILE)
    export_task.set_editor_property("automated", True)
    export_task.set_editor_property("prompt", False)
    export_task.set_editor_property("replace_identical", True)
    export_task.set_editor_property("write_empty_files", False)
    export_task.set_editor_property("exporter", unreal.LevelExporterT3D())

    if not unreal.Exporter.run_asset_export_task(export_task):
        raise RuntimeError(f"Could not export level to: {OUTPUT_FILE}")

    unreal.log(f"Exported backward-migration level data to: {OUTPUT_FILE}")


export_level()
