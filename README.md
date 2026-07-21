# HZL Particle Dynamics Engine

A real-time 3D particle dynamics engine written in C++20, OpenGL 3.3, and CUDA. It simulates spherical particles with gravity, container collisions, and particle-particle collisions, then renders the GPU-produced positions through a shared CUDA-OpenGL buffer.

The current demo launches red particles from the left and blue particles from the right, making their collisions easy to inspect visually.

## Accomplishments

- Developed a real-time 3D particle dynamics engine in C++20, OpenGL 3.3, and CUDA, with fixed-timestep integration, gravity, container boundaries, and sphere-sphere collision response.
- Implemented a GPU uniform-grid broad phase and iterative Jacobi collision solver, reducing collision candidate checks to each particle's local 27-cell neighborhood.
- Built a GPU-resident simulation-to-rendering pipeline using CUDA-OpenGL interoperability and instanced sphere rendering, avoiding per-frame particle-position transfers through CPU memory.
- Added Catch2/CUDA correctness tests and performance benchmarks; verified 100,000 particles with four collision iterations in 4.3505 ms per GPU update on an RTX 3080 Release build.

## Highlights

- C++20 application structured into application, platform, renderer, scene, and simulation modules.
- OpenGL 3.3 renderer with lit sphere meshes and instanced particle drawing.
- Fixed-timestep simulation at 120 Hz.
- GPU-resident CUDA integration, broad phase, and particle collision solving.
- Uniform-grid broad phase: each particle checks its own cell and 26 neighboring cells instead of every particle.
- Four configurable collision-solver iterations per timestep for improved dense-cluster separation.
- CUDA-OpenGL interoperability: CUDA writes positions directly into the OpenGL instance VBO.
- CPU fallback implementation, Catch2 tests, CUDA smoke test, and CPU/GPU benchmarks.

## Architecture

```text
Application
+-- platform/
|   +-- GlfwWindow                 Window, timing, keyboard input
+-- renderer/
|   +-- OpenGLRenderer             OpenGL frame rendering
|   +-- OpenGLMesh                 Cube/sphere geometry and instancing
|   +-- OpenGLProgram              Shader loading and uniforms
|   +-- CudaOpenGLParticleBuffer   CUDA-OpenGL shared position VBO
+-- scene/
|   +-- Container                  Floor/wall rendering and physics bounds
+-- simulation/
    +-- ParticleSystem             Fixed-step orchestration and CPU fallback
    +-- CudaParticleIntegrator     GPU gravity and container collisions
    +-- CudaUniformGrid            GPU cell-key generation and sorting
    +-- CudaParticleCollisionSolver GPU Jacobi collision passes
    +-- CudaParticleBuffer          GPU-resident particle state
```

## GPU pipeline

```text
CPU upload once
    ->
CUDA integration
    ->
[GPU uniform-grid build -> GPU collision solver] x 4
    ->
CUDA writes positions into a mapped OpenGL VBO
    ->
OpenGL instanced sphere draw
```

`ParticleSystem` owns a non-blocking CUDA stream. Integration, grid construction, collision solving, and VBO population are queued on that stream. The CPU does not wait after each physics stage; synchronization occurs at the CUDA-OpenGL handoff, CPU fallback, teardown, and test-result boundaries.

## Requirements

- Windows 10/11
- Visual Studio 2022 with Desktop development with C++
- CMake 3.25 or newer
- NVIDIA CUDA Toolkit 12.6 or compatible toolkit
- NVIDIA GPU supporting CUDA and OpenGL interoperability
  - Tested on NVIDIA GeForce RTX 3080, compute capability 8.6
- Python with the GLAD 2 generator:

```powershell
python -m pip install "glad2==2.0.8"
```

The project uses CMake `FetchContent` to obtain GLFW, GLAD, GLM, and Catch2 during configuration.

## Build and run

1. Open the repository folder in Visual Studio 2022.
2. Select either `x64-Debug` or `x64-Release` from `CMakeSettings.json`.
3. Run **CMake generation**, then **Build All**.
4. Set `HZLParticleDynamicsEngine.exe` as the startup target and run it.

The application copies `assets/` beside the executable after each build, so shader file paths work from the build output directory.

### Controls

| Key | Action |
| --- | --- |
| `W` / `S` | Move camera forward / backward |
| `A` / `D` | Strafe camera left / right |
| `Esc` | Close the application |

## Tests

The test suite covers gravity, floor and wall collisions, head-on particle collisions, dense-overlap solver iterations, CUDA integration, CUDA grid grouping, and CUDA collision solving.

After building the `x64-Debug` configuration, run:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe" `
    --test-dir out/build/x64-Debug `
    --output-on-failure
```

Latest verified result:

```text
All tests passed (41 assertions in 9 test cases)
```

### Release validation

Before publishing a release, select `x64-Release` in Visual Studio, reconfigure, build, and run the same test suite against the Release build directory:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe" `
    --test-dir out/build/x64-Release `
    --output-on-failure
```

Latest verified Release result:

```text
100% tests passed, 0 tests failed out of 2
```

## Benchmarks

Build and run the `HZLParticleSystemCudaBenchmark` target to measure GPU-resident integration, grid construction/sorting, and collision solving. The benchmark uploads particles once, measures with CUDA events, and excludes host-device transfer time from the timed loop.

RTX 3080 Debug measurements:

| Particle count | 1 solver iteration | 4 solver iterations |
| ---: | ---: | ---: |
| 1,000 | 0.1539 ms | 0.5281 ms |
| 10,000 | 0.3197 ms | 1.1111 ms |
| 100,000 | 1.4152 ms | 5.1063 ms |

RTX 3080 Release measurements:

| Particle count | 1 solver iteration | 4 solver iterations |
| ---: | ---: | ---: |
| 1,000 | 0.1145 ms | 0.4397 ms |
| 10,000 | 0.2470 ms | 0.8939 ms |
| 100,000 | 0.8668 ms | 4.3505 ms |

At 120 Hz, one simulation update has an 8.33 ms budget. The verified Release measurement for 100,000 particles and four solver iterations is 4.3505 ms, which fits within that GPU-only budget.

Other available benchmark targets:

- `HZLParticleDynamicsBenchmark` - CPU particle-system benchmark.
- `HZLParticleIntegrationBenchmark` - CPU versus CUDA integration benchmark.

## Current limitations and future work

- The GPU collision solver uses a Jacobi-style approximation; dense scenes may need more iterations for greater rigidity.
- CUDA physics and OpenGL rendering share particle positions, while particle colors remain a static OpenGL instance buffer.
- The current benchmark measures simulation work; a future benchmark can measure complete simulation-to-render frame cost.
- Future extensions include CUDA/OpenGL profiling, dynamic particle spawning, UI controls, and GPU-friendly broad-phase refinements.

## License

This project is distributed under the terms in [LICENSE](LICENSE).
