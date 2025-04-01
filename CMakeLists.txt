cmake_minimum_required(VERSION 3.9)
project(library VERSION 1.0.0 DESCRIPTION "VoluMol")

include_directories(include)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

#add_executable(library test.cpp src/graphics/glad.c)
#add_library(library SHARED test.cpp src/graphics/glad.c)

add_library(library SHARED src/volumol/Python.cpp)
target_sources(
	library PRIVATE
	
	src/graphics/3D/3D\ Renderer.cpp
	src/graphics/3D/ShadowMap.cpp
	src/graphics/3D/Texture3D.cpp
	src/graphics/Animation.cpp
	src/graphics/BitmapText.cpp
	src/graphics/Blur.cpp
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
	src/volumol/TextUtil.cpp
	src/volumol/XYZReader.cpp
	src/volumol/WFXReader.cpp
)

set_target_properties(library PROPERTIES PREFIX "")
set_target_properties(library PROPERTIES OUTPUT_NAME "VoluMol")
target_link_libraries(library PUBLIC glfw)
