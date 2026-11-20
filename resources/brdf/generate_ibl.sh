#!/usr/bin/env bash
# Generates IBL cubemaps from an HDR environment with gltf-ibl-sampler.
# Usage: ./generate_ibl.sh [input.hdr]   (default: university_workshop_4k.hdr)
# Output goes next to this script.
set -euo pipefail

input="${1:-university_workshop_4k.hdr}"
out="$(cd "$(dirname "$0")" && pwd)"

gltf-ibl-sampler -inputPath "$input" -outCubeMap "$out/charlie.ktx"    -distribution Charlie    -cubeMapResolution 512
gltf-ibl-sampler -inputPath "$input" -outCubeMap "$out/prefilter.ktx"  -distribution GGX        -cubeMapResolution 512
gltf-ibl-sampler -inputPath "$input" -outCubeMap "$out/irradiance.ktx" -distribution Lambertian -cubeMapResolution 256
