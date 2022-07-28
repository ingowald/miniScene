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
#include <utility>

#include "miniScene/Scene.h"
#include "miniScene/Serialized.h"

namespace mini {

  void usage(const std::string& error = "")
  {
    if (!error.empty())
      std::cerr << MINI_COLOR_RED << "Error: " << error
                << MINI_COLOR_DEFAULT << std::endl << std::endl;
    std::cout << "miniSubdivide a.mini -o subdivided.mini" << std::endl;
    exit(error.empty() ? 0 : 1);
  }

  int addMidpoint(std::map<std::pair<int, int>, int>& alreadyAddedVertices,
                  Mesh::SP oldMesh,
                  int v0, int v1,
                  Mesh::SP newMesh)
  {
    vec3f midpoint = (oldMesh->vertices[v0] + oldMesh->vertices[v1]) / 2.0f; // midpoint
    // Add midpoint to new vertices, if not added already
    std::pair<int, int> pair = std::make_pair(v0, v1);
    if (v0 > v1)
      pair = std::make_pair(v1, v0);
    if (alreadyAddedVertices.find(pair) == alreadyAddedVertices.end())
      {
        alreadyAddedVertices[pair] = newMesh->vertices.size();
        newMesh->vertices.push_back(midpoint);
      }
    // Return the index of midpoint
    return alreadyAddedVertices[pair];
  }

  void miniSubdivide(int ac, char** av)
  {
    std::string outFileName = "";

    if (ac == 1)
      usage();
    std::string inFileName = "";
    for (int i = 1; i < ac; i++)
      {
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

    try
      {
        std::cout << "Creating scene \n";

        std::cout << MINI_COLOR_LIGHT_BLUE
                  << "Loading mini file from " << inFileName
                  << MINI_COLOR_DEFAULT << std::endl;

        Scene::SP scene = Scene::load(inFileName);

        // Save uniques instances, objects and meshes
        std::map<Instance::SP, Instance::SP> instances;
        std::map<Object::SP, Object::SP> objects;
        std::map<Mesh::SP, Mesh::SP> meshes;

        // List of new instances
        std::vector<Instance::SP> newInstances;

        // Iterate each instance
        for (auto& inst : scene->instances)
          {
            // Reuse object if found
            if (objects.find(inst->object) != objects.end())
              {
                newInstances.push_back(Instance::create(objects[inst->object], inst->xfm));
                continue;
              }
            // List of new meshes
            std::vector<Mesh::SP> newMeshes;

            // Iterate each mesh
            for (auto& oldMesh : inst->object->meshes)
              {
                // Reuse mesh if found
                if (meshes.find(oldMesh) != meshes.end())
                  {
                    newMeshes.push_back(meshes[oldMesh]);
                    continue;
                  }
                // Create an empty mesh with a dummy material
                Mesh::SP newMesh = Mesh::create(Material::create());
                // Put original vertices to new mesh
                newMesh->vertices = oldMesh->vertices;
                // Mapping between original vertices and midpoints vs new vertices
                std::map<std::pair<int, int>, int> alreadyAddedVertices;
                // Iterate each triangle
                for (auto& index : oldMesh->indices)
                  {
                    int A = index.x, B = index.y, C = index.z;
                    int AB = addMidpoint(alreadyAddedVertices, oldMesh, A, B, newMesh);
                    int BC = addMidpoint(alreadyAddedVertices, oldMesh, B, C, newMesh);
                    int CA = addMidpoint(alreadyAddedVertices, oldMesh, C, A, newMesh);
                    newMesh->indices.push_back({ A, AB, CA });
                    newMesh->indices.push_back({ B, BC, AB });
                    newMesh->indices.push_back({ C, CA, BC });
                    newMesh->indices.push_back({ AB, BC, CA });
                  }
                std::cout << "Old: vertices=" << oldMesh->vertices.size() << ", "
                          << "triangles=" << oldMesh->indices.size() << std::endl;
                std::cout << "New: vertices=" << newMesh->vertices.size() << ", "
                          << "triangles=" << newMesh->indices.size() << std::endl;
                // Append to the list of new meshes
                newMeshes.push_back(newMesh);
                // Update unique set of meshes for later use
                meshes[oldMesh] = newMesh;
              }
            // Create a new object with the new list of meshes
            Object::SP newObject = Object::create(newMeshes);
            // Create a new instance with the new object
            Instance::SP newInstance = Instance::create(newObject);
            // Append to the list of new instances
            newInstances.push_back(newInstance);
            // Update unique set of objects for later use
            objects[inst->object] = newObject;
            //   // Update unique set of instances for later use
            //   instances[inst] = newInstance;
          }
        // Create a new scene from the list of new instances
        Scene::SP newScene = Scene::create(newInstances);

        // Save scene
        std::cout << "saving scene \n";
        newScene->save(outFileName);

        std::cout << MINI_COLOR_LIGHT_GREEN
                  << "#miniInfo: subdivided scene saved."
                  << MINI_COLOR_DEFAULT << std::endl;
      }
    catch (const std::exception& exc)
      {
        std::cerr << exc.what();
      }
  }
} // ::mini

int main(int ac, char** av)
{
  mini::miniSubdivide(ac, av); return 0;
}



