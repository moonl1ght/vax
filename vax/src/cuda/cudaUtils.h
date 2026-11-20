#pragma once

#include "logger.h"
#include <cuda_runtime.h>

namespace vax::cuda {

bool check_launch(const char* op, const vax::Logger& logger = vax::Logger("cuda"));

bool sync_device(const vax::Logger& logger = vax::Logger("cuda"));

}