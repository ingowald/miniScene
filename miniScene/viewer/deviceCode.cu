// ======================================================================== //
// Copyright 2022-2022 Ingo Wald                                            //
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

#include "deviceCode.h"
#include <owl/common/math/random.h>

using namespace device;

extern "C" __constant__
LaunchParams optixLaunchParams;

using Random = owl::common::LCG<4>;
  
  struct PRD {
    int primID = -1, instID = -1;
    float t, u, v;
  };
  
  OPTIX_MISS_PROGRAM(miss)()
  {
  }
  
  OPTIX_CLOSEST_HIT_PROGRAM(MeshCH)()
  {
    // const LaunchParams &lp = optixLaunchParams;
    auto &prd  = getPRD<PRD>();
    auto &self = getProgramData<Mesh>();
    prd.instID = optixGetInstanceIndex(); //optixGetInstanceId();
    prd.primID = optixGetPrimitiveIndex();
    
    prd.t      = optixGetRayTmax();
    prd.u      = optixGetTriangleBarycentrics().x;
    prd.v      = optixGetTriangleBarycentrics().y;
  }

  OPTIX_RAYGEN_PROGRAM(renderFrame)()
  {
    auto &lp = optixLaunchParams;
    const vec2i launchIdx = getLaunchIndex();
    if (launchIdx.x >= lp.fbSize.x) return;
    if (launchIdx.y >= lp.fbSize.y) return;

    Random random;
    random.init(launchIdx.x,
                launchIdx.y);
    
    const vec2f screen = vec2f(launchIdx)+vec2f(random(),random());
    const Camera &camera = lp.camera;
    
    Ray ray(camera.lens_00,
            camera.dir_00
            +screen.x*camera.dir_du
            +screen.y*camera.dir_dv,
            0.f,1e20f);
    PRD prd;
    traceRay(lp.world,ray,prd);

    // if (launchIdx == vec2i{600,400}) {
    //   printf("---------------------------------\nrender on dev!\n");
    //   printf("fbsize.x %i\n",lp.fbSize.x);
    //   printf("fbsize.y %i\n",lp.fbSize.y);
    //   printf("fbtpr %lx\n",lp.fbPointer);
    //   printf("org %f %f %f\n",ray.origin.x,ray.origin.y,ray.origin.z);
    //   printf("dir %f %f %f\n",ray.direction.x,ray.direction.y,ray.direction.z);
    // }

    uint32_t rgba = make_rgba(owl::randomColor(prd.instID));
    int pixelID = launchIdx.x+launchIdx.y*lp.fbSize.x;
    if (lp.fbPointer) lp.fbPointer[pixelID] = rgba;
  }

