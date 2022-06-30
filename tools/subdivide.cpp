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

#include <map>
#include <string>
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

  std::string midpointIndexToString(int32_t i1, int32_t i2)
  {
      if (i1 < i2)
          return std::to_string(i1) + "," + std::to_string(i2);
      else
          return std::to_string(i2) + "," + std::to_string(i1);
  }

  
  void miniSubdivide(int ac, char** av)
  {
      
      std::string outFileName = "";
      
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

          std::cout << "Creating scene" << std::endl;

         // Scene::SP out = Scene::create();

          std::cout << MINI_COLOR_LIGHT_BLUE
              << "loading mini file from " << inFileName
              << MINI_COLOR_DEFAULT << std::endl;

          Scene::SP scene = Scene::load(inFileName);


          //subdivision

          for (auto& inst : scene->instances)
          {
              for (auto& mesh : inst->object->meshes)
              {
                  // Create a new mesh
                  std::map<std::string, int32_t> vertexMap;
                  Mesh::SP newMesh = Mesh::create(mesh->material);
                  std::vector<vec3f> vertices;
                  std::vector<vec3i> indices;
                  for (auto& index : mesh->indices)
                  {
                      int32_t i[3]; // indices for original vertices
                      int32_t j[3]; // indices for midpoints
                      vec3f v[3];   // original vertices
                      vec3f u[3];   // midpoints
                      // get original vertices
                      for (int k = 0; k < 3; k++)
                      {
                          i[k] = index[k];
                          v[k] = mesh->vertices[i[k]];
                      }
                      // calculate midpoints and add to 'vertices'
                      for (int k = 0; k < 3; k++)
                      {
                          int i1 = k;
                          int i2 = (k + 1) % 3;
                          for (int l = 0; l < 3; l++)
                              u[k][l] = (v[i1][l] + v[i2][l]) / 2.0f;
                          std::string indexString = midpointIndexToString(i[i1], i[i2]);
                          if (vertexMap.find(indexString) != vertexMap.end())
                          {
                              j[k] = vertexMap[indexString];
                          }
                          else
                          {
                              j[k] = vertices.size();
                              vertexMap[indexString] = j[k];
                              vertices.push_back(u[k]);
                          }
                      }
                      // add vertices to 'vertices'
                      for (int k = 0; k < 3; k++)
                      {
                          std::string indexString = std::to_string(i[k]);
                          if (vertexMap.find(indexString) != vertexMap.end())
                          {
                              i[k] = vertexMap[indexString];
                          }
                          else
                          {
                              i[k] = vertices.size();
                              vertexMap[indexString] = i[k];
                              vertices.push_back(v[k]);
                          }
                      }
                      // add 4 subtriangles to 'indices'
                      for (int k = 0; k < 3; k++)
                      {
                          vec3i newIndex;
                          newIndex[0] = i[k];
                          newIndex[1] = j[k];
                          newIndex[2] = j[(k + 2) % 3];
                          indices.push_back(newIndex);
                      }
                      vec3i newIndex;
                      newIndex[0] = j[0];
                      newIndex[1] = j[1];
                      newIndex[2] = j[2];
                      indices.push_back(newIndex);
                  }
                  newMesh->vertices = vertices;
                  newMesh->indices = indices;
                  std::cout << "Original: vertices=" << mesh->vertices.size() << ", " <<
                      "triangles=" << mesh->indices.size() << std::endl;
                  std::cout << "New: vertices=" << newMesh->vertices.size() << ", " <<
                      "triangles=" << newMesh->indices.size() << std::endl;

                  // Create a new object
                  std::vector<Mesh::SP> newMeshes;
                  newMeshes.push_back(mesh);
                  Object::SP newObject = Object::create(newMeshes);

                  // Create a new instance
                  Instance::SP newInstance = Instance::create(newObject);

                  // Create a new scene
                  std::vector<Instance::SP> newInstances;
                  newInstances.push_back(newInstance);
                  Scene::SP newScene = Scene::create(newInstances);

                  std::cout << "saving scene \n";
                  newScene->save(outFileName);

                  break;
              }
              break;
          }

         
          std::cout << MINI_COLOR_LIGHT_GREEN
              << "#miniInfo: subdivided scene saved."
              << MINI_COLOR_DEFAULT << std::endl;

      }

      catch (const std::exception & exc) {
          std::cerr << exc.what();
      }
     
      
  }
} // ::mini

int main(int ac, char** av)
{
    mini::miniSubdivide(ac, av); return 0;
}
