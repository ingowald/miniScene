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

  void makeBox(int ac, char **av)
  {
    vec3d pos { 1e8, .001, -10000. };
    // vec3d pos { 0., 0., 0. };
    vec3d size { 1., 1., 1. };
    
    std::string inFileName = "";
    std::string outFileName = "makeBox.dpmini";
    bool instancePerFace = false;
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg == "-o") {
        outFileName = av[++i];
      } else if (arg == "-s" || arg == "--size") {
        size.x = std::stod(av[++i]);
        size.y = std::stod(av[++i]);
        size.z = std::stod(av[++i]);
      } else if (arg == "-p" || arg == "--pos") {
        pos.x = std::stod(av[++i]);
        pos.y = std::stod(av[++i]);
        pos.z = std::stod(av[++i]);
      } else if (arg == "--instancePerFace") {
        instancePerFace = true;
      } else
        throw std::runtime_error("unknown cmdline argument '"+arg+"'");
    }
    std::cout << MINI_TERMINAL_LIGHT_BLUE
              << "loading mini file from " << inFileName 
              << MINI_TERMINAL_DEFAULT << std::endl;
    Mesh::SP mesh = Mesh::create();

    const int NUM_VERTICES = 8;
    static const vec3f unitBoxVertices[NUM_VERTICES] =
      {
        {-1.f, -1.f, -1.f},
        {+1.f, -1.f, -1.f},
        {+1.f, +1.f, -1.f},
        {-1.f, +1.f, -1.f},
        {-1.f, +1.f, +1.f},
        {+1.f, +1.f, +1.f},
        {+1.f, -1.f, +1.f},
        {-1.f, -1.f, +1.f},
      };

    vec3d lo = pos;
    vec3d hi = pos+size;
    PRINT(lo);
    PRINT(hi);
    for (int i=0;i<8;i++) {
      vec3f u = unitBoxVertices[i];
      vec3d v(u.x < 0. ? lo.x : hi.x,
              u.y < 0. ? lo.y : hi.y,
              u.z < 0. ? lo.z : hi.z);
      mesh->vertices.push_back(v);
    }

    const int NUM_INDICES = 12;
    std::vector<vec3i> unitBoxIndices =
      {
        {0, 2, 1}, //face front
        {0, 3, 2},
        {2, 3, 4}, //face top
        {2, 4, 5},
        {1, 2, 5}, //face right
        {1, 5, 6},
        {0, 7, 4}, //face left
        {0, 4, 3},
        {5, 4, 7}, //face back
        {5, 7, 6},
        {0, 6, 7}, //face bottom
        {0, 1, 6}
      };
    mesh->indices = unitBoxIndices;

    Scene::SP scene;
    if (instancePerFace) {
      scene = Scene::create();
      for (int i=0;i<6;i++) {
        Mesh::SP mm = Mesh::create();
        mm->vertices = mesh->vertices;
        mm->indices.push_back(unitBoxIndices[2*i+0]);
        mm->indices.push_back(unitBoxIndices[2*i+1]);
        scene->instances.push_back(Instance::create(Object::create({mm})));
      }
    } else {
      scene = Scene::create({Instance::create(Object::create({mesh}))});
    }
    std::cout << MINI_TERMINAL_LIGHT_GREEN
              << "saving to " << outFileName
              << MINI_TERMINAL_DEFAULT << std::endl;
    scene->save(outFileName);
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::makeBox(ac,av); return 0; }

