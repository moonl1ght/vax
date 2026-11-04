#include "textureGenerator.h"
#include "avxChecker.h"
#include <immintrin.h>

using namespace vax::vk;

std::optional<TextureManager::TextureResource> TextureGenerator::generateCheckerboardTexture(
    const std::string& name, int width, int height, int cellSize
) const noexcept {
    std::vector<uint8_t> pixels(width * height * 4);
    if (isAvxSupported()) {
        uint32_t* pixelData = reinterpret_cast<uint32_t*>(pixels.data());

        __m256i vecWhite = _mm256_set1_epi32(0xFFFFFFFF);
        __m256i vecBlack = _mm256_set1_epi32(0xFF000000);

        for (int y = 0; y < height; ++y) {
            bool yEven = ((y / cellSize) % 2) == 0;
            int x = 0;

            for (; x <= width - 8; x += 8) {
                bool xEven = ((x / cellSize) % 2) == 0;
                bool isWhite = (xEven == yEven);

                _mm256_storeu_si256((__m256i*)(&pixelData[y * width + x]), isWhite ? vecWhite : vecBlack);
            }

            for (; x < width; ++x) {
                bool xEven = ((x / cellSize) % 2) == 0;
                bool isWhite = (xEven == yEven);
                pixelData[y * width + x] = isWhite ? 0xFFFFFFFF : 0xFF000000;
            }
        }
    } else {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                bool isWhite = ((x / cellSize) % 2) == ((y / cellSize) % 2);

                uint8_t color = isWhite ? 255 : 0;

                int index = (y * width + x) * 4;
                pixels[index + 0] = color;
                pixels[index + 1] = color;
                pixels[index + 2] = color;
                pixels[index + 3] = 255;
            }
        }
    }
    return _textureLoader.get().loadRawRGBATexture(name, pixels, width, height);
}