// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA
// CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "DGEF.h"
#include <fstream>

namespace mini {
  namespace dgef {
    
    Scene::SP load(const std::string &fileName)
    {
      std::ifstream in(fileName.c_str(),std::ios::binary);

      std::vector<Mesh::SP> meshes;
      
      size_t header;
      in.read((char*)&header,sizeof(header));
      
      bool isProperDGEF = (header == 0xdefdefdefULL);
      size_t numMeshes;
      in.read((char*)&numMeshes,sizeof(numMeshes));
      for (int meshID=0;meshID<numMeshes;meshID++) {
        Mesh::SP _mesh = Mesh::create();
        meshes.push_back(_mesh);
        Mesh &mesh = *_mesh;
      
        size_t count;
        in.read((char*)&count,sizeof(count));
        mesh.vertices.resize(count);
        in.read((char*)mesh.vertices.data(),
                count*sizeof(vec3d));
      
        in.read((char*)&count,sizeof(count));
        for (size_t i=0;i<count;i++) {
          vec3ul idx;
          in.read((char*)&idx,sizeof(idx));
          mesh.indices.push_back({(int)idx.x,(int)idx.y,(int)idx.z});
        }
      }

      if (!isProperDGEF) {
        Object::SP object = Object::create(meshes);
        return Scene::create({Instance::create(object)});
      }
      
      size_t numObjects;
      in.read((char*)&numObjects,sizeof(numObjects));
      int meshBegin = 0;
      std::vector<Object::SP> objects;
      for (int i=0;i<numObjects;i++) {
        int count;
        in.read((char*)&count,sizeof(count));
        Object::SP object = Object::create();
        for (int i=0;i<count;i++)
          object->meshes.push_back(meshes[meshBegin++]);
        objects.push_back(object);
      }
      
      size_t numInstances;
      Scene::SP scene = Scene::create();
      in.read((char*)&numInstances,sizeof(numInstances));
      for (int instID=0;instID<numInstances;instID++) {
        Instance::SP inst = Instance::create();
        in.read((char*)&inst->xfm,sizeof(inst->xfm));
        int objectID;
        in.read((char*)&objectID,sizeof(objectID));
        inst->object = objects[objectID];
        scene->instances.push_back(inst);
      }
      return scene;
    }

  }
}

int main(int ac, char **av)
{
  std::string inFileName = "not_specified.mini";
  std::string outFileName = "";
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg == "-o")
      outFileName = av[++i];
    else if (arg[0] != '-')
      inFileName = arg;
    else
      throw std::runtime_error("unrecognized cmdline arg");
  }
  mini::Scene::SP scene = mini::dgef::load(inFileName);
  scene->save(outFileName);
  return 0;
}

