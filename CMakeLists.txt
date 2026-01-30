cmake_minimum_required(VERSION 3.30)

project(factory VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_DIR ${CMAKE_SOURCE_DIR}/libs)

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

#SOURCES
set(SRC
    src/main.cpp
    sigel/SigelEngine.cpp
    sigel/Instance.cpp
    sigel/PhysicalDevice.cpp
    sigel/LogicalDevice.cpp
)

add_executable(${PROJECT_NAME} ${SRC})

#link lib
target_link_libraries(${PROJECT_NAME} PRIVATE 
    VulkanConfig
    glfw
    glm
)

target_include_directories(${PROJECT_NAME} PRIVATE ${LIB_DIR})