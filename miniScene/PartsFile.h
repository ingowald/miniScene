// // ======================================================================== //
// // Copyright 2018 Ingo Wald                                                 //
// //                                                                          //
// // Licensed under the Apache License, Version 2.0 (the "License");          //
// // you may not use this file except in compliance with the License.         //
// // You may obtain a copy of the License at                                  //
// //                                                                          //
// //     http://www.apache.org/licenses/LICENSE-2.0                           //
// //                                                                          //
// // Unless required by applicable law or agreed to in writing, software      //
// // distributed under the License is distributed on an "AS IS" BASIS,        //
// // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// // See the License for the specific language governing permissions and      //
// // limitations under the License.                                           //
// // ======================================================================== //

// #pragma once

// #include "brix/common/common.h"

// #define PBRT_PARSER_VECTYPE_NAMESPACE  owl::common

// #include "pbrtParser/semantic/Scene.h"

// namespace brix {
//   namespace input {
//     using namespace pbrt::semantic;
//   };
  
//   namespace scene {
    
//     /*! each "brick" is a little bounding box that represents some
//       fraction of the volume of each scene, or scene part. Each
//       scene part can be represented through multiple bricks */
//     struct Brick {
//       Brick() = default;
//       Brick(const box3f &domain, int partID)
//         : domain(domain),
//           partID(partID)
//       {}
      
//       box3f domain;
//       int   partID;
//     };

//     struct PartsFile {
//       typedef std::shared_ptr<PartsFile> SP;

//       /*! describes one part of a scene. is _not_ mutually exlusive
//         with other parts (ie, the same geometry _may_ be in other
//         parts of the scene, too */
//       struct Part {
//         /*! the bricks describing the volume that this part is
//           responsible for */
//         std::vector<box3f> bricks;
        
//         /*! the list of instgeoms that this part _must_ handle */
//         std::vector<int>   activeIGs;
//       };

//       /*! constructor */
//       PartsFile(std::vector<Part> &parts);
      
//       /*! read parts file from disk... */
//       static PartsFile::SP read(const std::string &fileName);
      
//       /*! the list of parts that the scnee was split into. any node
//         will typically own only some of them - will be set in the
//         constructor, and should never be messed with */
//       std::vector<Part> parts;
      
//       /*! the list of _all_ bricks, irrespective of which scene parts
//         they belong to - will be set in the constructor, and should
//         never be messed with */
//       std::vector<Brick> allBricks;

//       /*! the world bounding box, across all bricks/parts - will be
//         set in the constructor, and should never be messed with */
//       box3f worldBounds;
//     };
    
//   } // ::brix::scene
// } // ::brix
