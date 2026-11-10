#pragma once

#include <string>

#define RELATIVE_PATH(val) std::string(PROJ_DIR) + "/" + val
#define SRC_PATH(val) std::string(PROJ_DIR) + "/src/" + val
#define RES_PATH(val) std::string(PROJ_DIR) + "/resources/" + val
