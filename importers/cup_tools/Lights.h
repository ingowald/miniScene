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

#pragma once

#include "cup.h"
// #include "cup/pod/Lights.h"

namespace cup {
  namespace pod {
    /*! a *instantiated* quad light - in pbrt, area ligths can live in
      instances, but for ligth sampling you proabbly want to have them
      in a single list of world-space lights.... this is one
      world-sapce instance of a instantiated quad light */
    struct QuadLight {
      vec3f L;
      /*! vertex at (u,v)=(0,0) */
      vec3f P;
      /*! vector pointing along u edge */
      vec3f du;
      /*! vector pointing along v edge */
      vec3f dv;

      float area;
      vec3f N;

    //   template<typename Random>
    //   inline __device__ bool sample(LightSample &light,
    //                                 const SurfaceSample &surface,
    //                                 Random &random) const;
    };
  }
  
  namespace tools {

    using LightsList = std::vector<std::pair<pbrt::Instance::SP,pbrt::Shape::SP>>;
    using QuadLightsList = std::vector<pod::QuadLight>;
    
    /*! iterate through the given scene (which must be a single-level
      scene), and create a serializaiton of all shapes in the scene
      that are light sources */
    LightsList findAllAreaLights(pbrt::Scene::SP scene);

    /*! given a global light list, extract all those that can be
        recognized as quad lights, and removes them from the input
        light list . */
    QuadLightsList extractQuadLights(const LightsList &lightsList);
    
    /*! kick out all shapes that are tagged as area lights */
    void removeAreaLightShapes(pbrt::Scene::SP scene,
                               bool retainNullInstances = false);
    
  } // ::cup::tools
} // ::cup
