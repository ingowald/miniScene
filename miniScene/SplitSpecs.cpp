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
#include "SplitSpecs.h"
#include <fstream>

namespace brix {
  namespace scene {

    const int SplitSpecs::g_magic = 0x12300003;

    /*! load a previously saved SplitSpec */
    SplitSpecs::SP SplitSpecs::load(const std::string &fileName)
    {
      std::cout << "#brx.scene: loading split-specs from " << fileName << std::endl;

      std::ifstream in(fileName,std::ios::binary);
      
      int magic, numInsts, numProxies, numParts;
      int sumProxies = 0;
      in.read((char*)&magic,sizeof(magic));
      if (magic != g_magic)
        throw std::runtime_error(fileName+" : wrong file format or version");
      in.read((char*)&numParts,sizeof(numParts));

      auto ss = std::make_shared<SplitSpecs>(numParts);
      readVec(in,ss->perInst);
      readVec(in,ss->proxies);
      assert(!ss->perInst.empty());
      assert(!ss->proxies.empty());
      
      std::cout << "#brx.scene: done reading split-spec, found "
                << prettyNumber(ss->perInst.size())
                << " instances, with a total of "
                << prettyNumber(ss->proxies.size())
                << " proxies" << std::endl;
      return ss;
    }
    

    /*! save to binary file. note this does *not* contain the scene
      itself, so always has to be used in conjuction with the
      scene used to construct this split spec */
    void SplitSpecs::saveTo(const std::string &fileName) const
    {
      std::cout << "#brx.scene: writing split-specs to " << fileName << std::endl;

      std::ofstream out(fileName,std::ios::binary);
      
      int magic = g_magic;
      out.write((char*)&magic,sizeof(magic));
      out.write((char*)&numParts,sizeof(numParts));

      writeVec(out,perInst);
      writeVec(out,proxies);
      
      std::cout << "#brx.scene: done writing split-specs ..." << std::endl;
    }
    
  }
}
#endif
