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

#if 0
#pragma once

#include "brix/scene/Scene.h"
#include "brix/scene/SplitSpecs.h"
#include "brix/scene/Proxy.h"

namespace brix {
  namespace scene {
    
    struct LocalInstance {
      affine3f xfm;
      int localMeshID;
      int globalInstID;
    };
    
    struct GlobalMeta {
      std::vector<pod::QuadLight> quadLights;
      std::vector<scene::Proxy>   proxies;
      std::vector<int>            localIDof;
      std::vector<NodeMask>       ownerOfInstance;
      box3f                       worldBoundsExLights;
    };
    
    struct LocalScene : public cup::serialized::Scene {
      typedef std::shared_ptr<LocalScene> SP;

      LocalScene(pbrt::Scene::SP localSceneForSerialization,
                 const GlobalMeta &meta,
                 const std::vector<LocalInstance> &localInstances);
      
      static LocalScene::SP extractFrom(pbrt::Scene::SP masterScene,
                                        pbrt::Scene::SP thisRankScene,
                                        SplitSpecs::SP  splitSpecs,
                                        const NodeMask &myBits);
      
      /*! one entry per *global* instance */
      std::vector<LocalInstance> localInstances;
      GlobalMeta meta;
    };
    
    
  } // ::brix::scene
} // ::brix
#endif
