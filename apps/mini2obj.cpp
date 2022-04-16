// ======================================================================== //
// Copyright 2018++ Ingo Wald                                               //
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
#include <fstream>

namespace mini {

  void writeToOBJ(Scene::SP scene,
                  const std::string &outFileName)
  {
    SerializedScene serialized(scene.get());
    PRINT(outFileName);
    std::ofstream obj(outFileName);
    std::ofstream mtl(outFileName+".mtl");
    obj << "mtllib " << outFileName << ".mtl" << std::endl;

    std::cout << "#mini2obj: writing " << serialized.materials.size() << " materials" << std::endl;
    for (int matID=0;matID<serialized.materials.size();matID++) {
      mtl << "newmaterial mat_" << matID << std::endl;
      Material::SP mat = serialized.materials[matID];
      if (mat) {
        mtl << "kd "
            << mat->baseColor.x << " "
            << mat->baseColor.y << " "
            << mat->baseColor.z << std::endl;
      }
      mtl << std::endl;
    }

    std::cout << "writing meshes ...." << std::endl;
    for (int instID=0;instID<scene->instances.size();instID++) {
      Instance::SP inst = scene->instances[instID];
        
      if (!inst) continue;

      Object::SP object = inst->object;
      // it's valid to have null instnaces and null objects, but not
      // to have non-nullinstances of null objects...
      assert(object);
      for (int meshID=0;meshID<object->meshes.size();meshID++) {
        Mesh::SP mesh = object->meshes[meshID];
        if (!mesh)
          continue;
        std::cout << "\r# writing inst " << instID << "/" << scene->instances.size()
                  << " mesh " << meshID << "/" << object->meshes.size()
                  << "         " << std::flush;
        // PRINT(mesh->indices.size());
        obj << "o inst_" << instID << "_mesh_" << meshID << std::endl;
        obj << "usemtl mat_" << serialized.getID(mesh->material) << std::endl;

        for (int i=0;i<mesh->vertices.size();i++) {
          vec3f v = xfmPoint(inst->xfm,mesh->vertices[i]);
          obj << "v " << v.x << " " << v.y << " " << v.z << std::endl;
        }

        int v0 = mesh->vertices.size();
        for (int i=0;i<mesh->indices.size();i++) {
          vec3i v = mesh->indices[i];
          obj << "f " << (v.x-v0) << " " << (v.y-v0) << " " << (v.z-v0) << std::endl;
        }
      }
    }
    std::cout << std::endl;
    obj.close();
    mtl.close();
  }
    
  void miniToOBJ(int ac, char **av)
  {
    std::string outFileName = "a.obj";
    std::string inFileName = "";
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg == "-o")
        outFileName = av[++i];
      else if (arg[0] != '-')
        inFileName = arg;
      else
        throw std::runtime_error("unknown cmdline argument '"+arg+"'");
    }
    if (inFileName.empty())
      throw std::runtime_error("no input file specified");

    std::cout << OWL_TERMINAL_LIGHT_BLUE
              << "loading mini file from " << inFileName 
              << OWL_TERMINAL_DEFAULT << std::endl;
    Scene::SP scene = Scene::load(inFileName);
    std::cout << OWL_TERMINAL_LIGHT_GREEN
              << "#mini2obj: scene loaded."
              << OWL_TERMINAL_DEFAULT << std::endl;

    std::cout << OWL_TERMINAL_LIGHT_BLUE
              << "saving to " << outFileName 
              << OWL_TERMINAL_DEFAULT << std::endl;
    writeToOBJ(scene,outFileName);
    std::cout << OWL_TERMINAL_LIGHT_GREEN
              << "#mini2obj: OBJ and MTL files saved."
              << OWL_TERMINAL_DEFAULT << std::endl;
  }
    
} // ::mini

int main(int ac, char **av)
{ mini::miniToOBJ(ac,av); return 0; }

