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

#include "miniScene/Scene.h"

namespace mini {
  // namespace scene {
    
    template<typename T>
    struct Serialized {
      inline size_t size() const { return list.size(); }
      inline const T &operator[](int ID) const
      { assert(ID>=0); assert(ID<list.size()); return list[ID]; }
      
      int getID(T t)
      {
        if (registry.find(t) == registry.end())
          return -1;

        return registry[t];
      }
      
      inline bool wasKnown(T t) const
      {
        return  (registry.find(t) != registry.end());
      }

      bool addWasKnown(T t)
      {
        if (registry.find(t) != registry.end())
          return true;

        registry[t] = (int)list.size();
        list.push_back(t);
        return false;
      }
      
      void add(T t) { addWasKnown(t); }
      
      std::map<T,int> registry;
      std::vector<T>  list;
    };

    struct SerializedScene {
      SerializedScene() {}
      SerializedScene(Scene *scene);
      
      int getID(Texture::SP t)  { return textures.getID(t); }
      int getID(Material::SP m) { return materials.getID(m); }
      int getID(Mesh::SP t)     { return meshes.getID(t); }
      int getID(Object::SP t)   { return objects.getID(t); }
      
      Serialized<Texture::SP>  textures;
      Serialized<Material::SP> materials;
      Serialized<Object::SP>   objects;
      Serialized<Mesh::SP>     meshes;
    };

} // ::mini
