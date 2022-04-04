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

#include "brix/common/common.h"
#include "brix/common/NodeMask.h"
#include "brix/scene/Proxy.h"
// std
#include <set>
#include <map>
#include <vector>
#include <queue>

namespace brix {
  namespace scene {

    using brix::NodeMask;

    template<typename T>
    inline void readVec(std::istream &in, std::vector<T> &vec)
    {
      int cnt;
      in.read((char *)&cnt,sizeof(cnt));
      vec.resize(cnt);
      in.read((char*)vec.data(),cnt*sizeof(T));
    }

    template<typename T>
    inline void writeVec(std::ostream &out, const std::vector<T> &vec)
    {
      int cnt = (int)vec.size();
      out.write((char *)&cnt,sizeof(cnt));
      out.write((char*)vec.data(),cnt*sizeof(T));
    }
    
    /*! this struct describes, for a given scene, which nodes any
        given IG shuld go to, and which proxies each IG would generate
        if not owned */
    struct SplitSpecs {
      static const int g_magic;
      
      SplitSpecs(int numParts) : numParts(numParts) {}
      
      typedef std::shared_ptr<SplitSpecs> SP;

      /*! describes an IG in a given partitioned scene */
      struct PerInst {
        /*! which ranks this IG goes on */
        scene::NodeMask   owner;
      };
      
      inline size_t size() const { return perInst.size(); }
      
      std::vector<PerInst> perInst;
      // std::vector<PerPart> perPart;
      std::vector<Proxy>   proxies;

      const int numParts;
      
      /*! load a previously saved SplitSpec */
      static SplitSpecs::SP load(const std::string &fileName);

      /*! save to binary file. note this does *not* contain the scene
          itself, so always has to be used in conjuction with the
          scene used to construct this split spec */
      void saveTo(const std::string &filename) const;
    };
    
  }
}
#endif
