# VAX

<p align="center">
  <img src="docs/images/logo.png" alt="VAX logo" width="480">
</p>

<p align="center"><b>Simulator for training RL agents</b></p>

![Linux](https://img.shields.io/badge/Linux-tested-success?logo=linux&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-untested-lightgrey?logo=windows&logoColor=white)
![macOS](https://img.shields.io/badge/macOS-untested-lightgrey?logo=apple&logoColor=white)

VAX is an open-source, Vulkan-based simulator for training reinforcement-learning agents.
It runs the full loop inside one engine: sensors → policy → actions → physics → rendering.

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

- CMake 3.18+
- Conan 2.x
- Vulkan SDK (set `VULKAN_SDK` environment variable)

## License

VAX is licensed under the [Apache License 2.0](LICENSE).
You are free to use, modify, and distribute it, including for commercial purposes,
as long as you keep the copyright notice and the [NOTICE](NOTICE) file.
The license includes an explicit patent grant from contributors and terminates
the patent rights of anyone who files a patent lawsuit over this project.

Third-party components are distributed under their own licenses,
see [NOTICE](NOTICE).

Contributions are accepted under the same license (Apache-2.0, Section 5).
