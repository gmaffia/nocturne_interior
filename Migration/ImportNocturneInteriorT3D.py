import os
import tempfile

import unreal


SOURCE_FILE = os.path.join(
    unreal.Paths.project_dir(),
    "Migration",
    "NocturneInterior_UE58.t3d",
)
TARGET_MAP = "/Game/StylizedKitchen/Levels/NocturneInterior54"
MINIMUM_EXPECTED_ACTORS = 650
SOURCE_MAP_PATH = "/Game/StylizedKitchen/Levels/NocturneInterior"


def normalize_actor_order(level_text: str) -> str:
    actor_start = "      Begin Actor "
    actor_end = "      End Actor"
    lines = level_text.splitlines(keepends=True)
    actor_blocks = []
    output_lines = []
    current_block = None

    for line in lines:
        if line.startswith(actor_start):
            current_block = [line]
        elif current_block is not None:
            current_block.append(line)
            if line.rstrip("\r\n") == actor_end:
                actor_blocks.append("".join(current_block))
                current_block = None
        else:
            output_lines.append(line)

    if current_block is not None:
        raise RuntimeError("The T3D export contains an incomplete actor block.")

    world_settings = [
        block
        for block in actor_blocks
        if block.startswith("      Begin Actor Class=/Script/Engine.WorldSettings ")
    ]
    builder_brushes = [
        block
        for block in actor_blocks
        if block.startswith("      Begin Actor Class=/Script/Engine.Brush ")
    ]
    if len(world_settings) != 1 or not builder_brushes:
        raise RuntimeError("The T3D export is missing WorldSettings or its builder brush.")

    remaining_actors = [
        block
        for block in actor_blocks
        if block not in world_settings and block not in builder_brushes
    ]
    ordered_actors = world_settings + builder_brushes[:1] + remaining_actors

    level_marker = "   Begin Level NAME=PersistentLevel"
    marker_index = next(
        index
        for index, line in enumerate(output_lines)
        if line.rstrip("\r\n") == level_marker
    )
    output_lines[marker_index + 1:marker_index + 1] = ordered_actors
    return "".join(output_lines)


def import_level() -> None:
    if not os.path.isfile(SOURCE_FILE):
        raise RuntimeError(f"Migration source does not exist: {SOURCE_FILE}")

    if unreal.EditorAssetLibrary.does_asset_exist(TARGET_MAP):
        if not unreal.EditorAssetLibrary.delete_asset(TARGET_MAP):
            raise RuntimeError(f"Could not replace migration target: {TARGET_MAP}")

    if not unreal.EditorLevelLibrary.new_level(TARGET_MAP):
        raise RuntimeError(f"Could not create migration target: {TARGET_MAP}")

    # UE 5.4 extracts the import extension from the complete console command.
    # A quoted path containing spaces can therefore be rejected as unsupported.
    # Import through a temporary path without spaces and update the exported map
    # identity so it does not collide with the original UE 5.8 package.
    temporary_file = os.path.join(
        tempfile.gettempdir(), "NocturneInterior_UE54_Import.t3d"
    ).replace("\\", "/")

    with open(SOURCE_FILE, "r", encoding="utf-8-sig") as source_stream:
        level_text = source_stream.read()
    level_text = level_text.replace(SOURCE_MAP_PATH, TARGET_MAP)
    level_text = normalize_actor_order(level_text)
    with open(temporary_file, "w", encoding="utf-8") as temporary_stream:
        temporary_stream.write(level_text)

    world = unreal.EditorLevelLibrary.get_editor_world()
    import_command = f"MAP IMPORTADD FILE={temporary_file}"
    unreal.SystemLibrary.execute_console_command(world, import_command)

    imported_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    if len(imported_actors) < MINIMUM_EXPECTED_ACTORS:
        raise RuntimeError(
            f"Only {len(imported_actors)} actors were imported; "
            f"expected at least {MINIMUM_EXPECTED_ACTORS}."
        )

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError(f"Could not save migrated map: {TARGET_MAP}")

    unreal.log(
        f"Imported {len(imported_actors)} actors into native UE 5.4 map: {TARGET_MAP}"
    )


import_level()
