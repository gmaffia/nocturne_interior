# Nocturne Television

`NocturneTelevision` is a reusable Unreal Engine 5.4.4 runtime plugin that displays a video on an emissive television surface and provides configurable analog distortion, audio, and room lighting.

## Setup

1. Enable the plugin and restart Unreal Editor.
2. Add a `File Media Source` asset and select a video stored in the project's `Content/Movies` directory.
3. Place a `NocturneTelevisionScreen` actor slightly in front of the television glass.
4. Assign the media source to `Video Source`.
5. Match `Screen Width` and `Screen Height` to the television display.
6. Rotate the actor so its local positive X axis faces the room.

## Playback

The actor supports autoplay and looping and exposes these Blueprint-callable functions:

- `OpenVideoSource`
- `Play`
- `Pause`
- `Stop`
- `TogglePlayback`

## Analog Distortion

The `Distortion` category exposes independent controls for scanlines, animated noise, horizontal jitter, vertical roll, chromatic aberration, flicker, vignette, and a moving tracking bar. `Distortion Amount` blends the complete effect with the clean video, while `Enable Distortion` provides a runtime-safe bypass.

Call `RefreshVisualSettings` after changing visual properties from Blueprint during play. Properties edited in the Details panel are refreshed automatically when the actor is reconstructed.

## Lighting

The screen material provides emissive brightness through `Emissive Intensity`. A `Rect Light` provides stable room illumination through `Light Intensity`, `Light Color`, and `Light Attenuation Radius`.

Enable `Reactive Light` to make the room light follow the video's average brightness and color. The plugin samples an 8 x 8 render target at a configurable rate and smooths the result to avoid abrupt light changes.

- `Reactive Light Sample Rate` controls how often the video is sampled. The default 8 Hz is recommended.
- `Reactive Light Color Strength` controls how closely the light follows the video's color.
- `Reactive Light Intensity Strength` controls how strongly video brightness affects light intensity.
- `Reactive Light Brightness Gain`, `Minimum Brightness`, and `Maximum Brightness` define the response range.
- `Reactive Light Smoothing Speed` controls transition speed and reduces visible stepping.

Reactive lighting uses a small asynchronous GPU readback. The CPU never waits for the GPU; if a sample is still pending, the plugin keeps the previous light value and tries again on a later frame. Keep the sample rate between 4 and 12 Hz for predictable performance.
