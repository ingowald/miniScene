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

  std::string myPretty(size_t n)
  {
    if (n < 1024) return "  "+prettyNumber(n);

    return "  "+prettyNumber(n)+"\t("+std::to_string(n)+")";
  }
  
  void miniDumpBBs(int ac, char **av)
  {
    std::string inFileName = "";
    std::string outFileName = "miniDumpBBs.bb";
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
              << "#miniDumpBBs: scene loaded."
              << MINI_TERMINAL_DEFAULT << std::endl;

    std::vector<box3f> boxes;
    for (auto inst : scene->instances) {
      for (auto mesh : inst->object->meshes) {
        for (auto idx : mesh->indices) {
          box3f bb;
          bb.extend(xfmPoint(inst->xfm,mesh->vertices[idx.x]));
          bb.extend(xfmPoint(inst->xfm,mesh->vertices[idx.y]));
          bb.extend(xfmPoint(inst->xfm,mesh->vertices[idx.z]));
          boxes.push_back(bb);
        }
      }
    }

    std::ofstream out(outFileName,std::ios::binary);
    out.write((const char *)boxes.data(),boxes.size()*sizeof(box3f));
    if (!out.good()) throw std::runtime_error("error in writing array of boxes...");
    std::cout << MINI_TERMINAL_GREEN
              << "done. written " << prettyNumber(boxes.size())
              << " boxes to " << outFileName << MINI_TERMINAL_DEFAULT << std::endl;
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::miniDumpBBs(ac,av); return 0; }

