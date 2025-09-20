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
  std::cout << "Usage: ./lavai32f32toMini in.idx.i32 in.vtx.flt -o out.mini" << std::endl;
  std::cout << "Imports a model in 'idx.i32+vtx.flt' format (as used in some.\n";
  std::cout << "internal data exchange w/ lava team.\n";
  std::cout << "The .idx.i32 file is  a binary file with the following structure:\n";
  std::cout << "  size_t numIndices\n";
  std::cout << "  vec3i  indices[numIndices]\n";
  std::cout << "The .vtx.flt file is  a binary file with the following structure:\n";
  std::cout << "  size_t numVertices\n";
  std::cout << "  vec3f  vertices[numVertices]\n";
  exit(msg != "");
}

template<typename T>
void readVector(std::vector<T> &vec, std::ifstream &in)
{
  in.seekg(0,std::ios::end);
  size_t sz = in.tellg();
  PRINT(sz);
  in.seekg(0,std::ios::beg);
  size_t cnt = sz/sizeof(T);
  vec.resize(cnt);
  in.read((char*)vec.data(),sz);
}

int main(int ac, char **av)
{
  std::string inFileNameIdx = "";
  std::string inFileNameVtx = "";
  std::string outFileName = "";
  
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg == "-o") {
      outFileName = av[++i];
    } else if (arg[0] != '-') {
      if (inFileNameIdx == "")
        inFileNameIdx = arg;
      else
        inFileNameVtx = arg;
    } else
      usage("unknown cmd line arg '"+arg+"'");
  }
    
  if (inFileNameVtx.empty()) usage("no vtx input file name specified");
  if (inFileNameIdx.empty()) usage("no idx input file name specified");
  if (outFileName.empty()) usage("no output file name base specified");
  
  std::cout << MINI_TERMINAL_BLUE
            << "loading lava dump file from " << inFileNameIdx << " and " << inFileNameVtx
            << MINI_TERMINAL_DEFAULT << std::endl;
  
  Mesh::SP mesh = Mesh::create();
  
  std::ifstream in_vtx(inFileNameVtx,std::ios::binary);
  std::ifstream in_idx(inFileNameIdx,std::ios::binary);

  readVector(mesh->vertices,in_vtx);
  readVector(mesh->indices,in_idx);
  mesh->material = DisneyMaterial::create();
  
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
