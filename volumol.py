import ctypes
import os
import pathlib

__VOLUMOL_PATH = os.path.dirname(__file__).replace("\\", "/") + "/"

__WORKING_PATH = os.getcwd().replace("\\", "/")

__lib = pathlib.Path(__VOLUMOL_PATH + "VoluMol.so")

if (__lib.is_file()):
    __library = ctypes.cdll.LoadLibrary(__VOLUMOL_PATH + "VoluMol.so")
else:
    __lib = pathlib.Path(__VOLUMOL_PATH + "VoluMol.dll")
    if (__lib.is_file()):
        __library = ctypes.cdll.LoadLibrary(__VOLUMOL_PATH + "VoluMol.dll")
    else:
        print("Could not find library!")

__library.pyLoadMoldenFile.argtypes = [ctypes.c_char_p]
__library.pyLoadWFXFile.argtypes = [ctypes.c_char_p]
__library.pyLoadXYZFile.argtypes = [ctypes.c_char_p]
__library.pyGetAtom.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
__library.pyAddBond.argtypes = [ctypes.c_int, ctypes.c_int]
__library.pyRemoveBond.argtypes = [ctypes.c_int, ctypes.c_int]
__library.pySetTransform.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_float)]
__library.pyMOCount.restype = ctypes.c_int
__library.pyGetHOMO.restype = ctypes.c_int
__library.pyGetHOMO.argtypes = [ctypes.c_bool]
__library.pyMOInfo.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_char_p), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_bool)]
__library.pyMOSetOccupation.argtypes = [ctypes.c_int, ctypes.c_float]
__library.pyCubemapResolution.argtypes = [ctypes.c_int]
__library.pyMOCubemap.argtypes = [ctypes.c_int]
__library.pySetCameraOrientation.argtypes = [ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float]
__library.pyGetCameraOrientation.argtypes = [ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
__library.pySetElementProperties.argtypes = [ctypes.c_int, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float]
__library.pyGetElementProperties.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
__library.pyUpdateSettings.argtypes = [ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_bool)]
__library.pySaveImage.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int]
__library.pySetPath.argtypes = [ctypes.c_wchar_p]

__library.pySetPath(ctypes.c_wchar_p(__VOLUMOL_PATH))

class Settings:
    size_factor = 0.2
    bond_thickness = 0.2
    bond_length_tolerance = 0.3
    fov = 70.
    outline_radius = 2.
    ao_intensity = 1.
    ao_radius = 0.5
    ao_exponent = 2.
    cubemap_clearence = 4.
    cubemap_density = 8.
    volumetric_light_distance = 3.
    volumetric_cutoff = 0.00001
    isovalue = 0.02
    isosurface_roughness = 0.5
    isosurface_metallicity = 0.
    volumetric_density = 50.
    brightness = 1.
    z_near = 0.3
    z_far = 300.
    volumetric_gradient = 1.

    ambient_color = (0.4, 0.4, 0.4)
    sun_color = (2., 2., 2.)
    sun_position = (2., 1., 1.)
    mo_color_0 = (1., 0.25, 0.)
    mo_color_1 = (0., 0.4, 1.)
    clear_color = (1., 1., 1.)

    sphere_subdivisions = 3
    cylinder_resolution = 32
    volumetric_iterations = 100
    volumetric_light_iterations = 5
    aa_quality = 1
    cubemap_slice_count = 1
    ao_iterations = 16

    smooth_bonds = False
    premultiply_color = True
    cubemap_use_gpu = True
    orthographic = False
    volumetric_shadowmap = True
    emissive_volume = False
    volumetric_color_mode = False
    multicenter_coordination = False

SPIN_UP = False
SPIN_DOWN = True

class MOInfo:
    energy = 0.
    name = "a0"
    occupation = 0.
    spin = SPIN_UP

    def __init__(self, energy, name, occupation, spin):
        self.energy = energy
        self.name = name
        self.occupation = occupation
        self.spin = spin

class Atom:
    Z = 0
    position = (0., 0., 0.)

    def __init__(self, Z, position):
        self.Z = Z
        self.position = position

def compressVec3(*vectors):
    result = (ctypes.c_float * (3 * len(vectors)))()
    for i in range(len(vectors)):
        result[i * 3    ] = ctypes.c_float(vectors[i][0])
        result[i * 3 + 1] = ctypes.c_float(vectors[i][1])
        result[i * 3 + 2] = ctypes.c_float(vectors[i][2])
    return result

def createWindow():
    __library.pyCreateWindow()
    
def createContext():
    __library.pyCreateContext()

def closeWindow():
    __library.pyCloseWindow()

def dispose():
    __library.pyDispose()

def loadMoldenFile(path):
    __library.pyLoadMoldenFile(path.encode("utf-8"))

def loadWFXFile(path):
    __library.pyLoadWFXFile(path.encode("utf-8"))

def loadXYZFile(path):
    __library.pyLoadXYZFile(path.encode("utf-8"))

def loadCubeFile(path):
    __library.pyLoadCubeFile(path.encode("utf-8"))

def getAtom(id):
    Z = ctypes.c_int()
    x = ctypes.c_float()
    y = ctypes.c_float()
    z = ctypes.c_float()
    __library.pyGetAtom(id, ctypes.byref(Z), ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
    return Atom(Z.value, (x.value, y.value, z.value))

def setTransform(atom0, atom1, atom2, pos0, dir01, dir02):
    vectors = compressVec3(pos0, dir01, dir02)
    __library.pySetTransform(ctypes.c_int(atom0), ctypes.c_int(atom1), ctypes.c_int(atom2), vectors)

def MOCount():
    return __library.pyMOCount()

def getHOMO(spin):
    return __library.pyGetHOMO(spin)

def getMOInfo(orbital):
    energy = ctypes.c_float()
    name = ctypes.c_char_p()
    occupation = ctypes.c_float()
    spin = ctypes.c_bool()
    __library.pyMOInfo(orbital, ctypes.byref(energy), ctypes.pointer(name), ctypes.byref(occupation), ctypes.byref(spin))
    return MOInfo(energy.value, name.value.decode("utf-8"), occupation.value, spin.value)

def setMOOccupation(orbital, occupation):
    __library.pyMOSetOccupation(orbital, occupation)

def setCubemapResolution(resolution):
    __library.pyCubemapResolution(resolution)

def MOCubemap(orbital):
    __library.pyMOCubemap(orbital)

def densityCubemap():
    __library.pyDensityCubemap()

def setIsosurface():
    __library.pySetIsosurface()

def setVolumetric():
    __library.pySetVolumetric()

def setCameraOrientation(position, direction):
    __library.pySetCameraOrientation(ctypes.c_float(position[0]), ctypes.c_float(position[1]), ctypes.c_float(position[2]), ctypes.c_float(direction[0]), ctypes.c_float(direction[1]), ctypes.c_float(direction[2]))

def getCameraOrientation():
    position = (ctypes.c_float * 3)()
    direction = (ctypes.c_float * 3)()
    __library.pyGetCameraOrientation(position, direction)
    return (position[0], position[1], position[2]), (direction[0], direction[1], direction[2])

def setElementProperties(Z, color, roughness, metallic):
    __library.pySetElementProperties(Z, color[0], color[1], color[2], roughness, metallic)

def getElementProperties(Z):
    color = (ctypes.c_float * 3)()
    roughness = ctypes.c_float()
    metallic = ctypes.c_float()
    __library.pyGetElementProperties(Z, color, ctypes.pointer(roughness), ctypes.pointer(metallic))
    return ((color[0], color[1], color[2]), roughness, metallic)

def addBond(a,b):
    __library.pyAddBond(a,b)

def removeBond(a,b):
    __library.pyRemoveBond(a,b)

def updateSettings(settings):
    floats = (ctypes.c_float * 20)(
        settings.size_factor, 
        settings.bond_thickness, 
        settings.bond_length_tolerance,
        settings.fov,
        settings.outline_radius,
        settings.ao_intensity,
        settings.ao_radius,
        settings.ao_exponent,
        settings.cubemap_clearence,
        settings.cubemap_density,
        settings.volumetric_light_distance,
        settings.volumetric_cutoff,
        settings.isovalue,
        settings.isosurface_roughness,
        settings.isosurface_metallicity,
        settings.volumetric_density,
        settings.brightness,
        settings.z_near,
        settings.z_far,
        settings.volumetric_gradient
    )

    vec3s = compressVec3(
        settings.ambient_color,
        settings.sun_color,
        settings.sun_position,
        settings.mo_color_0,
        settings.mo_color_1,
        settings.clear_color
    )

    ints = (ctypes.c_int * 7)()
    ints[0] = settings.sphere_subdivisions
    ints[1] = settings.cylinder_resolution
    ints[2] = settings.volumetric_iterations
    ints[3] = settings.volumetric_light_iterations
    ints[4] = settings.aa_quality
    ints[5] = settings.cubemap_slice_count
    ints[6] = settings.ao_iterations

    bools = (ctypes.c_bool * 8)(
        settings.smooth_bonds,
        settings.premultiply_color,
        settings.cubemap_use_gpu,
        settings.orthographic,
        settings.volumetric_shadowmap,
        settings.emissive_volume,
        settings.volumetric_color_mode,
        settings.multicenter_coordination
    )

    __library.pyUpdateSettings(floats, vec3s, ints, bools)

def launchInterface():
    __library.pyLaunchInterface()

def saveImage(path, width, height):
    __library.pySaveImage(path.encode("utf-8"), width, height)
