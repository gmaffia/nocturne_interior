# Nocturne Strobe

`NocturneStrobe` is a reusable Unreal Engine 5.4.4 runtime plugin that provides a configurable volumetric strobe spotlight with optional sweeping motion.

## Placement

1. Enable the plugin and restart Unreal Editor.
2. Place one or more `NocturneStrobeLight` actors in the level.
3. Rotate each actor so its local positive X axis points toward the target area.
4. For a backlit subject, place the actor behind the subject and aim it through the window.

## Volumetric Lighting

Visible light beams require an enabled `ExponentialHeightFog` actor with `Volumetric Fog` enabled. Use low fog density and adjust `Volumetric Scattering Intensity` on each strobe actor. `Cast Volumetric Shadow` is enabled by default and can be disabled per actor when maximum performance is preferred over subject silhouettes inside the beam.

A useful starting range for a dark interior is:

- Fog Density: `0.005` to `0.02`
- Volumetric Fog Extinction Scale: `1.0` to `1.5`
- Strobe Volumetric Scattering Intensity: `4.0` to `10.0`
- Effects scalability: `High` or above

## Patterns

- `Regular` produces a single pulse per cycle.
- `Double Flash` produces two closely spaced pulses per cycle.
- `Irregular` randomizes the interval between pulses using the configured seed.

## Sweep

The sweep system oscillates rather than rotating continuously. `Yaw Amplitude`, `Pitch Amplitude`, `Sweep Frequency`, and `Sweep Phase Degrees` can be different on every placed instance.

## Blueprint Control

The actor exposes `SetStrobeEnabled`, `ToggleStrobe`, `TriggerFlash`, `RefreshSettings`, and `IsStrobeEnabled`.
