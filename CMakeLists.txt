cmake_minimum_required(VERSION 3.9)
project(library VERSION 1.0.0 DESCRIPTION "VoluMol")

option(COMPUTE_SHADERS "Use compute shaders on the GPU, this requires OpenGL 4.3+." OFF)

if (COMPUTE_SHADERS)
	add_definitions(-DUSE_COMPUTE_SHADERS=1)
else(COMPUTE_SHADERS)
	add_definitions(-DUSE_COMPUTE_SHADERS=0)
endif(COMPUTE_SHADERS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_PARALLEL_LEVEL 8)

if (MSVC)
    add_compile_options(/MP)
endif()

include_directories(include)

find_package(OpenGL REQUIRED)

include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
)
FetchContent_MakeAvailable(glfw)
FetchContent_GetProperties(glfw)
if (NOT glfw_POPULATED)
  FetchContent_Populate(glfw)
  add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

add_library(library SHARED src/volumol/Python.cpp)
target_sources(
	library PRIVATE
	
	src/graphics/3D/3D\ Renderer.cpp
	src/graphics/3D/ShadowMap.cpp
	src/graphics/3D/Texture3D.cpp
	src/graphics/Animation.cpp
	src/graphics/BitmapText.cpp
	src/graphics/Blur.cpp
	src/graphics/ComputeShader.cpp
	src/graphics/FrameBuffer.cpp
	src/graphics/GErrorHandler.cpp
	src/graphics/glad.c
	src/graphics/Renderstate.cpp
	src/graphics/Shader.cpp
	src/graphics/Sprite.cpp
	src/graphics/Texture.cpp
	src/graphics/VertexArray.cpp
	src/graphics/Window.cpp
	
	src/logic/Allocator.cpp
	src/logic/Clipboard.cpp
	src/logic/ConsoleUtils.cpp
	src/logic/Input.cpp
	src/logic/MathUtil.cpp
	src/logic/Matrices.cpp
	src/logic/Random.cpp
	src/logic/SpriteSheet.cpp
	src/logic/TextReading.cpp
	
	src/volumol/Isosurface.cpp
	src/volumol/MeshGenerator.cpp
	src/volumol/Molden.cpp
	src/volumol/Molecule.cpp
	src/volumol/MolInterface.cpp
	src/volumol/MolRenderer.cpp
	src/volumol/Orbital.cpp
	src/volumol/Settings.cpp
	src/volumol/TextUtil.cpp
	src/volumol/XYZReader.cpp
	src/volumol/WFXReader.cpp
)

set_target_properties(library PROPERTIES PREFIX "")
set_target_properties(library PROPERTIES OUTPUT_NAME "VoluMol")
target_link_libraries(library PUBLIC glfw)
