# Nocturne Look

`NocturneLook` is a reusable Unreal Engine 5.4.4 runtime plugin for a dark, grainy, desaturated visual treatment.

## Installation

1. Copy the `NocturneLook` directory into the target project's `Plugins` directory.
2. Open the target project and enable **Nocturne Look** if Unreal does not enable it automatically.
3. Restart Unreal Editor and compile the project when prompted.
4. Place one `NocturneLookController` actor in the level.

To use the included first-person movement, set the project's default game mode to `NocturnePlayerGameMode`. The game mode spawns `NocturnePlayerCharacter`, which uses `WASD` and mouse input with grounded movement, braking, friction, gravity, and subtle walking head bob.

Required axis mappings:

- `MoveForward`: `W` at `1.0`, `S` at `-1.0`
- `MoveRight`: `D` at `1.0`, `A` at `-1.0`
- `Turn`: `Mouse X` at `1.0`
- `LookUp`: `Mouse Y` at `-1.0`

Do not combine the controller with another unbound post-process volume unless the additional volume has a lower priority and intentionally complementary settings.

## Editor controls

Select the controller and use the `Nocturne Look` categories in the Details panel to adjust exposure, film grain, vignette, chromatic aberration, saturation, and contrast. Changes are previewed in the editor viewport.

## Runtime controls

- `Z`: show or hide the tuning panel
- `X`: enable or disable the look
- `R`: reset the look
- `1` / `2`: decrease or increase exposure
- `3` / `4`: decrease or increase film grain
- `5` / `6`: decrease or increase vignette
- `7` / `8`: decrease or increase chromatic aberration

## Runtime API

The controller exposes these Blueprint-callable functions:

- `ToggleLookEnabled`
- `ResetLook`
- `AdjustExposure`
- `AdjustFilmGrain`
- `AdjustVignette`
- `AdjustChromaticAberration`

The controller owns the only required post-process component. Toggling changes its blend weight between a near-zero neutral value and one, re-registering the component so Unreal cannot prune it from the active post-process list.
