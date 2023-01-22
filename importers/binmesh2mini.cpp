// ======================================================================== //
// Copyright 2018-2023 Ingo Wald                                            //
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
  std::cout << "Usage: ./binmesh2mini in.binmesh -o out.mini" << std::endl;
  std::cout << "Imports a 'binmesh' formatted mesh into a mini scene.\n";
  std::cout << "Each binmesh is a binary file with the following structure:\n";
  std::cout << "  size_t numVertices\n";
  std::cout << "  vec3f  vertices[numVertices]\n";
  std::cout << "  size_t numIndices\n";
  std::cout << "  vec3i  indices[numIndices]\n";
  exit(msg != "");
}

int main(int ac, char **av)
{
  std::string inFileName = "";
  std::string outFileName = "";
    
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg == "-o") {
      outFileName = av[++i];
    } else if (arg[0] != '-')
      inFileName = arg;
    else
      usage("unknown cmd line arg '"+arg+"'");
  }
    
  if (inFileName.empty()) usage("no input file name specified");
  if (outFileName.empty()) usage("no output file name base specified");
  
  std::cout << MINI_TERMINAL_BLUE
            << "loading binmesh file from " << inFileName
            << MINI_TERMINAL_DEFAULT << std::endl;
  
  Mesh::SP mesh = Mesh::create();
  
  std::ifstream in(inFileName,std::ios::binary);
  size_t count;
  in.read((char*)&count,sizeof(count));
  mesh->vertices.resize(count);
  in.read((char*)mesh->vertices.data(),count*sizeof(vec3f));
  in.read((char*)&count,sizeof(count));
  mesh->indices.resize(count);
  in.read((char*)mesh->indices.data(),count*sizeof(vec3i));
  mesh->material = Material::create();
  
  Object::SP object = Object::create({mesh});
  Scene::SP scene = Scene::create({Instance::create(object)});
  std::cout << MINI_TERMINAL_DEFAULT
            << "done importing; saving to " << outFileName
            << MINI_TERMINAL_DEFAULT << std::endl;
  scene->save(outFileName);
  std::cout << MINI_TERMINAL_LIGHT_GREEN
            << "scene saved"
            << MINI_TERMINAL_DEFAULT << std::endl;
  return 0;
}
