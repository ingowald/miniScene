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

#include "miniScene/common.h"

namespace mini {

  /*! a object for storing texture data; allowing to store both image
      textures and raw data for 'embedded' ptex texture (where
      'embedded' means that this stores the same data as the .ptex
      file, but without any claim to being able to do something with
      it) */
  struct Texture {
    typedef std::shared_ptr<Texture> SP;

    static SP create() { return std::make_shared<Texture>(); }
      
    typedef enum {
                  UNDEFINED=0, EMBEDDED_PTEX, FLOAT4, FLOAT1, RGBA_UINT8, BYTE4=RGBA_UINT8, 
    } Format;
      
    /* for embedded ptex, size is always {0,0}, since data vector
       containst the raw ptex fiel content; for all others it is the
       number of pixels in x and y */
    vec2i  size    { 0, 0 };
    Format format  { UNDEFINED };
    std::vector<uint8_t> data;
  };

} // ::mini
