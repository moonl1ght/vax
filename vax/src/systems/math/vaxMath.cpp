#include "vaxMath.h"

namespace vax::math {

bool isBorderIndex(std::vector<int> indices, std::vector<int> dimensions) {
    for (std::vector<int>::size_type i = 0; i < indices.size(); ++i) {
        if (indices[i] == 0 || indices[i] == dimensions[i] - 1) {
            return true;
        }
    }
    return false;
}

} // namespace vax::math