// ======================================================================== //
// Copyright 2022++ Ingo Wald                                               //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
//#include <pybind11/eigen.h>  // Optional, if you need Eigen support
#include <memory>
#include <string>
#include <fstream>

#include "miniScene/Scene.h"

namespace py = pybind11;
using namespace mini;

PYBIND11_MAKE_OPAQUE(std::vector<vec3f>);
PYBIND11_MAKE_OPAQUE(std::vector<vec2f>);
PYBIND11_MAKE_OPAQUE(std::vector<vec3i>);

PYBIND11_MODULE(pyminiScene, m) {
	// Owl.h
	py::class_<vec3f>(m, "vec3f")
		.def(py::init<float, float, float>(), py::arg("x"), py::arg("y"), py::arg("z"))  // Constructor with values
		.def_readwrite("x", &vec3f::x)
		.def_readwrite("y", &vec3f::y)
		.def_readwrite("z", &vec3f::z);

	py::class_<vec2f>(m, "vec2f")
		.def(py::init<float, float>(), py::arg("x"), py::arg("y"))  // Constructor with values
		.def_readwrite("x", &vec2f::x)
		.def_readwrite("y", &vec2f::y);

	py::class_<vec3i>(m, "vec3i")
		.def(py::init<int, int, int>(), py::arg("x"), py::arg("y"), py::arg("z"))  // Constructor with values
		.def_readwrite("x", &vec3i::x)
		.def_readwrite("y", &vec3i::y)
		.def_readwrite("z", &vec3i::z);

	////https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html#stl-containers
	//py::class_<std::vector<vec3f>>(m, "arrayVec3f")
	//	.def(py::init<>())
	//	.def("clear", &std::vector<vec3f>::clear)
	//	.def("pop_back", &std::vector<vec3f>::pop_back)
	//	.def("push_back", (void (std::vector<vec3f>::*)(const vec3f&)) & std::vector<vec3f>::push_back)
	//	.def("__len__", [](const std::vector<vec3f>& v) { return v.size(); })
	//	.def("__iter__", [](std::vector<vec3f>& v) { return py::make_iterator(v.begin(), v.end()); }, py::keep_alive<0, 1>());
	py::bind_vector<std::vector<vec3f>>(m, "arrayVec3f");

	////https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html#stl-containers
	//py::class_<std::vector<vec3i>>(m, "arrayVec3i")
	//	.def(py::init<>())
	//	.def("clear", &std::vector<vec3i>::clear)
	//	.def("pop_back", &std::vector<vec3i>::pop_back)
	//	.def("push_back", (void (std::vector<vec3i>::*)(const vec3i&)) & std::vector<vec3i>::push_back)
	//	.def("__len__", [](const std::vector<vec3i>& v) { return v.size(); })
	//	.def("__iter__", [](std::vector<vec3i>& v) { return py::make_iterator(v.begin(), v.end()); }, py::keep_alive<0, 1>());
	py::bind_vector<std::vector<vec3i>>(m, "arrayVec3i");

	////https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html#stl-containers
	//py::class_<std::vector<vec2f>>(m, "arrayVec2f")
	//	.def(py::init<>())
	//	.def("clear", &std::vector<vec2f>::clear)
	//	.def("pop_back", &std::vector<vec2f>::pop_back)
	//	.def("push_back", (void (std::vector<vec2f>::*)(const vec2f&)) & std::vector<vec2f>::push_back)
	//	.def("__len__", [](const std::vector<vec2f>& v) { return v.size(); })
	//	.def("__iter__", [](std::vector<vec2f>& v) { return py::make_iterator(v.begin(), v.end()); }, py::keep_alive<0, 1>());
	py::bind_vector<std::vector<vec2f>>(m, "arrayVec2f");

	py::class_<LinearSpace3<vec3f>>(m, "LinearSpace3f")
		.def(py::init<>())  // Default constructor
		.def(py::init<const vec3f&, const vec3f&, const vec3f&>(), py::arg("vx"), py::arg("vy"), py::arg("vz"))  // Constructor with 3 vectors
		.def("inverse", &LinearSpace3<vec3f>::inverse)  // Bind the inverse function
		.def("det", &LinearSpace3<vec3f>::det)  // Bind the determinant function
		.def_readwrite("vx", &LinearSpace3<vec3f>::vx)
		.def_readwrite("vy", &LinearSpace3<vec3f>::vy)
		.def_readwrite("vz", &LinearSpace3<vec3f>::vz);

	py::class_<AffineSpaceT<LinearSpace3<vec3f>>>(m, "AffineSpace3f")
		.def(py::init<>())  // Default constructor
		.def(py::init<const LinearSpace3<vec3f>&, const vec3f&>(), py::arg("linear_part"), py::arg("translation"))  // Constructor with linear and translation
		.def_static("scale", &AffineSpaceT<LinearSpace3<vec3f>>::scale, py::arg("scale"))  // Static scale method
		.def_static("translate", &AffineSpaceT<LinearSpace3<vec3f>>::translate, py::arg("translation"))  // Static translate method
		//.def_static("rotate_2D", py::overload_cast<const vec3f::scalar_t&>(&AffineSpaceT<LinearSpace3<vec3f>>::rotate), py::arg("r"), "Rotate in 2D with a scalar.")
		.def_static("rotate_around_axis", py::overload_cast<const vec3f&, const vec3f::scalar_t&>(&AffineSpaceT<LinearSpace3<vec3f>>::rotate), py::arg("axis"), py::arg("angle"), "Rotate in 3D around an axis.")
		.def_static("rotate_around_point", py::overload_cast<const vec3f&, const vec3f&, const vec3f::scalar_t&>(&AffineSpaceT<LinearSpace3<vec3f>>::rotate), py::arg("point"), py::arg("axis"), py::arg("angle"), "Rotate around a point and axis in 3D.")
		.def_readwrite("l", &AffineSpaceT<LinearSpace3<vec3f>>::l)  // Expose the linear part
		.def_readwrite("p", &AffineSpaceT<LinearSpace3<vec3f>>::p);  // Expose the translation part

	// Scene.h
	py::class_<Texture, Texture::SP>(m, "Texture")
		.def(py::init<>())  // Constructor
		.def_readwrite("size", &Texture::size)
		.def_readwrite("format", &Texture::format)
		.def_readwrite("filterMode", &Texture::filterMode)
		.def_readwrite("data", &Texture::data)
		.def_static("create", &Texture::create);  // Static create function

	py::enum_<Texture::Format>(m, "Format")
		.value("UNDEFINED", Texture::Format::UNDEFINED)
		.value("EMBEDDED_PTEX", Texture::Format::EMBEDDED_PTEX)
		.value("FLOAT4", Texture::Format::FLOAT4)
		.value("FLOAT1", Texture::Format::FLOAT1)
		.value("RGBA_UINT8", Texture::Format::RGBA_UINT8)
		.export_values();

	py::enum_<Texture::FilterMode>(m, "FilterMode")
		.value("FILTER_BILINEAR", Texture::FILTER_BILINEAR)
		.value("FILTER_NEAREST", Texture::FILTER_NEAREST)
		.export_values();

	// Expose Material class, but prevent instantiation since it's abstract
	py::class_<Material, Material::SP>(m, "Material")
		.def("toString", &Material::toString)
		.def("write", &Material::write)
		.def("read", &Material::read)
		.def("clone", &Material::clone);

	py::class_<BlenderMaterial, Material, BlenderMaterial::SP>(m, "BlenderMaterial")
		.def(py::init<>())  // Default constructor
		.def_static("create", &BlenderMaterial::create)
		.def("clone", &BlenderMaterial::clone)
		.def_readwrite("baseColor", &BlenderMaterial::baseColor)
		.def_readwrite("roughness", &BlenderMaterial::roughness)
		.def_readwrite("metallic", &BlenderMaterial::metallic)
		.def_readwrite("specular", &BlenderMaterial::specular)
		.def_readwrite("specularTint", &BlenderMaterial::specularTint)
		.def_readwrite("transmission", &BlenderMaterial::transmission)
		.def_readwrite("transmissionRoughness", &BlenderMaterial::transmissionRoughness)
		.def_readwrite("ior", &BlenderMaterial::ior)
		.def_readwrite("alpha", &BlenderMaterial::alpha)
		.def_readwrite("subsurfaceRadius", &BlenderMaterial::subsurfaceRadius)
		.def_readwrite("subsurfaceColor", &BlenderMaterial::subsurfaceColor)
		.def_readwrite("subsurface", &BlenderMaterial::subsurface)
		.def_readwrite("anisotropic", &BlenderMaterial::anisotropic)
		.def_readwrite("anisotropicRotation", &BlenderMaterial::anisotropicRotation)
		.def_readwrite("sheen", &BlenderMaterial::sheen)
		.def_readwrite("sheenTint", &BlenderMaterial::sheenTint)
		.def_readwrite("clearcoat", &BlenderMaterial::clearcoat)
		.def_readwrite("clearcoatRoughness", &BlenderMaterial::clearcoatRoughness)
		.def_readwrite("baseColorTexture", &BlenderMaterial::baseColorTexture)
		.def_readwrite("alphaTexture", &BlenderMaterial::alphaTexture);

	py::class_<DisneyMaterial, Material, DisneyMaterial::SP>(m, "DisneyMaterial")
		.def(py::init<>())  // Default constructor
		.def_static("create", &DisneyMaterial::create)
		.def("clone", &DisneyMaterial::clone)
		.def_readwrite("emission", &DisneyMaterial::emission)
		.def_readwrite("baseColor", &DisneyMaterial::baseColor)
		.def_readwrite("metallic", &DisneyMaterial::metallic)
		.def_readwrite("roughness", &DisneyMaterial::roughness)
		.def_readwrite("transmission", &DisneyMaterial::transmission)
		.def_readwrite("ior", &DisneyMaterial::ior)
		.def_readwrite("colorTexture", &DisneyMaterial::colorTexture)
		.def_readwrite("alphaTexture", &DisneyMaterial::alphaTexture);

	py::class_<Mesh, Mesh::SP>(m, "Mesh")
		.def(py::init<Material::SP>(), py::arg("material") = Material::SP())
		.def_static("create", &Mesh::create, py::arg("material") = Material::SP())
		.def_readwrite("vertices", &Mesh::vertices)
		.def_readwrite("normals", &Mesh::normals)
		.def_readwrite("texcoords", &Mesh::texcoords)
		.def_readwrite("indices", &Mesh::indices)
		.def_readwrite("material", &Mesh::material)
		.def("getBounds", &Mesh::getBounds, "Computes and returns the world-space bounding box")
		.def("getNumPrims", &Mesh::getNumPrims);

	py::class_<Object, Object::SP>(m, "Object")
		.def(py::init<const std::vector<Mesh::SP>&>(), py::arg("meshes") = std::vector<Mesh::SP>())
		.def_static("create", &Object::create, py::arg("meshes") = std::vector<Mesh::SP>())
		.def("getBounds", &Object::getBounds, "Computes and returns the world-space bounding box")
		.def_readwrite("meshes", &Object::meshes);

	py::class_<Instance, Instance::SP>(m, "Instance")
		.def(py::init<Object::SP, const affine3f&>(), py::arg("object") = Object::SP(), py::arg("xfm") = affine3f())
		.def_static("create", &Instance::create, py::arg("object") = Object::SP(), py::arg("xfm") = affine3f())
		.def("getBounds", &Instance::getBounds, "Computes and returns the world-space bounding box")
		.def_readwrite("xfm", &Instance::xfm)
		.def_readwrite("object", &Instance::object);		

	py::class_<QuadLight>(m, "QuadLight")
		.def_readwrite("corner", &QuadLight::corner)
		.def_readwrite("edge0", &QuadLight::edge0)
		.def_readwrite("edge1", &QuadLight::edge1)
		.def_readwrite("emission", &QuadLight::emission)
		.def_readwrite("normal", &QuadLight::normal)
		.def_readwrite("area", &QuadLight::area);

	py::class_<DirLight>(m, "DirLight")
		.def_readwrite("direction", &DirLight::direction)
		.def_readwrite("radiance", &DirLight::radiance);

	py::class_<EnvMapLight, EnvMapLight::SP>(m, "EnvMapLight")
		.def(py::init<>())  // Constructor
		.def_static("create", &EnvMapLight::create)
		.def_readwrite("texture", &EnvMapLight::texture)
		.def_readwrite("transform", &EnvMapLight::transform);

	//py::class_<Scene, std::shared_ptr<Scene>>(m, "Scene")
	//	.def(py::init<const std::vector<Instance::SP>&>(), py::arg("instances") = std::vector<Instance::SP>())
	//	.def_static("create", &Scene::create, py::arg("instances") = std::vector<Instance::SP>())
	//	.def_readwrite("instances", &Scene::instances)
	//	.def_readwrite("quadLights", &Scene::quadLights)
	//	.def_readwrite("dirLights", &Scene::dirLights)
	//	.def_readwrite("envMapLight", &Scene::envMapLight);

	// Bindings for Scene class
	py::class_<Scene, Scene::SP>(m, "Scene")
		.def(py::init<const std::vector<Instance::SP>&>(), py::arg("instances") = std::vector<Instance::SP>())
		.def_static("create", &Scene::create, py::arg("instances") = std::vector<Instance::SP>(), "Create a new Scene object")
		.def("getBounds", &Scene::getBounds, "Compute and return the world space bounding box of the scene")
		.def_static("load", &Scene::load, py::arg("fileName"), "Load the scene from a .mini file")
		.def("save", &Scene::save, py::arg("fileName"), "Save the scene to a file with the given name")
		.def_readwrite("instances", &Scene::instances, "List of instances in the scene")
		.def_readwrite("quadLights", &Scene::quadLights, "List of quad lights in the scene")
		.def_readwrite("dirLights", &Scene::dirLights, "List of directional lights in the scene")
		.def_readwrite("envMapLight", &Scene::envMapLight, "Environment map light in the scene");
}