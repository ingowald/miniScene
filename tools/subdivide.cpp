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

  void usage(const std::string &error = "")
  {
    if (!error.empty())
      std::cerr << MINI_COLOR_RED << "Error: " << error
                << MINI_COLOR_DEFAULT << std::endl << std::endl;
    std::cout << "miniSubdivide inputScene.mini -o subdividedScene.mini" << std::endl;
    exit(error.empty() ? 0 : 1);
  }

  int addMidpoint(std::map<std::pair<int, int>, int> &alreadyAddedVertices,
                  Mesh::SP oldMesh,
                  int v0, int v1,
                  Mesh::SP newMesh)
  {
    // Add midpoint to new vertices, if not added already
    std::pair<int, int> pair = std::make_pair(std::min(v0, v1),std::max(v0, v1));
    if (alreadyAddedVertices.find(pair) != alreadyAddedVertices.end())
      return alreadyAddedVertices[pair];
    
    alreadyAddedVertices[pair] = newMesh->vertices.size();
    
    vec3f midpoint = .5f* (oldMesh->vertices[v0] + oldMesh->vertices[v1]);
    newMesh->vertices.push_back(midpoint);
    
    if (!oldMesh->normals.empty()) {
      vec3f midpoint = .5f* (oldMesh->normals[v0] + oldMesh->normals[v1]);
      newMesh->normals.push_back(midpoint);
    }
    
    if (!oldMesh->texcoords.empty()) {
      vec2f midpoint = .5f* (oldMesh->texcoords[v0] + oldMesh->texcoords[v1]);
      newMesh->texcoords.push_back(midpoint);
    }
    // Return the index of midpoint
    return alreadyAddedVertices[pair];
  }

  Mesh::SP subdivide(Mesh::SP in)
  {
    Mesh::SP out = Mesh::create(in->material);

    // Put original vertices to new mesh
    out->vertices  = in->vertices;
    out->normals   = in->normals;
    out->texcoords = in->texcoords;
    // Mapping between original vertices and midpoints vs new vertices
    std::map<std::pair<int, int>, int> alreadyAddedVertices;
    // Iterate each triangle
    for (auto index : in->indices) {
      int A = index.x, B = index.y, C = index.z;
      int AB = addMidpoint(alreadyAddedVertices, in, A, B, out);
      int BC = addMidpoint(alreadyAddedVertices, in, B, C, out);
      int CA = addMidpoint(alreadyAddedVertices, in, C, A, out);
      out->indices.push_back({ A, AB, CA });
      out->indices.push_back({ B, BC, AB });
      out->indices.push_back({ C, CA, BC });
      out->indices.push_back({ AB, BC, CA });
    }
    
    return out;
  }
  
  void miniSubdivide(int ac, char** av)
  {
    std::string outFileName = "";

    if (ac == 1)
      usage();
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

    std::cout << MINI_COLOR_LIGHT_BLUE
              << "loading mini file from " << inFileName
              << MINI_COLOR_DEFAULT << std::endl;

    Scene::SP scene = Scene::load(inFileName);
    std::set<Object::SP> objects;
    std::map<Mesh::SP, Mesh::SP> meshSubstitutions;
    // create list of all input objects
    for (auto inst : scene->instances) objects.insert(inst->object);
    // create list of all input meshes that we need to create 4x substitutions for.
    for (auto obj : objects)
      for (auto mesh : obj->meshes)
        meshSubstitutions[mesh] = {};
    
    // now, compute 'replacement' for each input mesh, by subdividing it.
    for (auto meshesIt : meshSubstitutions)
      meshSubstitutions[meshesIt.first] = subdivide(meshesIt.first);
    
    // now go over all objects, and do the substitution
    for (auto obj : objects)
      for (auto &mesh : obj->meshes)
        mesh = meshSubstitutions[mesh];

    // nothing to do for objects or instances; they've got their old
    // content swapped out by now.
    std::cout << "saving scene" << std::endl;
    scene->save(outFileName);
    
    std::cout << MINI_COLOR_LIGHT_GREEN
              << "#miniInfo: subdivided scene saved."
              << MINI_COLOR_DEFAULT << std::endl;
  }
} // ::mini

int main(int ac, char** av)
{
  mini::miniSubdivide(ac, av); return 0;
}



