# AlienThemePark
**COSC363 Assignment 1 — Ilan Germano (37527645)**

An alien-themed rollercoaster park set in outer space, built with OpenGL 2 / FreeGLUT.

---

## Requirements

- Linux or WSL2 (Ubuntu recommended)
- CMake >= 2.8
- FreeGLUT (`libglut-dev`)
- OpenGL + GLU (`libgl-dev`, `libglu-dev`)

Install dependencies on Ubuntu/WSL2:
```bash
sudo apt update
sudo apt install cmake freeglut3-dev libgl-dev libglu-dev
```

---

## Build & Run

```bash
cd AlienThemePark
mkdir build
cd build
cmake ..
make
./AlienThemePark.out
```

> **Note:** If you add new texture files, re-run `cmake ..` before `make` so they are copied into the build directory.

---

## Controls

| Key / Input     | Action                          |
|-----------------|---------------------------------|
| `W / S`         | Move camera forward / backward  |
| `A / D`         | Move camera left / right        |
| `Spacebar`      | Fly camera up                   |
| `F`             | Fly camera down                 |
| `Q`             | Toggle wireframe / solid mode   |
| `ESC`           | Exit the program                |
| Mouse drag      | Look around (first-person)      |

---

## Textures Required

The following texture files must be present in the `AlienThemePark/` directory before building:

| File              | Used for         |
|-------------------|------------------|
| `Moon.tga`        | Ground plane     |
| `Earth.bmp`       | Background planet|
| `Sun.bmp`         | Background planet|
| `Stone_Texture.tga` | Station tunnel |
| `space_ft.tga`    | Skybox front     |
| `space_bk.tga`    | Skybox back      |
| `space_lf.tga`    | Skybox left      |
| `space_rt.tga`    | Skybox right     |
| `space_up.tga`    | Skybox top       |
| `space_dn.tga`    | Skybox bottom    |

---

## Features

- Catmull-Rom spline track with arc-length parameterisation
- Physics-based cart speed (faster downhill, slower uphill)
- Boarding and alighting alien animation at the station
- Space skybox (6-faced cube)
- Dynamic spotlight attached to the lead cart
- Planar shadows projected onto the ground
- Station tunnel — linear extrusion sweep surface
- Support pillars — surface of revolution
- Smooth first-person camera (WASD + mouse)