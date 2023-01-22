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

  std::string myPretty(size_t n)
  {
    if (n < 1024) return "  "+prettyNumber(n);

    return "  "+prettyNumber(n)+"\t("+std::to_string(n)+")";
  }
  
  Scene::SP separateRootMeshes(Scene::SP in)
  {
    Scene::SP out = Scene::create();

    out->quadLights = in->quadLights;
    out->dirLights = in->dirLights;
    out->envMapLight = in->envMapLight;
    
    std::map<Object::SP,std::vector<Instance::SP>> objects;
    for (auto inst : in->instances) 
      objects[inst->object].push_back(inst);
    
    for (auto it : objects) {
      if (it.second.size() == 1) {
        // object with one parent - split!
        for (auto mesh : it.first->meshes) {
          Object::SP newObj = Object::create();
          newObj->meshes.push_back(mesh);
          out->instances.push_back(Instance::create(newObj));
        }
      } else {
        // object with multiple parents - emit parents
        for (auto inst : it.second)
          out->instances.push_back(inst);
      }
    }
    return out;
  }
  
  void separate(int ac, char **av)
  {
    std::string outFileName = "";
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
    if (outFileName.empty())
      throw std::runtime_error("no output file specified (-o)");

    std::cout << MINI_TERMINAL_LIGHT_BLUE
              << "loading mini file from " << inFileName 
              << MINI_TERMINAL_DEFAULT << std::endl;
    Scene::SP scene = Scene::load(inFileName);
    std::cout << MINI_TERMINAL_LIGHT_GREEN
              << "#miniSeparateRootMeshes: scene loaded."
              << MINI_TERMINAL_DEFAULT << std::endl;

    Scene::SP separated = separateRootMeshes(scene);
    std::cout << MINI_TERMINAL_LIGHT_BLUE
              << "done separating; saving to " << outFileName 
              << MINI_TERMINAL_DEFAULT << std::endl;
    separated->save(outFileName);
    std::cout << MINI_TERMINAL_LIGHT_GREEN
              << "#miniSeparateRootMeshes: scene saved."
              << MINI_TERMINAL_DEFAULT << std::endl;
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::separate(ac,av); return 0; }

