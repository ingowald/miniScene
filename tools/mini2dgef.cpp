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
  std::cout << "Usage: ./mini2dgef in.mini -o out.dgef" << std::endl;
  std::cout << "loads a mini scene, flattens into flat list "
            << "of triangles, and writes as dgef format\n";
  std::cout << "Each dgef is a binary file with the following structure:\n";
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
            << "loading dgef file from " << inFileName
            << MINI_TERMINAL_DEFAULT << std::endl;

  Scene::SP scene = Scene::load(inFileName);
  std::cout << MINI_TERMINAL_GREEN
            << "scene loaded... "
            << std::endl;

  std::set<Object::SP> uniqueObjects;
  for (auto inst : scene->instances)
    uniqueObjects.insert(inst->object);
  std::map<Object::SP,int> objectIDof;
  std::vector<int> numMeshesInObject;
  std::vector<Mesh::SP> meshes;
  for (auto obj : uniqueObjects) {
    objectIDof[obj] = objectIDof.size();
    numMeshesInObject.push_back(obj->meshes.size());
    for (auto mesh : obj->meshes)
      meshes.push_back(mesh);
  }
  
  std::ofstream out(outFileName,std::ios::binary);
  
  size_t header = 0xdefdefdefULL;
  out.write((char*)&header,sizeof(header));
  
  size_t count = meshes.size();
  out.write((char*)&count,sizeof(count));
  for (auto mesh : meshes) {
    count = mesh->vertices.size();
    out.write((char*)&count,sizeof(count));
    for (auto v : mesh->vertices) {
      vec3d vv = vec3d(v);
      out.write((char*)&vv,sizeof(vv));
    }

    std::vector<vec3ul> indices;
    for (auto idx : mesh->indices)
      indices.push_back(vec3ul(idx));
    count = indices.size();
    out.write((char*)&count,sizeof(count));
    out.write((char*)indices.data(),count*sizeof(indices[0]));
  }

  count = numMeshesInObject.size();
  out.write((char*)&count,sizeof(count));
  out.write((char*)numMeshesInObject.data(),count*sizeof(numMeshesInObject[0]));

  count = scene->instances.size();
  out.write((char*)&count,sizeof(count));
  for (auto inst : scene->instances) {
    affine3d xfm = affine3d(inst->xfm);
    int objID = objectIDof[inst->object];
    out.write((char*)&xfm,sizeof(xfm));
    out.write((char*)&objID,sizeof(objID));
  }
    
  
  std::cout << MINI_TERMINAL_LIGHT_GREEN
            << "lattened scene saved in dgef format; done."
            << MINI_TERMINAL_DEFAULT << std::endl;
  return 0;
}
