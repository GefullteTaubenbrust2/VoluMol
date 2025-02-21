#include "MolInterface.h"
#include "Molecule.h"
#include "Molden.h"
#include "WFXReader.h"
#include "Orbital.h"
#include "MolRenderer.h"
#include "XYZReader.h"
#include "../logic/TextReading.h"
#include "../graphics/Window.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT extern "C" 
#endif

extern std::string executable_path;

namespace mol {
	extern mol::RenderProperties settings;

	namespace Renderer {
		extern glm::vec3 camera_position, camera_direction;
	}
}

glm::vec3 vec3FromFloats(float* floats, uint offset) {
	return glm::vec3(floats[3 * offset], floats[1 + 3 * offset], floats[2 + 3 * offset]);
}

DLLEXPORT void pySetPath(wchar_t const* path) {
	std::wstring str = path;
	executable_path = std::string(str.begin(), str.end());
}

DLLEXPORT void pyCreateWindow() {
	mol::createWindow();
}

DLLEXPORT void pyCreateContext() {
	mol::createContext();
}

DLLEXPORT void pyCloseWindow() {
	fgr::window::close();
}

DLLEXPORT void pyDispose() {
	fgr::window::checkEvents();
	fgr::window::flush();
	fgr::window::dispose();
}

DLLEXPORT void pyLoadMoldenFile(char const* path) {
	mol::Molden::loadFile(path);
}

DLLEXPORT void pyLoadWFXFile(char const* path) {
	mol::WFX::loadFile(path);
}

DLLEXPORT void pyLoadXYZFile(char const* path) {
	mol::Renderer::setMolecule(mol::readXYZ(flo::readFile(path)));
}

DLLEXPORT void pyGetAtom(int id, int& Z, float& x, float& y, float& z) {
	mol::Atom atom = mol::Renderer::getAtom(id);
	Z = atom.Z;
	x = atom.position.x;
	y = atom.position.y;
	z = atom.position.z;
}

DLLEXPORT void pySetTransform(int id0, int id1, int id2, float* vectors) {
	glm::mat4 transform = mol::Renderer::getTransform(id0, id1, id2, vec3FromFloats(vectors, 0), vec3FromFloats(vectors, 1), vec3FromFloats(vectors, 2));
	mol::Renderer::setTransform(transform);
}

DLLEXPORT int pyMOCount() {
	return mol::MOcount();
}

DLLEXPORT int pyGetLUMO() {
	return mol::findLUMO();
}

DLLEXPORT void pyMOInfo(int orbital, float& energy, char const** name, float& occupation, bool& spin) {
	mol::MolecularOrbital& mo = mol::getMO(orbital);
	energy = mo.energy;
	*name = mo.name.c_str();
	occupation = mo.occupation;
	spin = mo.spin == mol::Spin::beta;
}

DLLEXPORT void pyMOSetOccupation(int orbital, float occupation) {
	mol::MolecularOrbital& mo = mol::getMO(orbital);
	mo.occupation = occupation;
}

DLLEXPORT void pyCubemapResolution(int x) {
	mol::resizeCubeMap(x, x, x);
}

DLLEXPORT void pyMOCubemap(int orbital) {
	mol::MOCubeMap(orbital);
}

DLLEXPORT void pyDensityCubemap() {
	mol::densityCubeMapMO();
}

DLLEXPORT void pySetIsosurface() {
	mol::Renderer::setIsosurface();
}

DLLEXPORT void pySetVolumetric() {
	mol::Renderer::setVolumetric();
}

DLLEXPORT void pySetCameraOrientation(float px, float py, float pz, float dx, float dy, float dz) {
	mol::Renderer::orientCamera(glm::vec3(px, py, pz), glm::vec3(dx, dy, dz));
}

DLLEXPORT void pyGetCameraOrientation(float* position, float* direction) {
	position[0] = mol::Renderer::camera_position.x;
	position[1] = mol::Renderer::camera_position.y;
	position[2] = mol::Renderer::camera_position.z;
	direction[0] = mol::Renderer::camera_direction.x;
	direction[1] = mol::Renderer::camera_direction.y;
	direction[2] = mol::Renderer::camera_direction.z;
}

DLLEXPORT void pyElementProperties(int Z, float r, float g, float b, float roughness, float metallicity) {
	mol::settings.materials[Z].color = glm::vec3(r, g, b);
	mol::settings.materials[Z].roughness = roughness;
	mol::settings.materials[Z].metallicity = metallicity;
}

DLLEXPORT void pyUpdateSettings(float* floats, float* vectors, int* ints, bool* bools) {
	mol::RenderProperties settings;
	
	for (int i = 0; i < 119; ++i)
		settings.materials[i] = mol::settings.materials[i];

	settings.size_factor					= floats[0];
	settings.bond_thickness					= floats[1];
	settings.bond_length_tolerance			= floats[2];
	settings.fov							= floats[3];
	settings.outline_radius					= floats[4];
	settings.ao_intensity					= floats[5];
	settings.ao_radius						= floats[6];
	settings.ao_exponent					= floats[7];
	settings.cubemap_clearance				= floats[8];
	settings.cubemap_density				= floats[9];
	settings.volumetric_light_distance		= floats[10];
	settings.volumetric_cutoff				= floats[11];
	settings.isovalue						= floats[12];
	settings.isosurface_roughness			= floats[13];
	settings.isosurface_metallicity			= floats[14];
	settings.volumetric_density				= floats[15];
	settings.brightness						= floats[16];
	settings.z_near							= floats[17];
	settings.z_far							= floats[18];

	settings.ambient_color					= vec3FromFloats(vectors, 0);
	settings.sun_color						= vec3FromFloats(vectors, 1);
	settings.sun_position					= vec3FromFloats(vectors, 2);
	settings.mo_colors[0]					= vec3FromFloats(vectors, 3);
	settings.mo_colors[1]					= vec3FromFloats(vectors, 4);
	settings.clear_color					= vec3FromFloats(vectors, 5);

	settings.sphere_subdivisions			= ints[0];
	settings.cylinder_resolution			= ints[1];
	settings.volumetric_iterations			= ints[2];
	settings.volumetric_light_iterations	= ints[3];
	settings.taa_quality					= ints[4];
	settings.cubemap_thread_count			= ints[5];
	settings.ao_iterations					= ints[6];

	settings.smooth_bonds					= bools[0];
	settings.premulitply_color				= bools[1];
	settings.cubemap_use_gpu				= bools[2];
	settings.orthographic					= bools[3];

	mol::Renderer::updateSettings(settings);
}

DLLEXPORT void pyLaunchInterface() {
	mol::interactiveInterface();
}

DLLEXPORT void pySaveImage(char const* path, int width, int height) {
	mol::Renderer::saveImage(path, width, height);
}
