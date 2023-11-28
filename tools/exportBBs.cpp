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
#include <fstream>

namespace mini {

  void miniExportBBs(int ac, char **av)
  {
    std::string inFileName = "";
    std::string outPrefix = "miniExportBBs-";
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg[0] != '-')
        inFileName = arg;
      else if (arg == "-o")
        outPrefix = av[++i];
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
              << "#miniInfo: scene loaded."
              << MINI_TERMINAL_DEFAULT << std::endl;

    std::set<Object::SP> allObjects;
    std::vector<box3f> instBounds;
    for (auto inst : scene->instances)
      if (inst && inst->object) {
        allObjects.insert(inst->object);
        instBounds.push_back(inst->getBounds());
      }
    PRINT(allObjects.size());

    if (scene->instances.size() == 1) {
      std::cout << "model has a single instance; not dumping this and only dumping the mm for the root object..." << std::endl;
    } else{
      std::string instsFileName = outPrefix+"-instances.bb3";
      std::cout << "saving " << prettyNumber(instBounds.size()) << "instances to " << instsFileName << std::endl;
      std::ofstream out(instsFileName.c_str());
      out.write((char*)instBounds.data(),instBounds.size()*sizeof(box3f));
    }

    int numWritten = 0;
    for (auto obj : allObjects) {
      int objID = numWritten++;
      std::string mmFileName
        = (scene->instances.size() == 1)
        ? outPrefix
        : (outPrefix+"-obj"+std::to_string(objID)+".mm");
      std::cout << "saving object's meshes in " << mmFileName << std::endl;
      std::ofstream mm(mmFileName.c_str(),std::ios::binary);
      size_t numMeshes = (int)obj->meshes.size();
      mm.write((char*)&numMeshes,sizeof(numMeshes));
      for (auto mesh : obj->meshes) {
        size_t numVertices = mesh->vertices.size();
        mm.write((char*)&numVertices,sizeof(numVertices));
        mm.write((char*)mesh->vertices.data(),mesh->vertices.size()*sizeof(mesh->vertices[0]));
        size_t numIndices = mesh->indices.size();
        mm.write((char*)&numIndices,sizeof(numIndices));
        mm.write((char*)mesh->indices.data(),mesh->indices.size()*sizeof(mesh->indices[0]));
      }
    }
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::miniExportBBs(ac,av); return 0; }

