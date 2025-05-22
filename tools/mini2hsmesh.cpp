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
#include "miniScene/IO.h"
#include <fstream>

namespace mini {

  std::vector<vec3f> vertices;
  std::vector<vec3i> indices;
  
  void writeToHSMESH(Scene::SP scene,
                     const std::string &outFileName)
  {
    std::vector<vec3f> vertices;
    std::vector<vec3i> indices;

    std::cout << "flattening scene into one simple vertices/indices only mesh ..." << std::endl;
    for (auto inst : scene->instances)
      for (auto mesh : inst->object->meshes) {
        int ofs = vertices.size();
        for (auto v : mesh->vertices)
          vertices.push_back(xfmPoint(inst->xfm,v));
        for (auto idx : mesh->indices)
          indices.push_back(idx+ofs);
      }
    
    std::cout << "done. writing to " << outFileName << std::endl;
    std::ofstream out(outFileName,std::ios::binary);

    std::vector<vec3f> noNormals, noColors;
    std::vector<float> noScalars;
    
    io::writeVector(out,vertices);
    io::writeVector(out,noNormals);
    io::writeVector(out,noColors);
    io::writeVector(out,indices);
    io::writeVector(out,noScalars);
  }
    
  void miniToHSMESH(int ac, char **av)
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

    std::cout << MINI_TERMINAL_LIGHT_BLUE
              << "loading mini file from " << inFileName 
              << MINI_TERMINAL_DEFAULT << std::endl;
    Scene::SP scene = Scene::load(inFileName);
    std::cout << MINI_TERMINAL_LIGHT_GREEN
              << "#mini2obj: scene loaded."
              << MINI_TERMINAL_DEFAULT << std::endl;

    std::cout << MINI_TERMINAL_LIGHT_BLUE
              << "saving to " << outFileName 
              << MINI_TERMINAL_DEFAULT << std::endl;
    writeToHSMESH(scene,outFileName);
    std::cout << MINI_TERMINAL_LIGHT_GREEN
              << "#mini2obj: OBJ and MTL files saved."
              << MINI_TERMINAL_DEFAULT << std::endl;
  }
    
} // ::mini

int main(int ac, char **av)
{ mini::miniToHSMESH(ac,av); return 0; }

