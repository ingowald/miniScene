// ======================================================================== //
// Copyright 2026-2026 Ingo Wald                                            //
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
#include <fstream>

using namespace mini;

void usage(const std::string &msg)
{
  if (!msg.empty()) std::cerr << std::endl << "***Error***: " << msg << std::endl << std::endl;
  std::cout << "Usage: ./mini2tstri in.mini -o out.tstri" << std::endl;
  std::cout << "loads a mini scene, flattens into flat list "
            << "of triangles, and writes as 'ts' (tim sandstrom) triangles format\n";
  std::cout << "Each tstri file is a binary file with the following structure:\n";
  std::cout << "  struct Tri { struct { vec3f pos, float scalar } vertex[3] };\n";
  std::cout << "  file = Tri*\n";
  std::cout << "(no header, no trailer, just plain list of triangles)\n";
  exit(msg != "");
}

int main(int ac, char **av)
{
  std::string inFileName = "";
  std::string outFileName = "";
  bool firstInstOnly = false;

  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg == "-o") {
      outFileName = av[++i];
    } else if (arg == "--first-inst-only") {
      firstInstOnly = true;
    } else if (arg[0] != '-')
      inFileName = arg;
    else
      usage("unknown cmd line arg '"+arg+"'");
  }
    
  if (inFileName.empty()) usage("no input file name specified");
  if (outFileName.empty()) usage("no output file name base specified");
  
  std::cout << MINI_TERMINAL_BLUE
            << "loading input file from " << inFileName
            << MINI_TERMINAL_DEFAULT << std::endl;

  Scene::SP scene = Scene::load(inFileName);
  std::cout << MINI_TERMINAL_GREEN
            << "scene loaded... "
            << std::endl;
  std::ofstream out(outFileName,std::ios::binary);

  for (auto inst : scene->instances) {
    for (auto mesh : inst->object->meshes) {
      static int meshID = 0;
      const auto &push = [&](vec3f v) {
        v = xfmPoint(inst->xfm,v);
        out.write((char *)&v,sizeof(v));
        float f = meshID;
        out.write((char *)&f,sizeof(f));
      };
      for (auto tri : mesh->indices) {
        push(mesh->vertices[tri.x]);
        push(mesh->vertices[tri.y]);
        push(mesh->vertices[tri.z]);
      }
      ++meshID;
    }
  }
  
  std::cout << MINI_TERMINAL_LIGHT_GREEN
            << "flattened scene saved in tstri format; done."
            << MINI_TERMINAL_DEFAULT << std::endl;
  return 0;
}
