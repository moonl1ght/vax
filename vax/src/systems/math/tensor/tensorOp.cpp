#include "tensorOp.h"
#include <cassert>
#include <iomanip>
#include <iostream>

using namespace vax::math;

void TensorOp::print(const Tensor& tensor, bool inline_mode) {
    if (inline_mode) {
        for (int i = 0; i < tensor.totalSize(); i++) {
            std::cout << " " << std::fixed << std::setprecision(4) << tensor.data()[i] << " ";
        }
        std::cout << std::endl;
    } else {
        std::cout << "[";
        for (int i = 0; i < tensor.totalSize(); i++) {
            int index = 1;
            for (std::vector<int>::size_type j = 1; j < tensor.shape().size(); j++) {
                index *= tensor.shape()[tensor.shape().size() - j];
                if (i % index == 0) {
                    std::cout << "[";
                }
            }
            std::cout << " " << std::fixed << std::setprecision(4) << tensor.data()[i] << " ";
            index = 1;
            bool was_closed = false;
            for (std::vector<int>::size_type j = 1; j < tensor.shape().size(); j++) {
                index *= tensor.shape()[tensor.shape().size() - j];
                if ((i + 1) % index == 0) {
                    std::cout << "]";
                    was_closed = true;
                }
            }
            if (was_closed && i != tensor.totalSize() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
}

Tensor TensorOp::argmax(const Tensor& tensor, int axis) {
    auto shape = tensor.shape();
    int rank = static_cast<int>(shape.size());

    if (axis == -1 || axis >= rank) {
        float maxValue = -std::numeric_limits<float>::infinity();
        int maxIndex = -1;
        for (int i = 0; i < tensor.totalSize(); ++i) {
            if (tensor._data[i] > maxValue) {
                maxValue = tensor._data[i];
                maxIndex = i;
            }
        }
        Tensor result({1});
        result._data[0] = static_cast<float>(maxIndex);
        return result;
    }

    std::vector<int> outputShape;
    outputShape.reserve(rank - 1);
    for (int i = 0; i < rank; ++i) {
        if (i != axis)
            outputShape.push_back(shape[i]);
    }

    Tensor result(outputShape);
    auto outputStrides = result.strides();

    std::vector<float> maxValues(result.totalSize(), -std::numeric_limits<float>::infinity());

    for (int i = 0; i < tensor.totalSize(); ++i) {
        auto idx = tensor.indices(i);

        int outputFlat = 0;
        int outputStride = 0;
        for (int j = 0; j < rank; ++j) {
            if (j == axis)
                continue;
            outputFlat += idx[j] * outputStrides[outputStride];
            ++outputStride;
        }

        if (tensor._data[i] > maxValues[outputFlat]) {
            maxValues[outputFlat] = tensor._data[i];
            result._data[outputFlat] = static_cast<float>(idx[axis]);
        }
    }

    return result;
}

std::vector<int> TensorOp::maxOverLastDim(const Tensor& tensor, std::vector<int> indices) {
    auto shape = tensor.shape();
    auto lastDim = shape.back();
    int rank = static_cast<int>(shape.size());

    if (indices.size() == rank) {
        return {};
    }
    std::vector<int> indicesToIterate(rank);
    using size_type = std::vector<int>::size_type;
    for (size_type i = 0; i < indices.size(); ++i) {
        indicesToIterate[i] = indices[i];
    }
    float minValue = -std::numeric_limits<float>::infinity();
    auto result = indicesToIterate;
    for (int i = 0; i < lastDim; ++i) {
        indicesToIterate[rank - 1] = i;
        auto value = tensor.get(indicesToIterate);
        if (value.has_value() && value.value() > minValue) {
            minValue = value.value();
            result = indicesToIterate;
        }
    }
    return result;
}

Tensor TensorOp::copy(const Tensor& tensor) {
    Tensor result = Tensor();
    result._shape = tensor.shape();
    result._strides = tensor.strides();
    result._totalSize = tensor.totalSize();
    cudaMallocHost((void**)&result._data, result._totalSize * sizeof(float));;
    std::copy(tensor.begin(), tensor.end(), result.begin());
    return result;
}