#pragma once

#include "logger.h"
#include <cuda_runtime.h>

namespace vax::cuda {

bool check_launch(const char* op, const dev::Logger& logger = dev::Logger("cuda"));

bool sync_device(const dev::Logger& logger = dev::Logger("cuda"));

}