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

#include "cup/cup.h"
#include "owl/common/parallel/parallel_for.h"

namespace cup {
  namespace tools {
    
    /*! compute the given scene's world bounds, including or exluding
        the shapes for area lights */
    box3f getWorldBounds(pbrt::Scene::SP pbrtScene,
                         bool excludeLights)
    {
      std::mutex mutex;
      box3f bounds;
      
      owl::parallel_for(pbrtScene->world->instances.size(),[&](const size_t instID){
          auto inst = pbrtScene->world->instances[instID];
          if (inst && inst->object)
            owl::parallel_for(inst->object->shapes.size(),[&](const size_t shapeID){
                auto shape = inst->object->shapes[shapeID];
                if (excludeLights && shape->areaLight)
                  return;
                pbrt::box3f shapeBounds = shape->getBounds();
                std::lock_guard<std::mutex> lock(mutex);
                bounds.extend((const box3f&)shapeBounds);
              });
        });
      return bounds;
    }
    
  } // ::cup::tools
} // ::cup
