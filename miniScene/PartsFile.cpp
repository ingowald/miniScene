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

// #include "PartsFile.h"
// // std
// #include <fstream>
// #include <set>

// namespace brix {
//   namespace scene {

//     /*! constructor */
//     PartsFile::PartsFile(std::vector<Part> &parts)
//       : parts(parts)
//     {
//       for (int partID=0;partID<parts.size();partID++) {
//         Part &part = this->parts[partID];
//         for (auto brickBox : part.bricks) {
//           allBricks.push_back(Brick(brickBox,partID));
//           worldBounds.extend(brickBox);
//         }
//       }
//     }

//     /*! get a list of _all_ bricks, irrespective of which scene
//       parts they belong to */
//     // std::vector<Brick> PartsFile::getAllBricks()
//     // {
//     //   std::vector<Brick> bricks;
//     //   for (int partID=0;partID<parts.size();partID++) {
//     //     for (auto proxy : parts[partID].bricks)
//     //       bricks.push_back(Brick(proxy,partID));
//     //   }
//     //   return bricks;
//     // }

//     template<typename T>
//     void read(std::ifstream &f, T &t)
//     {
//       f.read((char*)&t,sizeof(t));
//       if (!f.good())
//         throw std::runtime_error("could not read from file...");
//     }
    
//     template<typename T>
//     void read(std::ifstream &f, std::set<T> &st)
//     {
//       int num;
//       read<int>(f,num);
//       T t;
//       for (int i=0;i<num;i++) {
//         read(f,t);
//         st.insert(t);
//       }
//     }
    
//     template<typename T>
//     void read(std::ifstream &f, std::vector<T> &vt)
//     {
//       int num;
//       read<int>(f,num);
//       T t;
//       for (int i=0;i<num;i++) {
//         read(f,t);
//         vt.push_back(t);
//       }
//     }
    
//     PartsFile::SP PartsFile::read(const std::string &fileName)
//     {
//       std::ifstream in(fileName);
//       if (!in.good()) throw std::runtime_error("could not open '"+fileName+"'");

//       std::cout << "# reading parts file " << fileName << std::endl;
//       std::string token;
//       in >> token;
//       if (!in.good() || token != "brix")
//         throw std::runtime_error("'"+fileName+"' : not a brix file");
//       in >> token;
//       if (!in.good() || (token != "parts" && token != "bricks"))
//         throw std::runtime_error("'"+fileName+"' : not a brix file");
//       in >> token;
//       if (!in.good() || token != "file")
//         throw std::runtime_error("'"+fileName+"' : not a brix file");
//       in >> token;
//       if (!in.good() || token != "version")
//         throw std::runtime_error("'"+fileName+"' : not a brix file");
//       in >> token; // version string
//       std::string line;
//       std::getline(in,line);

//       int numSubs;
//       scene::read(in,numSubs);
//       std::cout << "reading " << numSubs << " scene parts .... " << std::flush;
//       std::vector<Part> parts;
//       for (int i=0;i<numSubs;i++) {
//         Part part;
//         scene::read(in,part.bricks);
//         scene::read(in,part.activeIGs);
//         parts.push_back(part);
//       }
//       std::cout << " done; #IGs=";
//       for (auto part : parts)
//         std::cout << "[" << prettyNumber(part.activeIGs.size()) << "]";
//       std::cout << std::endl;
//       return std::make_shared<PartsFile>(parts);
//     }

//   } // ::brix::scene
// } // ::brix
