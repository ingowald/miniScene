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

  void usage(const std::string &error = "")
  {
    if (!error.empty())
      std::cerr << MINI_COLOR_RED << "Error: " << error
                << MINI_COLOR_DEFAULT << std::endl << std::endl;
    std::cout << "miniSubdivide a.mini b.mini ... -o subdivided.mini" << std::endl;
    exit(error.empty()?0:1);
  }
  
  void miniSubdivide(int ac, char** av)
  {
      
      std::string outFileName = "";
      

      //*putting in places info from command line
      if (ac == 1) usage();
      std::string inFileName = "";
      for (int i = 1; i < ac; i++) {
          std::string arg = av[i];
          if (arg[0] != '-')
              inFileName = arg;
          else if (arg == "-o")
              outFileName = av[++i];
          else
               usage("unknown cmdline argument '" + arg + "'");
      }

      if (inFileName.empty())
          usage("no input file names specified");
      if (outFileName.empty())
          usage("no output file name specified");

      try {

          std::cout << "Creating scene \n";

          Scene::SP out = Scene::create();

          std::cout << MINI_COLOR_LIGHT_BLUE
              << "loading mini file from " << inFileName
              << MINI_COLOR_DEFAULT << std::endl;
           // code for new output scene 
         
          Scene::SP scene = Scene::load(inFileName);
          std::cout << "saving scene \n";

          out = scene;
          out->save(outFileName);
          std::cout << MINI_COLOR_LIGHT_GREEN
              << "#miniInfo: subdivided scene saved."
              << MINI_COLOR_DEFAULT << std::endl;

      }

      catch (...) {
          usage(" error occured");
      }
     
          /*
                 


                  //STL file
                  int numTris = -1;
                  in.read((char*)&numTris, sizeof(numTris));
                  if (!in) throw std::runtime_error("could not read STL tri count!?");

                  Mesh::SP mesh = Mesh::create();
                  std::map<vec3f, int> knownVertices;
                  for (int triID = 0; triID < numTris; triID++) {
                      struct { vec3f n, v[3]; } tri;
                      in.read((char*)&tri, sizeof(tri));
                      if (!in) throw std::runtime_error("could not read triangle from STL file");

                      char unused[2];
                      in.read(unused, sizeof(unused));

                      vec3i indices;
                     
                      for (int i = 0; i < 3; i++) {
                          auto thisVtx = tri.v[i];
                          if (knownVertices.find(thisVtx) == knownVertices.end()) {
                              knownVertices[thisVtx] = mesh->vertices.size();
                              mesh->vertices.push_back(thisVtx);
                          }
                          indices[i] = knownVertices[thisVtx];
                      }
                      mesh->indices.push_back(indices);
                  }

                  for (auto& inst : scene->instances)
                      for (auto& mesh : inst->object.meshes)
                          for (auto& index: mesh->indices) {
                              auto A = mesh->vertices[index.ab];
                              auto B = mesh->vertices[index.bc];
                              auto C = mesh->vertices[index.ca];
                              auto AB = (A + B) / 2;
                              auto BC = (B + C) / 2;
                              auto CA = (A + C) / 2;
                      }
                  }
              }

                    }

          
                }
              }
              */

        
      
  }
} // ::mini

int main(int ac, char **av) 
{ mini::miniSubdivide(ac, av);
    std::cout << "Hello World" << std :: endl;
    return 0;


}

