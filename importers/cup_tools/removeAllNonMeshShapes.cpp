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

#include "removeAllNonMeshShapes.h"
#include <set>

namespace cup {
  namespace tools {
    
    /*! a pre-processing pass that removes all shapes that aren't
      triangle meshes, and then removes all objects and instances
      that become empty by doing so */
    void removeAllNonMeshShapes(pbrt::Scene::SP scene)
    {
      assert(scene);
      assert(scene->world);
      assert(scene->isSingleLevel());

      std::set<pbrt::Object::SP> allObjects;
      for (auto inst : scene->world->instances)
        if (inst && inst->object)
          allObjects.insert(inst->object);

      for (auto object : allObjects) {
        std::vector<pbrt::Shape::SP> remainingShapes;
        for (auto shape : object->shapes)
          if (shape && shape->as<pbrt::TriangleMesh>())
            remainingShapes.push_back(shape);
        object->shapes = remainingShapes;
      }
      
      std::vector<pbrt::Instance::SP> remainingInstances;
      for (auto inst : scene->world->instances) {
        if (inst && inst->object && !inst->object->shapes.empty())
          remainingInstances.push_back(inst);
      }
      scene->world->instances = remainingInstances;
    }
    
  } // ::cup::tools
} // ::cup

