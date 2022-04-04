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

#include "brix/scene/Proxy.h"

namespace brix {
  namespace scene {

    struct LocalScene : public SerializedPBRTScene {
      typedef std::shared_ptr<LocalScene> SP;

      LocalScene(pbrt::Scene::SP input,
                 SplitSpecs::SP  splitSpecs,
                 const NodeMask &myBits);
      
      static LocalScene::SP extractFrom(pbrt::Scene::SP input,
                                        SplitSpecs::SP  splitSpecs,
                                        const NodeMask &myBits)
      {
        return std::make_shared<LocalScene>(input,splitSpecs,myBits);
      }

      box3f getBounds();
      
      /*! list of _all_ logical inst-shapes in the scene */
      std::vector<pbrt::Shape::SP> localShapes;
      std::vector<LocalObject>     localObjects;
      std::vector<LocalMesh>       localMeshes;
      std::vector<LocalInst>       localInsts;
      std::vector<Material>        materials;
      
      // /*! everything that *others* have and we do *not* have
      //     locally */
      // std::vector<Proxy>           remoteProxies;
      /*! list of ALL proxies, so primary rays can check if primary
          hit is to be traced on local device. TODO: should eventually
          move proxies into different groups by ownership, then build
          a two-level hierarchy and use active bits to restrict which
          ones we trace */
      std::vector<Proxy>           allProxies;

      /*! local refcounts of shapes we own, so their data arrays don't get released */
      std::set<pbrt::Shape::SP>    ourShapes;
    };

  } // ::brix::scene
} // ::brix
