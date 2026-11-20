# VAX

<p align="center">
  <img src="docs/images/logo.png" alt="VAX logo" width="480">
</p>

<p align="center"><b>Simulator for training RL agents</b></p>

<p align="center">
  <img src="https://img.shields.io/badge/Linux-developing-success?logo=linux&logoColor=white" alt="Linux: developing">
  <img src="https://img.shields.io/badge/Windows-untested-lightgrey?logo=windows&logoColor=white" alt="Windows: untested">
  <img src="https://img.shields.io/badge/macOS-untested-lightgrey?logo=apple&logoColor=white" alt="macOS: untested">
</p>

VAX is an open-source, Vulkan-based simulator for training reinforcement-learning agents.
It runs the full loop inside one engine: sensors → policy → actions → physics → rendering.

## Roadmap

- 🟢 **Engine v0.1** — real-time Vulkan renderer and the base to build the simulator on:
  bindless resources, PBR with IBL, shadows, render pass graph, CPU-driven indirect drawing,
  compute passes, Slang shaders, ECS, glTF/KTX asset loading.
- 🟡 **Simulation loop** — fixed-step simulation decoupled from rendering, headless mode, offscreen sensors
- 🟡 **Physics** — rigid bodies, colliders, joints, URDF robots
- ⚪ **Policy training & inference** — RL and VLA models controlling the rover inside the simulation loop
- ⚪ **Real2Sim** — scanned real rooms as environments: meshes for physics, Gaussian splats for cameras
- ⚪ **Software-in-the-loop** — motor models, C controller with a HAL, ROS 2

🟢 done · 🟡 in progress · ⚪ planned

## Build

### linux
```bash
conan install . --output-folder=build --build=missing --profile=profiles/linuxprofile -s "ktx/*:compiler.cppstd=gnu17"
```

## Debugging Slang Shaders

To debug Slang source (not SPIR-V) in RenderDoc or a shader debugger:

1. Add `-g -O0` flags to the Slang compilation command
2. Comment `float3 baryCoords : SV_Barycentrics;` in shaders - this built-in is incompatible with Slang-level debug info

## Requirements

**Hardware**
- GPU with Vulkan 1.4 support and `VK_KHR_fragment_shader_barycentric`
- NVIDIA GPU for CUDA (architecture is set by `CUDA_ARCH` in `CMakeLists.txt`, default `sm_120`)
- x86-64 CPU with AVX2

**Tools**
- GCC 14+ (C++23)
- CMake 3.18+ and Ninja
- Conan 2.x — installs the other C++ dependencies (see [conanfile.txt](conanfile.txt))
- Vulkan SDK 1.4+ (set `VULKAN_SDK` environment variable)
- [Slang](https://github.com/shader-slang/slang) SDK — `slangc` on `PATH`, in `/usr/slang`, or set `SLANG_DIR`
- CUDA Toolkit 13+ (expected at `/usr/local/cuda`, override with `-DCUDA_PATH=...`)

**Optional**
- `spirv-val` (from the Vulkan SDK) — validates shaders during the build
- [mold](https://github.com/rui314/mold) — faster linking, used automatically if found
- [gltf-ibl-sampler](https://github.com/KhronosGroup/glTF-IBL-Sampler) — regenerates IBL maps ([resources/brdf/generate_ibl.sh](resources/brdf/generate_ibl.sh))

## License

VAX is licensed under the [Apache License 2.0](LICENSE).
You are free to use, modify, and distribute it, including for commercial purposes,
as long as you keep the copyright notice and the [NOTICE](NOTICE) file.
The license includes an explicit patent grant from contributors and terminates
the patent rights of anyone who files a patent lawsuit over this project.

Third-party components are distributed under their own licenses,
see [NOTICE](NOTICE).

Contributions are accepted under the same license (Apache-2.0, Section 5).
