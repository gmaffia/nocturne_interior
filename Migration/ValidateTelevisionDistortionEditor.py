"""Run the render check in a full editor session and close without saving."""
from pathlib import Path
import unreal

try:
    script = Path(__file__).with_name("ValidateTelevisionDistortion.py")
    exec(compile(script.read_text(encoding="utf-8"), str(script), "exec"))
finally:
    unreal.SystemLibrary.quit_editor()
