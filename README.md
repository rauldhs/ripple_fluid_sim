# Ripple - an SPH fluid sim

A real-time SPH (Smoothed Particle Hydrodynamics) fluid simulation written in C++ with OpenGL rendering.

> Work in progress — multithreading and mesh generation are planned but not yet implemented.

<div align="center">
  <img src="gallery/20260224-233403.gif" width="96%"/>
  <br/>
  <img src="gallery/20260224-232452.gif" width="31%"/>
  <img src="gallery/20260224-232521.gif" width="31%"/>
  <img src="gallery/20260224-232630.gif" width="31%"/>
</div>

## Features

- SPH fluid simulation with pressure, viscosity, and surface tension forces
- Real-time 3D rendering via OpenGL 4.6 with instanced particle drawing
- Interactive camera
- ImGui control panel for tweaking simulation parameters at runtime
- Z-order (Morton code) sorting for improved cache coherency
- Spatial hash grid for neighbor queries

## Papers read

**Smoothed Particle Hydrodynamics Techniques for the Physics Based Simulation of Fluids and Solids**
Dan Koschier, Jan Bender, Barbara Solenthaler, Matthias Teschner

**Particle-Based Fluid Simulation for Interactive Applications**
Matthias Müller, David Charypar, Markus Gross

**A Parallel SPH Implementation on Multi-Core CPUs**
Markus Ihmsen, Nadir Akinci, Markus Becker, Matthias Teschner

## Controls

| Key | Action |
|-----|--------|
| `W A S D` | Move camera |
| `Mouse` | Look around |
| `L` | Lock mouse |
| `U` | Unlock mouse |
| `Esc` | Quit |

## Building

### 1. Clone

```bash
git clone https://github.com/rauldhs/ripple_fluid_sim
cd ripple
```

### 2. Install dependencies

```bash
mkdir externals && cd externals

# GLM
git clone https://github.com/g-truc/glm.git

# GLFW
git clone https://github.com/glfw/glfw.git

# ImGui
git clone https://github.com/ocornut/imgui.git

# GLAD — generate OpenGL 4.6 Core bindings at:
# https://gen.glad.sh/#generator=c&api=gl%3D4.6&profile=gl%3Dcore%2Cgles1%3Dcommon&options=ALIAS%2CDEBUG%2CLOADER
# Then move the generated files into externals/glad/

cd ..
```

### 3. Build

```bash
mkdir build
cmake -S . -B build
cmake --build build
```

### 4. Run

```bash
./build/ripple
```

## Dependencies

- **OpenGL 4.6** — instanced rendering, DSA(bindless)
- **GLFW** — windowing and input
- **GLM** — math stuff
- **ImGui** — UI
- **GLAD** — OpenGL loader

## Todo

- [ ] Multithreaded force computation
- [ ] Mesh generation
- [ ] Configurable spawn volumes
- [ ] Other cool stuff
