cmake_minimum_required(VERSION 3.18.0)

set(CUDA_PATH /usr/local/cuda CACHE PATH "CUDA toolkit root")
set(CMAKE_CUDA_COMPILER ${CUDA_PATH}/bin/nvcc)
set(CUDA_ARCH sm_120)

project(Luna VERSION 0.1.0 LANGUAGES C CXX CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_STANDARD 20)

string(REGEX REPLACE "^sm_" "" CMAKE_CUDA_ARCHITECTURES ${CUDA_ARCH})

set(CUDA_FLAGS -g -G -O0)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/build/)

find_package(Vulkan REQUIRED FATAL_ERROR)

find_package(assimp REQUIRED FATAL_ERROR)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED FATAL_ERROR)
if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "C:/msys64/mingw64/lib/cmake/SDL3")
    find_package(SDL3 REQUIRED)
else()
    find_package(SDL3 REQUIRED FATAL_ERROR)
endif()
find_package(imgui REQUIRED CONFIG)
find_package(Tracy REQUIRED)
set(Ktx_DIR "${CMAKE_CURRENT_LIST_DIR}/build" CACHE PATH "" FORCE)
find_package(Ktx REQUIRED CONFIG)
find_package(urdfdom REQUIRED)
foreach(_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    if(DEFINED imgui_PACKAGE_FOLDER_${_cfg})
        set(IMGUI_PACKAGE_DIR "${imgui_PACKAGE_FOLDER_${_cfg}}")
        break()
    endif()
endforeach()
if(NOT IMGUI_PACKAGE_DIR)
    message(FATAL_ERROR "Could not determine imgui package folder from Conan variables.")
endif()

if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "VULKAN_SDK is set to: $ENV{VULKAN_SDK}")
else()
    message(WARNING "VULKAN_SDK environment variable is not set.")
endif()

add_compile_definitions(PROJ_DIR="${PROJECT_SOURCE_DIR}")

if(WIN32)
    file(GLOB _SLANG_BIN_HINTS "C:/slang-*/bin")
else()
    file(GLOB _SLANG_BIN_HINTS "/usr/slang/bin")
endif()

find_program(SLANG_COMPILER slangc HINTS
    $ENV{SLANG_DIR}/bin
    ${_SLANG_BIN_HINTS}
    $ENV{VULKAN_SDK}/Bin
    $ENV{VULKAN_SDK}/Bin32
    /usr/slang/bin
    /usr/bin
    /usr/local/bin
)

if(NOT SLANG_COMPILER)
    message(FATAL_ERROR "slangc not found. Install Slang and ensure it is on PATH or set SLANG_DIR.")
endif()
message(STATUS "Found slangc: ${SLANG_COMPILER}")

include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE all_SRCS
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cu"
    "${PROJECT_SOURCE_DIR}/src/*.cuh"
    "${PROJECT_SOURCE_DIR}/src/*.spv"
)

set(STAGE_NAME_vert vertex)
set(STAGE_NAME_frag fragment)
set(STAGE_NAME_comp compute)
set(STAGE_NAME_geom geometry)
set(STAGE_NAME_tesc hull)
set(STAGE_NAME_tese domain)

set(ENTRY_NAME_vert vertMain)
set(ENTRY_NAME_frag fragMain)
set(ENTRY_NAME_comp compMain)
set(ENTRY_NAME_geom geomMain)
set(ENTRY_NAME_tesc tescMain)
set(ENTRY_NAME_tese teseMain)

file(GLOB_RECURSE SLANG_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.vert.slang"
    "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.frag.slang"
    "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.comp.slang"
    "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.geom.slang"
    "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.tesc.slang"
    "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.tese.slang"
)

file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/src/engine/shaders/out")

file(GLOB_RECURSE SHADER_SHARED_FILES "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.h")
file(GLOB SHADER_SHARED_SLANG_FILES "${PROJECT_SOURCE_DIR}/src/engine/shaders/*.slang")
list(REMOVE_ITEM SHADER_SHARED_SLANG_FILES ${SLANG_SOURCE_FILES})
list(APPEND SHADER_SHARED_FILES ${SHADER_SHARED_SLANG_FILES})

foreach(SLANG ${SLANG_SOURCE_FILES})
    string(REGEX MATCH "([^/\\]+)\\.([^.]+)\\.slang$" _ "${SLANG}")
    set(BASE_NAME    "${CMAKE_MATCH_1}")
    set(SUFFIX       "${CMAKE_MATCH_2}")
    set(STAGE        "${STAGE_NAME_${SUFFIX}}")
    set(EP           "${ENTRY_NAME_${SUFFIX}}")
    set(SPIRV        "${PROJECT_SOURCE_DIR}/src/engine/shaders/out/${BASE_NAME}.${SUFFIX}.spv")
    message(STATUS "BUILDING SLANG SHADER: ${BASE_NAME} [${STAGE}]")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/src/engine/shaders/out"
        COMMAND ${SLANG_COMPILER} "${SLANG}"
            -I "${PROJECT_SOURCE_DIR}/src/engine/shaders"
            -entry ${EP}
            -stage ${STAGE}
            -target spirv
            -g -O0
            -o "${SPIRV}"
        DEPENDS ${SLANG} ${SHADER_SHARED_FILES}
        COMMENT "Compiling ${BASE_NAME} [${STAGE}]"
        VERBATIM
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

macro(HEADER_DIRECTORIES return_list)
    file(GLOB_RECURSE new_list "${PROJECT_SOURCE_DIR}/src/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cuh")
    set(dir_list "")

    foreach(file_path ${new_list})
        get_filename_component(dir_path ${file_path} PATH)
        set(dir_list ${dir_list} ${dir_path})
    endforeach()

    list(REMOVE_DUPLICATES dir_list)
    set(${return_list} ${dir_list})
endmacro()

HEADER_DIRECTORIES(header_list)

set(IMGUI_BACKEND_SRCS
    "${IMGUI_PACKAGE_DIR}/res/bindings/imgui_impl_vulkan.cpp"
    "${IMGUI_PACKAGE_DIR}/res/bindings/imgui_impl_sdl3.cpp"
)

add_executable(Luna main.cpp ${all_SRCS} ${IMGUI_BACKEND_SRCS})
target_include_directories(Luna PUBLIC ${header_list})
target_include_directories(Luna PRIVATE ${CUDA_PATH}/include)
if(EXISTS ${CUDA_PATH}/include/cccl)
    target_include_directories(Luna PRIVATE ${CUDA_PATH}/include/cccl)
endif()
set_target_properties(Luna PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
add_dependencies(Luna Shaders)
target_compile_options(Luna PRIVATE -mavx2)

set(all_CUH ${all_SRCS})
list(FILTER all_CUH INCLUDE REGEX "\\.cuh$")
set_source_files_properties(${all_CUH} PROPERTIES HEADER_FILE_ONLY TRUE)

target_compile_options(Luna PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:${CUDA_FLAGS}>)

if(WIN32)
    add_custom_command(TARGET Luna POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/msys64/mingw64/bin/SDL3.dll"
        $<TARGET_FILE_DIR:Luna>/SDL3.dll)
endif()

target_include_directories(Luna PRIVATE
    "${IMGUI_PACKAGE_DIR}/include"
    "${IMGUI_PACKAGE_DIR}/res/bindings"
)

if(WIN32)
    if(VULKAN_FOUND)
        message(STATUS "Found Vulkan. Including and linking...")
        target_link_libraries(
            ${PROJECT_NAME}
            imgui::imgui
            Vulkan::Vulkan
            nlohmann_json::nlohmann_json
            assimp::assimp
            SDL3::SDL3
            glm::glm
            Tracy::TracyClient
            KTX::ktx
            urdfdom::urdfdom
            -L${CUDA_PATH}/lib64 -lcudart
        )
    endif(VULKAN_FOUND)
else()
    if(VULKAN_FOUND)
        message(STATUS "Found Vulkan. Including and linking...")
        target_link_libraries(
            ${PROJECT_NAME}
            imgui::imgui
            Vulkan::Vulkan
            nlohmann_json::nlohmann_json
            assimp::assimp
            SDL3::SDL3
            glm::glm
            Tracy::TracyClient
            KTX::ktx
            urdfdom::urdfdom
            -L${CUDA_PATH}/lib64 -lcudart
        )
    endif(VULKAN_FOUND)
endif()
