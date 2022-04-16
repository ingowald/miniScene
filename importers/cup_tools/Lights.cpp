// ======================================================================== //
// Copyright 2019-2020 Ingo Wald                                            //
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

#include "cup/tools/Lights.h"
// std
#include <set>

namespace cup {
  namespace tools {

    /*! iterate through the given scene (which must be a single-level
      scene), and create a serializaiton of all shapes in the scene
      that are light sources */
    LightsList findAllAreaLights(pbrt::Scene::SP scene)
    {
      LightsList result;
      assert(scene && scene->isSingleLevel());
      for (auto inst : scene->world->instances)
        if (inst && inst->object)
          for (auto shape : inst->object->shapes)
            if (shape && shape->areaLight)
              result.push_back({inst,shape});
      return result;
    }

    /*! given a global light list, extract all those that can be
        recognized as quad lights, and removes them from the input
        light list . */
    QuadLightsList extractQuadLights(const LightsList &lightsList)
    {
      LightsList unrecognized;
      QuadLightsList result;
      for (auto light : lightsList) {
        auto inst = light.first;
        auto shape = light.second;
        if (!shape) {
          unrecognized.push_back(light);
          continue;
        } 
        pbrt::TriangleMesh::SP mesh = shape->as<pbrt::TriangleMesh>();
        if (!mesh) {
          unrecognized.push_back(light);
          continue;
        }

        pod::QuadLight ql;
        const affine3f &xfm = (const affine3f&)inst->xfm;
        ql.P  = xfmPoint(xfm,(const vec3f&)mesh->vertex[0]);
        ql.du = xfmPoint(xfm,(const vec3f&)mesh->vertex[1]) - ql.P;
        ql.dv = xfmPoint(xfm,(const vec3f&)mesh->vertex[3]) - ql.P;
        ql.N  = cross(ql.du,ql.dv);
        ql.area = length(ql.N);
        ql.N  = normalize(ql.N);
        
        auto rgbLight = shape->areaLight->as<pbrt::DiffuseAreaLightRGB>();
        if (rgbLight) {
          ql.L = (const vec3f&)rgbLight->L;
          result.push_back(ql);
          continue;
        }

        std::cout << "#cup: Warning - found a quad light, "
                  << "but it's not a rgbLight ... skipping this" << std::endl;
        unrecognized.push_back(light);
      }
      // lightsList = unrecognized;
      return result;
    }
    
    /*! kick out all shapes that are tagged as area lights */
    void removeAreaLightShapes(pbrt::Scene::SP scene,
                               bool retainNullInstances)
    {
      std::set<pbrt::Object::SP> objects;
      for (auto inst : scene->world->instances)
        if (inst && inst->object)
          objects.insert(inst->object);

      for (auto object : objects) {
        std::vector<pbrt::Shape::SP> noLightShapes;
        for (auto shape : object->shapes)
          if (shape && !shape->areaLight)
            noLightShapes.push_back(shape);
        object->shapes = noLightShapes;
        object->haveComputedBounds = false;
      }

      std::vector<pbrt::Instance::SP> remainingInstances;
      for (auto inst : scene->world->instances) {
        if (!inst) {
          remainingInstances.push_back(inst);
          continue;
        }
        if (inst && inst->object && !inst->object->shapes.empty()) {
          remainingInstances.push_back(inst);
          inst->haveComputedBounds = false;
        }
      }
      scene->world->instances = remainingInstances;
      scene->world->haveComputedBounds = false;
    }
    
  } // ::cup::tools
} // ::cup
