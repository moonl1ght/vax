#pragma once

#include "tensor.h"

namespace vax::math {
class TensorOp final {
  public:
    static void print(const Tensor& tensor, bool inline_mode = false);

    static Tensor argmax(const Tensor& tensor, int axis = -1);

    static std::vector<int> maxOverLastDim(const Tensor& tensor, std::vector<int> indices);

    static Tensor copy(const Tensor& tensor);
};

class TensorOpGpu final {
  public:
    static bool add(Tensor& A, const Tensor& B);

    static bool add(const Tensor& A, const Tensor& B, Tensor& C);
};
} // namespace vax::math