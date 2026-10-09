# Nocturne Interior

Nocturne Interior is a first-person atmospheric horror prototype built with Unreal Engine 5.4.4. It combines a furnished stylized interior with custom runtime systems for an analog-horror post-process look, a video-driven television, a volumetric strobe light, player stance controls, and scalable rendering features.

The project is currently configured for Windows and starts directly in the main `NocturneInterior` level.

## Requirements

- Unreal Engine 5.4.4
- Windows 10 or Windows 11
- Visual Studio 2022 with **MSVC v143 C++ x64/x86 build tools**
- A recent Windows SDK installed through the Visual Studio Installer
- DirectX 12 and Shader Model 6 support

The game and editor targets use `BuildSettingsVersion.V5` and the Unreal 5.4 include order.

Hardware ray tracing is enabled in the project settings. It can be disabled at runtime, but using the hardware path requires compatible GPU hardware and drivers.

## Getting Started

1. Clone or copy the project.
2. Right-click `NocturneInterior.uproject` and select **Generate Visual Studio project files** if the solution is missing or stale.
3. Open `NocturneInterior.uproject` in Unreal Engine 5.4.4.
4. Allow Unreal to compile the native project modules when prompted.
5. Open `/Game/StylizedKitchen/Levels/NocturneInterior` if it is not already loaded.
6. Use **Play** to run the first-person experience.

If Unreal reports that a module is missing or was built with another engine version, close the editor, regenerate the project files, and build the `NocturneInteriorEditor` target in Visual Studio using the `Development Editor` and `Win64` configuration.

## Project Structure

```text
NocturneInterior/
|-- Config/                         Project, input, rendering, and user-setting defaults
|-- Content/
|   |-- Audio/                      Project sound assets
|   |-- Characters/Whisper/         Subject skeletal mesh, materials, animations, and test map
|   |-- Movies/                     Television media and media-source assets
|   |-- Nocturne/                   Project-specific materials and assets
|   |-- NocturneLook/               Project assets used by the player/look system
|   `-- StylizedKitchen/             Environment maps, meshes, materials, and textures
|-- Plugins/
|   |-- NocturneLook/               Post process, player character, HUD, and graphics controls
|   |-- NocturneStrobe/             Configurable strobe spotlight and directional audio
|   `-- NocturneTelevision/         Video screen, analog distortion, audio, and reactive light
|-- Source/NocturneInterior/        Minimal native game module
|-- NocturneInterior.uproject       Unreal project descriptor
`-- .gitignore                      Generated-file and local-tool exclusions
```

The following directories are generated locally and must not be treated as source: `Binaries`, `Build`, `BuildFixed`, `DerivedDataCache`, `Intermediate`, and `Saved`. Generated equivalents inside plugins are also excluded from version control.

## Maps

| Map | Purpose |
| --- | --- |
| `/Game/StylizedKitchen/Levels/NocturneInterior` | Main playable level and both the editor and packaged-game startup map. |
| `/Game/StylizedKitchen/Levels/Stylized_Interior` | Original imported furnished environment. |
| `/Game/StylizedKitchen/Levels/AssetsOverview` | Overview of the imported environment assets. |
| `/Game/Characters/Whisper/Maps/Whisper` | Character asset test map. |

## Runtime Systems

### Nocturne Look

`NocturneLook` supplies the reusable atmospheric post-process actor and the project's first-person framework:

- `NocturneLookController` controls exposure, film grain, vignette, chromatic aberration, saturation, and contrast.
- `NocturnePlayerCharacter` implements grounded movement, mouse capture, head bob, and standing, seated, and crouched camera heights.
- `NocturnePlayerGameMode` makes the custom player character and HUD the project defaults.
- `NocturnePlayerHUD` draws control hints and the runtime graphics-feature panel.

See [Plugins/NocturneLook/README.md](Plugins/NocturneLook/README.md) for installation and Blueprint API details.

### Nocturne Television

`NocturneTelevisionScreen` renders a `File Media Source` on an emissive surface and supports:

- Autoplay and looping video
- Media audio
- Scanlines, animated noise, jitter, vertical roll, chromatic aberration, flicker, vignette, and tracking distortion
- Stable rect-light illumination
- Optional asynchronous video sampling that changes room-light brightness and color with the image

Television video files must remain under `Content/Movies` so Unreal can stage them correctly for packaged builds. A checkerboard screen normally means the media source did not open, the media file was not staged, or the screen material lost its media texture input.

The UE 5.4 distortion material uses native material nodes. With Substrate enabled, its processed video must feed the emissive input of the node connected to `Front Material`; connecting only the legacy `Emissive Color` output does not update the rendered screen. The main level uses `M_NocturneTelevisionDistorted`, with distortion enabled and the original media source preserved. The migration render test checks that disabling distortion restores the original image and enabling it changes the rendered pixels.

See [Plugins/NocturneTelevision/README.md](Plugins/NocturneTelevision/README.md) for setup and tuning details.

### Nocturne Strobe

`NocturneStrobeLight` is a reusable spotlight actor with:

- Regular, double-flash, and seeded irregular pulse patterns
- Fixed or cycling cosmic-violet color modes
- Oscillating movement or continuous axial spin
- Configurable intensity, cone, attenuation, shadows, and volumetric scattering
- Optional directional audio aligned with the light
- Blueprint-callable runtime controls

Visible beams require an `ExponentialHeightFog` actor with **Volumetric Fog** enabled. Fog density, the strobe's volumetric scattering intensity, and effects quality all influence beam visibility and performance.

See [Plugins/NocturneStrobe/README.md](Plugins/NocturneStrobe/README.md) for placement and fog recommendations.

## Controls

### Player

| Input | Action |
| --- | --- |
| `W`, `A`, `S`, `D` | Move |
| Mouse | Look |
| `E` | Raise stance: crouched to seated to standing |
| `C` | Lower stance: standing to seated to crouched |
| `H` | Show or hide the main control hints |

The player begins seated by default. Camera heights, movement speed, acceleration, friction, gravity, and head bob can be adjusted on the player class or its Blueprint defaults.

### Graphics and Lighting

| Input | Action |
| --- | --- |
| `G` | Cycle Performance, High, and Native graphics profiles |
| `P` | Show or hide the graphics-feature panel |
| `J` | Toggle Lumen hardware or software ray tracing |
| `V` | Toggle volumetric fog |
| `T` | Toggle television-light shadows |
| `Y` | Toggle strobe shadows |
| `B` | Toggle the strobe between volumetric and surface-only lighting |

The graphics profiles change resolution scale, volumetric grid resolution, Virtual Shadow Map sampling, and shadow-page budgets. Feature-toggle choices are persisted in the local game user settings.

Virtual Shadow Maps and Substrate are project-level features and require a restart if their configuration changes.

### Nocturne Look Controller

These defaults apply while a `NocturneLookController` actor has runtime keyboard input enabled:

| Input | Action |
| --- | --- |
| `Z` | Show or hide the look-tuning panel |
| `X` | Enable or disable the look |
| `R` | Reset the look |
| `1` / `2` | Decrease or increase exposure |
| `3` / `4` | Decrease or increase film grain |
| `5` / `6` | Decrease or increase vignette |
| `7` / `8` | Decrease or increase chromatic aberration |

### Strobe

The default keys are configurable independently on each `NocturneStrobeLight` actor:

| Input | Action |
| --- | --- |
| `L` | Enable or disable the strobe |
| `K` | Show or hide the strobe controls panel |

Avoid assigning `F1` and `F2` to gameplay features because the Unreal Editor and viewport may consume those keys for editor visualization modes.

## Rendering Configuration

The default renderer is intentionally quality-focused:

- DirectX 12 with Shader Model 6
- Dynamic global illumination and reflections through Lumen
- Hardware ray tracing enabled at project level
- Virtual Shadow Maps
- Mesh distance fields
- Substrate materials
- Static lighting disabled
- High-resolution volumetric fog grids
- Maximum default scalability at 2560 x 1440

These settings can be expensive, especially volumetric fog, strobe shadows, Lumen hardware ray tracing, and the 100% resolution profile. Use the runtime controls to compare techniques and isolate performance costs before changing the project defaults.

## Packaging for Windows

Before packaging:

1. Compile the project and all three runtime plugins for `Win64`.
2. Save the main level and all modified assets.
3. Confirm that the startup map is `/Game/StylizedKitchen/Levels/NocturneInterior`.
4. Keep television media files in `Content/Movies` and verify playback in a standalone game session.
5. Package through **Platforms > Windows > Package Project**.

The custom plugins contain native C++ runtime modules. A packaged build must include binaries compiled for the same engine version and target configuration; copying only the plugin descriptors or editor-built binaries is insufficient.

## Version Control

Commit the following:

- `Config`
- `Content`
- `Plugins`, excluding generated plugin directories
- `Source`
- `NocturneInterior.uproject`
- Documentation and repository metadata

Do not commit generated build output or local editor state. Unreal assets and media are binary files, so Git LFS is strongly recommended for `*.uasset`, `*.umap`, large source textures, audio, and video when this repository is shared remotely.

## Content and Licensing

The `StylizedKitchen` environment and the `Whisper` character are imported content rather than original engine primitives. Before distributing the repository or a packaged build, verify the applicable marketplace or source licenses and document required attribution. Do not assume that possession of the local asset files grants permission to redistribute their editable source form.

## Development Conventions

- Use English for source-code identifiers, comments, asset documentation, UI text intended for development, and repository documentation.
- Keep reusable systems inside their corresponding plugin rather than adding project-specific dependencies to them.
- Keep scene-specific materials and configuration under `Content/Nocturne`.
- Prefer instance-editable actor properties for artistic tuning and Blueprint-callable functions for runtime control.
- Test important rendering and media behavior in a packaged build as well as in Play In Editor; editor-only fallback behavior can hide staging or runtime-module problems.
