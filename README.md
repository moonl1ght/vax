# VAX

Vulkan RL gym.

## Build

### linux
```bash
conan install . --output-folder=build --build=missing --profile=linuxprofile -s "ktx/*:compiler.cppstd=gnu17"
```

## Debugging Slang Shaders

To debug Slang source (not SPIR-V) in RenderDoc or a shader debugger:

1. Add `-g -O0` flags to the Slang compilation command
2. Comment `float3 baryCoords : SV_Barycentrics;` in shaders - this built-in is incompatible with Slang-level debug info

## IBL Env genetation

```bash
gltf-ibl-sampler -inputPath ./university_workshop_4k.hdr -outCubeMap ./charlie.ktx -distribution Charlie -cubeMapResolution 512
gltf-ibl-sampler -inputPath ./university_workshop_4k.hdr -outCubeMap ./prefilter.ktx -distribution GGX -cubeMapResolution 512
gltf-ibl-sampler -inputPath ./university_workshop_4k.hdr -outCubeMap ./irradiance.ktx -distribution Lambertian -cubeMapResolution 256
```

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
