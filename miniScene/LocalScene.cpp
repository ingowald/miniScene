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
#include "brix/scene/LocalScene.h"
#include "cup/tools/Lights.h"
#include "cup/tools/tessellateCurves.h"
#include "cup/tools/removeAllNonMeshShapes.h"
#include <iostream>

namespace brix {
  
  box3f xfmBox(const affine3f &xfm,
               const box3f &box)
  {
    const vec3f lo = box.lower;
    const vec3f hi = box.upper;

    box3f ret;
    ret.extend(xfmPoint(xfm,vec3f(lo.x,lo.y,lo.z)));
    ret.extend(xfmPoint(xfm,vec3f(hi.x,lo.y,lo.z)));
    ret.extend(xfmPoint(xfm,vec3f(lo.x,hi.y,lo.z)));
    ret.extend(xfmPoint(xfm,vec3f(hi.x,hi.y,lo.z)));
    ret.extend(xfmPoint(xfm,vec3f(lo.x,lo.y,hi.z)));
    ret.extend(xfmPoint(xfm,vec3f(hi.x,lo.y,hi.z)));
    ret.extend(xfmPoint(xfm,vec3f(lo.x,hi.y,hi.z)));
    ret.extend(xfmPoint(xfm,vec3f(hi.x,hi.y,hi.z)));
    return ret;
  }

  namespace scene {


    // box3f LocalScene::getBounds()
    // {
    //   box3f bounds;
    //   for (auto proxy : allProxies)
    //     bounds.extend(proxy.bounds);

    //   return bounds;
    // }
    
    LocalScene::LocalScene(pbrt::Scene::SP localSceneForSerialization,
                           const GlobalMeta &meta,
                           const std::vector<LocalInstance> &localInstances)
      : serialized::Scene(localSceneForSerialization),
      meta(meta),
      localInstances(localInstances)
    {
      std::cout << "computing local instances ..." << std::endl;
      for (int i=0;i<localInstances.size();i++)
        this->localInstances[i].localMeshID
          = objects.getIdx(localSceneForSerialization->world->instances[i]->object);
    }

    LocalScene::SP LocalScene::extractFrom(pbrt::Scene::SP masterScene,
                                           pbrt::Scene::SP thisRankScene,
                                           // pbrt::Scene::SP input,
                                           SplitSpecs::SP  splitSpecs,
                                           const NodeMask &myBits)
    {
      assert(masterScene);
      assert(thisRankScene);
      std::cout << "#brx.scene: extracting local scene from PBF and split-spec" << std::endl;
      // thisRankScene->makeSingleLevel();

      // PING; PRINT(thisRankScene->world->instances.size());
      PING;
      cup::tools::tessellateAllCurvesIn(thisRankScene);
#if 0
      // do NOT do this, as it will kill null instnaces, which we
      // require to maintain our global instnace ID mapping - for now,
      // it's the job of the partitioner to make sure the input scnees
      // don't contain anything we can't handle
      cup::tools::removeAllNonMeshShapes(thisRankScene,true,false);
#endif
          
      // ------------------------------------------------------------------
      // compute worldspace bounds for viewer
      // ------------------------------------------------------------------
      GlobalMeta global;
      PING;
      global.worldBoundsExLights = cup::tools::getWorldBounds(masterScene,/*ex lights:*/true);
      PING;
      global.proxies = splitSpecs->proxies;
      assert(!global.proxies.empty());

      PING;
      global.quadLights = cup::tools::extractQuadLights(cup::tools::findAllAreaLights(masterScene));
      std::cout << "#brx.scene: found " << global.quadLights.size() << " quad lights" << std::endl;

      PING; PRINT(thisRankScene->world->instances.size());
      cup::tools::removeAreaLightShapes(thisRankScene,true);
      
      PING; PRINT(thisRankScene->world->instances.size());
      // ------------------------------------------------------------------
      // extract our instances, and replace input's instance list with
      // ours only
      // ------------------------------------------------------------------
      std::vector<LocalInstance> localInstances;
      {
        std::vector<pbrt::Instance::SP> ourInstances;
        int numIGs = 0;
        for (int instID=0;instID<thisRankScene->world->instances.size();instID++) {
          const SplitSpecs::PerInst &thisSpec = splitSpecs->perInst[instID];
          global.ownerOfInstance.push_back(thisSpec.owner);
          
          pbrt::Instance::SP instance = thisRankScene->world->instances[instID];
          if (instance == nullptr) {
            global.localIDof.push_back(-1);
            continue;
          }
          assert(instance);
          pbrt::Object::SP   object   = instance->object;
          assert(object);
          
          const bool ownedLocally = (thisSpec.owner.bits & myBits.bits) != 0;
          if (!ownedLocally) {
            global.localIDof.push_back(-1);
          } else {
            global.localIDof.push_back(ourInstances.size());
            
            LocalInstance li;
            li.globalInstID = instID;
            li.localMeshID  = /* wont' know that until serialization */-1;
            li.xfm          = (const affine3f&)instance->xfm;
            localInstances.push_back(li);
            ourInstances.push_back(instance);
          }
        }

        // now replace input scene with ours only (this may free some objects...)
        thisRankScene->world->instances = ourInstances;
        // PING; PRINT(thisRankScene->world->instances.size());
        // PRINT(localInstances.size());
        // PRINT(global.localIDof.size());
      }

      LocalScene::SP local = std::make_shared<LocalScene>(thisRankScene,global,
                                                          std::move(localInstances));
      
      std::cout << "#brx.scene: done extracting local scene; found "
                << prettyNumber(local->shapes.size()) << " local meshes, in "
                << prettyNumber(thisRankScene->world->instances.size()) << " local instances"
                << std::endl;
      std::cout << "#brx.scene: num proxies "
                << prettyNumber(splitSpecs->proxies.size())
                << std::endl;
      return local;
    }
    
  } // ::brix::scene
} // ::brix

#endif

