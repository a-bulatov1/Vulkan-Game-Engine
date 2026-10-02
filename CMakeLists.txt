cmake_minimum_required(VERSION 3.29)

project(VulkanTutorial)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan 1.4.335 REQUIRED)
#find_package(tinyobjloader REQUIRED)   # uncomment when you reach model loading
#find_package(tinygltf REQUIRED)        # uncomment for the glTF/KTX chapter
#find_package(KTX REQUIRED)

# vulkan_hpp as a plain header (no C++20 modules)
add_library(VulkanCppModule INTERFACE)
add_library(Vulkan::cppm ALIAS VulkanCppModule)
target_link_libraries(VulkanCppModule INTERFACE Vulkan::Vulkan)
target_compile_definitions(VulkanCppModule INTERFACE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)

function(add_slang_shader_target TARGET)
    cmake_parse_arguments("SHADER" "" "CHAPTER_NAME" "SOURCES" ${ARGN})
    set(SHADERS_DIR ${CMAKE_BINARY_DIR}/${SHADER_CHAPTER_NAME}/shaders)

    file(GLOB HAS_COMPUTE ${CHAPTER_SHADER}.comp)
    set(ENTRY_POINTS -entry vertMain -entry fragMain)
    if(HAS_COMPUTE)
        list(APPEND ENTRY_POINTS -entry compMain)
    endif()

    add_custom_command(
        OUTPUT ${SHADERS_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
    )
    add_custom_command(
        OUTPUT ${SHADERS_DIR}/slang.spv
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4+spvRayQueryKHR
                -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
        WORKING_DIRECTORY ${SHADERS_DIR}
        DEPENDS ${SHADER_SOURCES}
        COMMENT "Compiling Slang Shaders for ${TARGET}"
        VERBATIM
    )
    add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

function(add_chapter CHAPTER_NAME)
    cmake_parse_arguments(CHAPTER "" "SHADER" "LIBS;TEXTURES;MODELS" ${ARGN})

    add_executable(${CHAPTER_NAME} ${CHAPTER_NAME}.cpp)
    set_target_properties(${CHAPTER_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CHAPTER_NAME}
        CXX_STANDARD 20)
    target_link_libraries(${CHAPTER_NAME} Vulkan::cppm glfw)
    target_compile_definitions(${CHAPTER_NAME} PRIVATE VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS)

    if(DEFINED CHAPTER_SHADER)
        file(GLOB SHADER_SLANG_SOURCES ${CHAPTER_SHADER}.slang)
        if(SHADER_SLANG_SOURCES)
            set(SHADER_TARGET ${CHAPTER_NAME}_slang_shader)
            add_slang_shader_target(${SHADER_TARGET} CHAPTER_NAME ${CHAPTER_NAME} SOURCES ${SHADER_SLANG_SOURCES})
            add_dependencies(${CHAPTER_NAME} ${SHADER_TARGET})
        endif()
    endif()

    if(DEFINED CHAPTER_LIBS)
        target_link_libraries(${CHAPTER_NAME} ${CHAPTER_LIBS})
    endif()

    if(DEFINED CHAPTER_MODELS)
        list(TRANSFORM CHAPTER_MODELS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/assets/")
        file(COPY ${CHAPTER_MODELS} DESTINATION ${CMAKE_BINARY_DIR}/${CHAPTER_NAME}/models)
    endif()

    if(DEFINED CHAPTER_TEXTURES)
        list(TRANSFORM CHAPTER_TEXTURES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/assets/")
        file(COPY ${CHAPTER_TEXTURES} DESTINATION ${CMAKE_BINARY_DIR}/${CHAPTER_NAME}/textures)
    endif()
endfunction()

add_chapter(engine
    LIBS glm::glm)