// ======================================================================== //
// Copyright 2018-2022 Ingo Wald                                            //
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

#include <owl/owl.h>

namespace device {
  using namespace owl;

  struct Material {
  };
  
  struct Camera {
    Camera()
    {}
    
    Camera(const vec2i &fbSize,
           const vec3f &from,
           const vec3f &at,
           const vec3f &up,
           const float fov);

    vec3f lens_00;
    vec3f dir_00;
    vec3f dir_du;
    vec3f dir_dv;
  };

  struct LaunchParams {
    OptixTraversableHandle world;
    Camera    camera;
    vec2i     fbSize;
    uint32_t *fbPointer;
  };

  inline Camera::Camera(const vec2i &fbSize,
                        const vec3f &from,
                        const vec3f &at,
                        const vec3f &up,
                        const float fov)
  {
    lens_00 = (from);
    dir_00  = normalize(at-from);

    const float angle = fov;
    dir_du = normalize(cross(dir_00,up));
    dir_du *= sinf(angle * M_PI/180.f);
    dir_du *= fbSize.x / (float)std::min(fbSize.x,fbSize.y);

    dir_dv = normalize(cross(dir_du,dir_00));
    dir_dv *= length(dir_du) * fbSize.y / (float)fbSize.x;

    dir_00 -= 0.5f * dir_du;
    dir_00 -= 0.5f * dir_dv;

    dir_du *= 1.f/fbSize.x;
    dir_dv *= 1.f/fbSize.y;
  }

  struct Mesh // : public scene::PartialScene::MeshBase
  {
    Material material;
    vec3i *indices;
    vec3f *vertices;
    vec3f *normals;
    vec2f *texcoords;
    // cudaTextureObject_t alphaTexture = 0;
    // cudaTextureObject_t colorTexture = 0;
  };
}
