# rm -rf build/
# cmake --preset debug
# cmake --build --preset debug --target run

cmake_minimum_required(VERSION 3.29)

project(VBGE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan 1.4.335 REQUIRED)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "" FORCE)

FetchContent_Declare(tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG v3.1.0
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(tinygltf)

set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_LOADTEST_APPS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(ktx
    GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software.git
    GIT_TAG v4.4.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(ktx)

find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)

# --- Executable ---
add_executable(engine engine.cpp)
set_target_properties(engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/engine
    CXX_STANDARD 20)

target_include_directories(engine SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libraries ${tinygltf_SOURCE_DIR})

target_link_libraries(engine PRIVATE Vulkan::Vulkan glfw glm::glm ktx)
target_compile_definitions(engine PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
)

# --- Shaders (Slang) ---
set(SHADERS_DIR ${CMAKE_BINARY_DIR}/engine/shaders)
file(GLOB SLANG_SHADER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.slang)

if(SLANG_SHADER_SOURCES)
    add_custom_command(
        OUTPUT ${SHADERS_DIR}/slang.spv
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
        COMMAND ${SLANGC_EXECUTABLE} ${SLANG_SHADER_SOURCES} -target spirv -profile spirv_1_4+spvRayQueryKHR
                -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain
                -o ${SHADERS_DIR}/slang.spv
        WORKING_DIRECTORY ${SHADERS_DIR}
        DEPENDS ${SLANG_SHADER_SOURCES}
        COMMENT "Compiling Slang shaders"
        VERBATIM
    )
    add_custom_target(engine_shaders DEPENDS ${SHADERS_DIR}/slang.spv)
    add_dependencies(engine engine_shaders)
endif()

# --- Assets ---
file(GLOB MODEL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/models/*.obj)
if(MODEL_FILES)
    file(COPY ${MODEL_FILES} DESTINATION ${CMAKE_BINARY_DIR}/engine/models)
endif()

file(GLOB TEXTURE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/textures/*.png
    ${CMAKE_CURRENT_SOURCE_DIR}/textures/*.jpg)
if(TEXTURE_FILES)
    file(COPY ${TEXTURE_FILES} DESTINATION ${CMAKE_BINARY_DIR}/engine/textures)
endif()

message(STATUS "Slang sources: ${SLANG_SHADER_SOURCES}")

add_custom_target(run
    COMMAND engine
    DEPENDS engine
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/engine
    USES_TERMINAL
)