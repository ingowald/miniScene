// ======================================================================== //
// Copyright 2025++ Ingo Wald                                               //
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

  void exportIndividualInstances(int ac, char **av)
  {
    std::string outFileBase = "";
    std::string inFileName = "";
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg == "-o")
        outFileBase = av[++i];
      else if (arg[0] != '-')
        inFileName = arg;
      else
        throw std::runtime_error("unknown cmdline argument '"+arg+"'");
    }
    if (inFileName.empty())
      throw std::runtime_error("no input file specified");
    if (outFileName.empty())
      throw std::runtime_error("no output prefix specified (-o)");

    std::cout << MINI_TERMINAL_LIGHT_BLUE
              << "loading mini file from " << inFileName 
              << MINI_TERMINAL_DEFAULT << std::endl;
    Scene::SP scene = Scene::load(inFileName);
    std::cout << MINI_TERMINAL_LIGHT_GREEN
              << "#miniSeparateRootMeshes: scene loaded."
              << MINI_TERMINAL_DEFAULT << std::endl;

    auto orgInstances = scene->instances;
    for (int i=0;i<instID;i++) {
      char suffix[1000];
      sprintf(suffix,"_%03i.mini",i);
      scene->instances = { orgInstances[i] };
      separated->save(outFileBase+suffix);
    }
    std::cout << MINI_TERMINAL_LIGHT_GREEN
              << "#exportIndividualInstances: scene saved."
              << MINI_TERMINAL_DEFAULT << std::endl;
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::exportIndividualInstances(ac,av); return 0; }
