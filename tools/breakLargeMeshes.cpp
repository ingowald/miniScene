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

/* this tool takes a given scene, and creates a new scene that
   contains exactly the same triangles, but in a way that each object
   in the scene has only a certain maximum number of triangles. If a
   object has more triangles than desired we first try to split that
   object based on the possibly mulitple meshes it has; if splitting
   based on meshes is not enoguh we will also split large meshes into
   multiple smaller ones until the desired threshold is reached. */

#include "miniScene/Scene.h"
#include "miniScene/Serialized.h"

namespace mini {

  std::string myPretty(size_t n)
  {
    if (n < 1024) return "  "+prettyNumber(n);

    return "  "+prettyNumber(n)+"\t("+std::to_string(n)+")";
  }

  box3f getBounds(Mesh::SP mesh, vec3i idx)
  {
    box3f bounds;
    bounds.extend(mesh->vertices[idx.x]);
    bounds.extend(mesh->vertices[idx.y]);
    bounds.extend(mesh->vertices[idx.z]);
    return bounds;
  }

  int findOrExtractVertex(Mesh::SP inMesh,
                          Mesh::SP outMesh,
                          int vtxID,
                          std::map<int,int> &alreadyExtracted)
  {
    auto it = alreadyExtracted.find(vtxID);
    if (it == alreadyExtracted.end())
      return it->second;

    int newID = outMesh->vertices.size();
    outMesh->vertices.push_back(inMesh->vertices[vtxID]);
    if (!inMesh->normals.empty())
      outMesh->normals.push_back(inMesh->normals[vtxID]);
    if (!inMesh->texcoords.empty())
      outMesh->texcoords.push_back(inMesh->texcoords[vtxID]);
    
    alreadyExtracted[vtxID] = newID;
    return newID;
  }
  
  Mesh::SP extractMesh(Mesh::SP in, const std::vector<vec3i> &indices)
  {
    std::map<int,int> alreadyExtracted;
    Mesh::SP out = Mesh::create(in->material);
    for (auto idx : indices) {
      idx.x = findOrExtractVertex(in,out,idx.x,alreadyExtracted);
      idx.y = findOrExtractVertex(in,out,idx.y,alreadyExtracted);
      idx.z = findOrExtractVertex(in,out,idx.z,alreadyExtracted);
      out->indices.push_back(idx);
    }
    return out;
  }

  std::string toString(Mesh::SP mesh)
  {
    std::stringstream ss;
    ss << "#tris = " << prettyNumber(mesh->indices.size())
       << " #vtx = " << prettyNumber(mesh->vertices.size())
       << std::endl;
    return ss.str();
  }
  
  std::vector<Mesh::SP> splitAtCenter(Mesh::SP mesh, int maxSize)
  {
    std::cout << "splitting mesh w/ " << prettyNumber(mesh->indices.size()) << " trianglges ..." << std::endl;
    box3f centBounds;
    for (auto idx : mesh->indices)
      centBounds.extend(getBounds(mesh,idx).center());

    std::vector<vec3i> lIndices, rIndices;
    if (centBounds.lower == centBounds.upper) {
      for (auto idx : mesh->indices)
        if (lIndices.size() <= rIndices.size())
          lIndices.push_back(idx);
        else
          rIndices.push_back(idx);
    } else {
      int dim = arg_max(centBounds.size());
      float pos = centBounds.center()[dim];
      for (auto idx : mesh->indices)
        if (getBounds(mesh,idx).center()[dim] < pos)
          lIndices.push_back(idx);
        else
          rIndices.push_back(idx);
    }
    Mesh::SP lMesh = extractMesh(mesh,lIndices);
    Mesh::SP rMesh = extractMesh(mesh,rIndices);
    std::cout << " > got l = " << toString(lMesh) << std::endl;
    std::cout << " > got r = " << toString(rMesh) << std::endl;
    return { lMesh,rMesh };
  }
  
  std::vector<Mesh::SP> breakMesh(Mesh::SP mesh, int maxSize)
  {
    std::cout << "breaking mesh w/ " << prettyNumber(mesh->indices.size()) << std::endl;
    if (mesh->indices.size() <= maxSize)
      return { mesh };

    std::cout << " -> mesh w/ " << prettyNumber(mesh->indices.size()) << " triangles is larger than threshold of " << prettyNumber(maxSize) << " ... breaking it" << std::endl;
    std::vector<Mesh::SP> halves = splitAtCenter(mesh,maxSize);
    std::vector<Mesh::SP> result;
    for (auto frag : breakMesh(halves[0],maxSize))
      result.push_back(frag);
    for (auto frag : breakMesh(halves[1],maxSize))
      result.push_back(frag);
    return result;
  }

  std::vector<Object::SP> breakObject(Object::SP in, int maxSize)
  {
    std::vector<Object::SP> out;
    std::vector<Mesh::SP> meshes;
    for (auto mesh : in->meshes)
      for (auto frag : breakMesh(mesh,maxSize))
        meshes.push_back(frag);

    size_t currentSize = 0;
    std::vector<Mesh::SP> currentMeshes;
    for (auto mesh : meshes) {
      if (currentSize + mesh->indices.size() > maxSize) {
        out.push_back(Object::create(currentMeshes));
        currentMeshes.clear();
        currentSize = 0;
      } else {
        currentMeshes.push_back(mesh);
        currentSize += mesh->indices.size();
      }
    }
    if (!currentMeshes.empty())
      out.push_back(Object::create(currentMeshes));
    return out;
  }
  
  Scene::SP breakLargeMeshes(Scene::SP in, size_t maxMeshSize)
  {
    Scene::SP out = Scene::create();

    out->quadLights = in->quadLights;
    out->dirLights = in->dirLights;
    out->envMapLight = in->envMapLight;
    
    std::map<Object::SP,std::vector<Object::SP>> brokenObjects;
    for (auto inst : in->instances) 
      brokenObjects[inst->object] = {};

    for (auto &pair : brokenObjects)
      pair.second = breakObject(pair.first,maxMeshSize);

    for (auto inst : in->instances) {
      auto &fragments = brokenObjects[inst->object];
      for (auto frag : fragments)
        out->instances.push_back(Instance::create(frag,inst->xfm));
    }
    return out;
  }
  
  void breakLargeMeshes(int ac, char **av)
  {
    std::string outFileName = "";
    std::string inFileName = "";
    int maxMeshSize = 10000000;
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg == "-o")
        outFileName = av[++i];
      else if (arg[0] != '-')
        inFileName = arg;
      else
        throw std::runtime_error("unknown cmdline argument '"+arg+"'");
    }
    if (inFileName.empty())
      throw std::runtime_error("no input file specified");
    if (outFileName.empty())
      throw std::runtime_error("no output file specified (-o)");

    std::cout << MINI_COLOR_LIGHT_BLUE
              << "loading mini file from " << inFileName 
              << MINI_COLOR_DEFAULT << std::endl;
    Scene::SP scene = Scene::load(inFileName);
    std::cout << MINI_COLOR_LIGHT_GREEN
              << "#miniSeparateRootMeshes: scene loaded."
              << MINI_COLOR_DEFAULT << std::endl;

    Scene::SP separated = breakLargeMeshes(scene,maxMeshSize);
    std::cout << MINI_COLOR_LIGHT_BLUE
              << "done separating; saving to " << outFileName 
              << MINI_COLOR_DEFAULT << std::endl;
    separated->save(outFileName);
    std::cout << MINI_COLOR_LIGHT_GREEN
              << "#miniSeparateRootMeshes: scene saved."
              << MINI_COLOR_DEFAULT << std::endl;
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::breakLargeMeshes(ac,av); return 0; }

