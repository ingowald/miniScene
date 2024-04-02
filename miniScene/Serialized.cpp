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

#include "miniScene/Serialized.h"

namespace mini {
    
    SerializedScene::SerializedScene(Scene *scene)
    {
      textures.add(nullptr);

      for (auto inst : scene->instances) {
        if (!inst) continue;

        auto obj = inst->object;
        
        if (!obj || objects.addWasKnown(obj)) continue;

        for (auto mesh : obj->meshes) {
          if (!mesh || meshes.addWasKnown(mesh)) continue;
          
          auto material = mesh->material;
          assert(material);
          if (materials.addWasKnown(material)) continue;

          DisneyMaterial::SP disney = material->as<DisneyMaterial>();
          if (disney) {
            textures.add(disney->colorTexture);
            textures.add(disney->alphaTexture);
          }
          BlenderMaterial::SP blender = material->as<BlenderMaterial>();
          if (blender) {
            textures.add(blender->baseColorTexture);
            textures.add(blender->metallicTexture);
            textures.add(blender->roughnessTexture);
            textures.add(blender->alphaTexture);
            textures.add(blender->normalTexture);
          }
        }
      }
    }

} // ::mini
