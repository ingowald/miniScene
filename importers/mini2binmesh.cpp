// ======================================================================== //
// Copyright 2018-2022 Ingo Wald                                            //
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
  std::cout << "Usage: ./mini2binmesh2 inmini. -o out.binmesh" << std::endl;
  std::cout << "loads a mini scene, flattens into flat list "
            << "of triangles, and writes as binmesh format\n";
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
  
  std::cout << MINI_COLOR_BLUE
            << "loading binmesh file from " << inFileName
            << MINI_COLOR_DEFAULT << std::endl;

  Scene::SP scene = Scene::load(inFileName);
  std::cout << MINI_COLOR_LIGHT_GREEN
            << "scene loaded; now flattening into a single mesh... " << outFileName
            << MINI_COLOR_DEFAULT << std::endl;
  std::vector<vec3f> vertices;
  std::vector<vec3i> indices;
  for (auto inst : scene->instances)
    for (auto mesh : inst->object->meshes) {
      int idxOfs = indices.size();
      for (auto vtx : mesh->vertices)
        vertices.push_back(xfmPoint(inst->xfm,vtx));
      for (auto idx : mesh->indices)
        indices.push_back(idxOfs+idx);
    }
  
  std::cout << MINI_COLOR_GREEN
            << "done flattening into a single mesh; saving to " << outFileName
            << MINI_COLOR_DEFAULT << std::endl;
  std::ofstream out(outFileName,std::ios::binary);
  size_t count;
  count = vertices.size();
  out.write((char*)&count,sizeof(count));
  out.write((char*)vertices.data(),count*sizeof(vec3f));
  count = indices.size();
  out.write((char*)&count,sizeof(count));
  out.write((char*)indices.data(),count*sizeof(vec3i));
  
  std::cout << MINI_COLOR_LIGHT_GREEN
            << "scene saved"
            << MINI_COLOR_DEFAULT << std::endl;
  return 0;
}
