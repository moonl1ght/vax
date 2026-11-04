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

Free for non-commercial use. Commercial use requires written permission.
Using this code as training data for AI/LLM models is not allowed for any purpose, including educational, research, and other non-commercial use.
Training and running models (e.g. RL agents) with the framework itself is allowed for non-commercial use.
Using AI to replicate or re-create this project as a separate work is not allowed.
Cloning or forking it and modifying it with AI assistance is allowed for non-commercial use.
See [LICENSE](LICENSE) for the full terms.
