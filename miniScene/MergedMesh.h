// ======================================================================== //
// Copyright 2018-2020 Ingo Wald                                            //
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

#pragma once

#include "brix/scene/SplitSpecs.h"
// #include "pbrtParser/Scene.h"
#include "brix/scene/Scene.h"

#if 0
namespace brix {

  struct MergedMeshBuilder {
    
    MergedMeshBuilder(const Scene *scene,
                      const pbrt::Object::SP object)
    {
      size_t numVertices = 0;
      size_t numIndices = 0;
      size_t numNormals = 0;
      size_t numTexcoords = 0;
      
      // -------------------------------------------------------
      // first pass - discover and count
      // -------------------------------------------------------
      std::vector<pbrt::TriangleMesh::SP> myMeshes;
      for (auto shape : shapes) {
        if (!shape) continue;
        pbrt::TriangleMesh::SP mesh = shape->as<pbrt::TriangleMesh>();
        if (!mesh) continue;

        MergedMesh mm;
        mm.globalMeshID = scene->getGlobalID(shape);
        mm.globalMaterialID = scene->getGlobalID(shape->material);
        mm.firstMergedIndex = numIndices;
        mm.firstMergedVertex = numVertices;
        mm.alphaTextureID = -1;
        auto it = mesh->textures.find("alpha");
        if (it != mesh->textures.end())
          mm.alphaTextureID = scene->textures.get(it->second);
        
        auto alphaTex = shape->textures.find("alpha");
        if (alphaTex != shape->textures.end())
          mm.alphaTextureID    = scene->textures.get(alphaTex->second);
        
        numIndices += mesh->index.size();
        numVertices += mesh->vertex.size();
        if (mesh->texcoord.empty()) 
          mm.firstMergedTexcoord = -1;
        else
          mm.firstMergedTexcoord = numTexcoords;
        numTexcoords += mesh->texcoord.size();
        if (mesh->normal.empty()) 
          mm.firstMergedNormal = -1;
        else
          mm.firstMergedNormal = numNormals;
        numNormals   += mesh->normal.size();
        subMeshes.push_back(mm);
        myMeshes.push_back(mesh);
      }
      if (myMeshes.empty()) return;

      // -------------------------------------------------------
      // second pass - execute
      // -------------------------------------------------------
      mergedIndices.resize(numIndices);
      mergedVertices.resize(numVertices);
      mergedNormals.resize(numNormals);
      mergedTexcoords.resize(numTexcoords);
      parallel_for(myMeshes.size(),[&](size_t subMeshID){
          const MergedMesh &mm = subMeshes[subMeshID];
          auto mesh = myMeshes[subMeshID];
          for (size_t i=0;i<mesh->index.size();i++)
            mergedIndices[mm.firstMergedIndex+i]
              = vec4i((const vec3i&)mesh->index[i]+vec3i(mm.firstMergedVertex),
                      (int)subMeshID);
          for (size_t i=0;i<mesh->vertex.size();i++) 
            mergedVertices[mm.firstMergedVertex+i]
              = (const vec3f&)mesh->vertex[i];
          if (!mesh->normal.empty())
            for (size_t i=0;i<mesh->normal.size();i++) 
              mergedNormals[mm.firstMergedNormal+i]
                = (const vec3f&)mesh->normal[i];
          if (!mesh->texcoord.empty())
            for (size_t i=0;i<mesh->texcoord.size();i++) 
              mergedTexcoords[mm.firstMergedTexcoord+i]
                = (const vec2f&)mesh->texcoord[i];
        });
      
      // -------------------------------------------------------
      // third pass: create owl data
      // -------------------------------------------------------
      geom = owlGeomCreate(owl,triMeshType);
      OWLBuffer vertex
        = owlDeviceBufferCreate(owl,OWL_FLOAT3,mergedVertices.size(),
                                mergedVertices.data());
      owlGeomSetBuffer(geom,"vertex",vertex);

      if (!mergedNormals.empty()) {
        OWLBuffer normal
          = owlDeviceBufferCreate(owl,OWL_FLOAT3,mergedNormals.size(),
                                  mergedNormals.data());
        owlGeomSetBuffer(geom,"normal",normal);
      } else 
        owlGeomSetBuffer(geom,"normal",nullptr);
          
      if (!mergedTexcoords.empty()) {
        OWLBuffer texcoord
          = owlDeviceBufferCreate(owl,OWL_FLOAT2,mergedTexcoords.size(),
                                  mergedTexcoords.data());
        owlGeomSetBuffer(geom,"texcoord",texcoord);
      } else 
        owlGeomSetBuffer(geom,"texcoord",nullptr);
          
      OWLBuffer index
        = owlDeviceBufferCreate(owl,OWL_INT4,mergedIndices.size(),
                                mergedIndices.data());
      owlGeomSetBuffer(geom,"index",index);
      
      owlTrianglesSetVertices(geom,vertex,mergedVertices.size(),sizeof(vec3f),0);
      owlTrianglesSetIndices(geom,index,mergedIndices.size(),sizeof(vec4i),0);

      OWLBuffer subMeshBuffer
        = owlDeviceBufferCreate(owl,OWL_USER_TYPE(MergedMesh),
                                subMeshes.size(),subMeshes.data());
      owlGeomSetBuffer(geom,"subMeshes",subMeshBuffer);
    }
    
    std::vector<vec4i> mergedIndices;
    std::vector<vec3f> mergedVertices;
    std::vector<vec3f> mergedNormals;
    std::vector<vec2f> mergedTexcoords;
    std::vector<MergedMesh> subMeshes;
  };
  
#endif
  
