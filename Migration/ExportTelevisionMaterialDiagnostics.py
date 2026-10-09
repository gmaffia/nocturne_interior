"""Export saved material properties for a non-mutating comparison."""
from pathlib import Path
import unreal

for name in ("M_NocturneTelevisionScreen", "M_NocturneTelevisionDistorted", "M_NocturneTelevisionDistorted54"):
    material = unreal.load_object(None, f"/NocturneTelevision/Materials/{name}.{name}")
    task = unreal.AssetExportTask()
    task.object = material
    task.exporter = unreal.ObjectExporterT3D()
    task.filename = str(Path(__file__).with_name(name + ".t3d"))
    task.automated = True
    task.prompt = False
    task.replace_identical = True
    if not unreal.Exporter.run_asset_export_task(task):
        raise RuntimeError(f"Could not export {name}")
