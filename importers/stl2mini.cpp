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

//std
#include <set>
#include <fstream>

namespace mini {
    
    Scene::SP loadSTL(const std::string &fileName)
    {
      std::ifstream in(fileName,std::ios::binary);
      if (!in) throw std::runtime_error("could not open '"+fileName+"'");
      char header[80];
      in.read(header,sizeof(header));
      if (!in) throw std::runtime_error("could not read STL header!?");

      header[79] = 0;
      if (strstr(header,"solid"))
        throw std::runtime_error("this STL file looks like a *ASCII* STL file; "
                                 "yet this parser only supports binary STL for now");
      
      
      int numTris = -1;
      in.read((char*)&numTris,sizeof(numTris));
      if (!in) throw std::runtime_error("could not read STL tri count!?");

      Mesh::SP mesh = Mesh::create();
      for (int triID=0;triID<numTris;triID++) {
        struct { vec3f n, v[3]; } tri;
        in.read((char*)&tri,sizeof(tri));
        if (!in) throw std::runtime_error("could not read triangle from STL file");

        char unused[2];
        in.read(unused,sizeof(unused));

        vec3i indices;
        std::map<vec3f,int> knownVertices;
        for (int i=0;i<3;i++) {
          auto thisVtx = tri.v[i];
          if (knownVertices.find(thisVtx) == knownVertices.end()) {
            knownVertices[thisVtx] = mesh->vertices.size();
            mesh->vertices.push_back(thisVtx);
          }
          indices[i] = knownVertices[thisVtx];
        }
        mesh->indices.push_back(indices);
      }
      // fclose(out);
      Object::SP object = Object::create({mesh});
      Instance::SP inst = Instance::create(object);
      Scene::SP scene = Scene::create({inst});

      return scene;
    }

} // ::mini


void usage(const std::string &msg)
{
  if (!msg.empty()) std::cerr << std::endl << "***Error***: " << msg << std::endl << std::endl;
  std::cout << "Usage: ./obj2brix inFile.pbf -o outfile.brx" << std::endl;
  std::cout << "Imports a STL+MTL file into brix's scene format.\n";
  std::cout << "(from where it can then be partitioned and/or rendered)\n";
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
            << "loading STL model from " << inFileName
            << MINI_TERMINAL_DEFAULT << std::endl;

  mini::Scene::SP scene = mini::loadSTL(inFileName);
    
  std::cout << MINI_TERMINAL_DEFAULT
            << "done importing; saving to " << outFileName
            << MINI_TERMINAL_DEFAULT << std::endl;
  scene->save(outFileName);
  std::cout << MINI_TERMINAL_LIGHT_GREEN
            << "scene saved"
            << MINI_TERMINAL_DEFAULT << std::endl;
  return 0;
}
