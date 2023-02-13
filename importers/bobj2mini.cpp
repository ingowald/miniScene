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
    
  size_t maxMeshSize = 1<<20;
  size_t maxTriangles = 1ull<<60;
  
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg == "-o") {
      outFileName = av[++i];
    } else if (arg == "-mms" || arg == "--max-mesh-size") {
      maxMeshSize = std::stol(av[++i]);
    } else if (arg == "-mt" || arg == "--max-triangles") {
      maxTriangles = std::stol(av[++i]);
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
  
  Object::SP object = Object::create();
  Mesh::SP mesh = Mesh::create();
  mesh->material = Material::create();
  
  std::ifstream in(inFileName,std::ios::binary);

  size_t numVertices;
  in.read((char*)&numVertices,sizeof(numVertices));
  std::cout << "expecting num vertices = " << prettyNumber(numVertices) << std::endl;
  size_t numTriangles;
  in.read((char*)&numTriangles,sizeof(numTriangles));
  std::cout << "expecting num triangles = " << prettyNumber(numTriangles) << std::endl;

  std::vector<vec3f> vertices(numVertices);
  in.read((char*)vertices.data(),numVertices*sizeof(vec3f));

  std::map<size_t,int> currentVertices;
  for (size_t triID=0;triID<std::min(numTriangles,maxTriangles);triID++) {
    vec3ul inputTri;
    in.read((char*)&inputTri,sizeof(inputTri));
    vec3i miniTri;
    for (int i=0;i<3;i++) {
      size_t idx = (&inputTri.x)[i];
      auto it = currentVertices.find(idx);
      if (it == currentVertices.end()) {
        miniTri[i] = mesh->vertices.size();
        mesh->vertices.push_back(vertices[idx]);
        currentVertices[idx] = miniTri[i];
      } else {
        miniTri[i] = it->second;
      }
    }
    mesh->indices.push_back(miniTri);
    if (mesh->vertices.size() >= maxMeshSize) {
      object->meshes.push_back(mesh);
      mesh = mini::Mesh::create();
      mesh->material = mini::Material::create();
      currentVertices.clear();
      std::cout << "[" << prettyNumber(triID+1) << "]" << std::flush;
    }
  }
  std::cout << "[" << prettyNumber(numTriangles) << "]" << std::flush;
  std::cout << std::endl;
  if (!mesh->indices.empty())
    object->meshes.push_back(mesh);
  
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
