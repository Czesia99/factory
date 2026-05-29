cmake_minimum_required(VERSION 3.30)

project(factory VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_DIR ${CMAKE_SOURCE_DIR}/libs)

include_directories(${LIB_DIR}/stb)

#Vulkan
find_package(Vulkan REQUIRED)

add_library(VulkanConfig INTERFACE)
target_compile_definitions(VulkanConfig INTERFACE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    VULKAN_HPP_NO_SPACESHIP_OPERATOR=1
)
target_link_libraries(VulkanConfig INTERFACE Vulkan::Vulkan)

#GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${LIB_DIR}/glfw-3.4)

#GLM
add_subdirectory(${LIB_DIR}/glm)

#VMA
add_subdirectory(${LIB_DIR}/vma)

#SOURCES
set(SRC
    src/main.cpp
    sigel/stb.cpp
    sigel/vkapi/vma.cpp
    sigel/SigelEngine.cpp
    sigel/vkapi/Instance.cpp
    sigel/vkapi/Device.cpp
    sigel/vkapi/GpuAllocator.cpp
    sigel/vkapi/WindowSurface.cpp
    sigel/vkapi/Swapchain.cpp
    sigel/vkapi/Pipeline.cpp
    sigel/vkapi/ResourceManager.cpp
    sigel/vkapi/Renderer.cpp
    sigel/Camera.cpp
    sigel/Scene.cpp
    src/TestScene.cpp
)

# file(GLOB SRC_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/sigel/shaders/*.slang")
# set(SRC_SHADERS
#     "${CMAKE_CURRENT_SOURCE_DIR}/sigel/shaders/shader.slang"
# )

# function (add_slang_shader_target TARGET)
#   cmake_parse_arguments ("SHADER" "" "" "SOURCES" ${ARGN})
#   set (SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/sigel/shaders)
#   set (ENTRY_POINTS -entry vertMain -entry fragMain)
#   add_custom_command (
#           OUTPUT ${SHADERS_DIR}
#           COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
#   )
#   add_custom_command (
#           OUTPUT  ${SHADERS_DIR}/slang.spv
#           COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
#           WORKING_DIRECTORY ${SHADERS_DIR}
#           DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
#           COMMENT "Compiling Slang Shaders"
#           VERBATIM
#   )
#   add_custom_target (${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
# endfunction()

# add_slang_shader_target(foo SOURCES ${SRC_SHADERS})

add_executable(${PROJECT_NAME} ${SRC})
# add_dependencies(${PROJECT_NAME} foo)

#link lib
target_link_libraries(${PROJECT_NAME} PRIVATE 
    VulkanConfig
    glfw
    glm
    GPUOpen::VulkanMemoryAllocator
)

target_include_directories(${PROJECT_NAME} PRIVATE ${LIB_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY         ${CMAKE_SOURCE_DIR}/build
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/build
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build
)