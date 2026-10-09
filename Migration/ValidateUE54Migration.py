import unreal


FINAL_MAP = "/Game/StylizedKitchen/Levels/NocturneInterior"
MINIMUM_EXPECTED_ACTORS = 650


def validate_migration() -> None:
    world = unreal.EditorLoadingAndSavingUtils.load_map(FINAL_MAP)
    if world is None:
        raise RuntimeError(f"Could not load final map: {FINAL_MAP}")

    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    if len(actors) < MINIMUM_EXPECTED_ACTORS:
        raise RuntimeError(
            f"Final map contains only {len(actors)} actors; "
            f"expected at least {MINIMUM_EXPECTED_ACTORS}."
        )

    class_counts = {}
    actor_labels = set()
    television_actors = []
    for actor in actors:
        class_name = actor.get_class().get_name()
        class_counts[class_name] = class_counts.get(class_name, 0) + 1
        actor_labels.add(actor.get_actor_label())
        if class_name == "NocturneTelevisionScreen":
            television_actors.append(actor)

    required_classes = (
        "NocturneLookController",
        "NocturneStrobeLight",
        "NocturneTelevisionScreen",
        "PlayerStart",
    )
    missing_classes = [name for name in required_classes if class_counts.get(name, 0) == 0]
    if missing_classes:
        raise RuntimeError(f"Final map is missing required actor classes: {missing_classes}")

    for television in television_actors:
        if television.get_editor_property("video_source") is None:
            raise RuntimeError("A television actor has no video source.")
        if television.get_editor_property("screen_material") is None:
            raise RuntimeError("A television actor has no screen material.")

    rebuilt_assets = (
        "/NocturneTelevision/Materials/M_NocturneTelevisionDistorted",
        "/NocturneTelevision/Materials/M_NocturneTelevisionScreen",
        "/Game/Movies/LivingroomTelevision",
        "/Game/Nocturne/Materials/M_SubjectVoidBlack",
        "/Game/NocturneLook/Materials/MI_Nocturne_NightSky",
    )
    missing_assets = []
    for asset_path in rebuilt_assets:
        asset_name = asset_path.rsplit("/", 1)[1]
        if unreal.load_object(None, f"{asset_path}.{asset_name}") is None:
            missing_assets.append(asset_path)
    if missing_assets:
        raise RuntimeError(f"UE 5.4 assets failed to load: {missing_assets}")

    unreal.log(
        "UE 5.4 migration validation passed: "
        f"{len(actors)} actors, "
        f"{class_counts['NocturneTelevisionScreen']} television, "
        f"{class_counts['NocturneStrobeLight']} strobe, "
        f"{class_counts['NocturneLookController']} look controller, "
        f"{class_counts['PlayerStart']} player start."
    )


validate_migration()
