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

#include "miniScene/Scene.h"
#include "miniScene/Serialized.h"

namespace mini {

  void checkFishy(const float &f, const std::string &where)
  {
    if (isnan(f) || isinf(f) || isinf(fabsf(f)))
      std::cout << MINI_TERMINAL_RED
                << "fishy float " << f << " in " << where
                << MINI_TERMINAL_DEFAULT
                << std::endl;
  }
  
  void checkFishy(const vec3f &v, const std::string &where)
  {
    checkFishy(v.x,where);
    checkFishy(v.y,where);
    checkFishy(v.z,where);
  }
  
  void checkFishy(const vec2f &v, const std::string &where)
  {
    checkFishy(v.x,where);
    checkFishy(v.y,where);
  }
  
  void checkFishy(const affine3f &xfm, const std::string &where)
  {
    checkFishy(xfm.l.vx,where);
    checkFishy(xfm.l.vy,where);
    checkFishy(xfm.l.vz,where);
    checkFishy(xfm.p,where);
  }
  
  void checkFishy(Material::SP _material, const std::string &where)
  {
    DisneyMaterial::SP material = _material->as<DisneyMaterial>();
    if (!material) return;
    checkFishy(material->emission,where+".emission");
    checkFishy(material->baseColor,where+".baseColor");
    checkFishy(material->metallic,where+".metallic");
    checkFishy(material->roughness,where+".roughness");
    checkFishy(material->transmission,where+".transmission");
    checkFishy(material->ior,where+".ior");
  }

  void checkRange(int idx, size_t numVertices, size_t numNormals, size_t numTexcoords)
  {
    if (idx < 0)
      throw std::runtime_error("NEGATIVE vertex index!");

    if (idx >= numVertices)
      throw std::runtime_error("vertex index "+std::to_string(idx)
                               +", but only have "+std::to_string(numVertices)+" vertices!");
    if (numNormals && idx >= numNormals)
      throw std::runtime_error("vertex index "+std::to_string(idx)
                               +", but only have "+std::to_string(numVertices)+" NORMALS!");
    if (numTexcoords && idx >= numTexcoords)
      throw std::runtime_error("vertex index "+std::to_string(idx)
                               +", but only have "+std::to_string(numVertices)+" TEXCOORDS!");
      
  }
  
  void checkRange(const vec3i &idx, size_t numVertices, size_t numNormals, size_t numTexcoords)
  {
    checkRange(idx.x, numVertices, numNormals, numTexcoords);
    checkRange(idx.y, numVertices, numNormals, numTexcoords);
    checkRange(idx.z, numVertices, numNormals, numTexcoords);
  }
    
  void lint(Scene::SP scene)
  {
    if (scene->instances.empty())
      std::cout << MINI_TERMINAL_RED
                << "suspicious - scene is completely empty!?"
                << MINI_TERMINAL_DEFAULT << std::endl;

    std::set<Object::SP> objects;
    for (auto inst : scene->instances) {
      if (!inst->object)
        throw std::runtime_error("instance without object!");
      checkFishy(inst->xfm,"instance transform");
      objects.insert(inst->object);
    }

    for (auto obj : objects) {
      if (obj->meshes.empty())
        throw std::runtime_error("object without any meshes!");
      for (auto mesh : obj->meshes) {
        if (!mesh->material)
          throw std::runtime_error("mesh without material!");
        checkFishy(mesh->material,"material");

        if (mesh->vertices.empty())
          throw std::runtime_error("mesh without any vertices!");
        if (mesh->indices.empty())
          throw std::runtime_error("mesh without any indices!");
          
        for (auto vtx : mesh->vertices)
          checkFishy(vtx,"vertex");
        for (auto tex : mesh->texcoords)
          checkFishy(tex,"texcoord");
        for (auto nor : mesh->normals)
          checkFishy(nor,"vertex normal");
        for (auto idx : mesh->indices) {
          checkRange(idx,mesh->vertices.size(),mesh->normals.size(),mesh->texcoords.size());
          vec3f v0 = mesh->vertices[idx.x];
          vec3f v1 = mesh->vertices[idx.y];
          vec3f v2 = mesh->vertices[idx.z];
          if (v0 == v1 || v0 == v2 || v1 == v2) {
            static size_t count = 0;
            std::cout << MINI_TERMINAL_RED
                      << "fishy triangle " << v0 << " " << v1 << " " << v2
                      << " (" << ++count << " such occurrances)"
                      << MINI_TERMINAL_DEFAULT
                      << std::endl;
          }
        }
      }
    }
  }
    
  void miniLint(int ac, char **av)
  {
    std::string inFileName = "";
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg[0] != '-')
        inFileName = arg;
      else
        throw std::runtime_error("unknown cmdline argument '"+arg+"'");
    }
    if (inFileName.empty())
      throw std::runtime_error("no input file specified");

    std::cout << MINI_TERMINAL_LIGHT_BLUE
              << "loading mini file from " << inFileName 
              << MINI_TERMINAL_DEFAULT << std::endl;
    Scene::SP scene = Scene::load(inFileName);
    std::cout << MINI_TERMINAL_LIGHT_GREEN
              << "#miniLint: scene loaded."
              << MINI_TERMINAL_DEFAULT << std::endl;

    lint(scene);
    std::cout << MINI_TERMINAL_GREEN
              << "#miniLint: done checking for NaNs, infs, missing pointers... all good."
              << MINI_TERMINAL_DEFAULT << std::endl;
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::miniLint(ac,av); return 0; }

