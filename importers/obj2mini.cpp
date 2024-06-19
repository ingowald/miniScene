// ======================================================================== //
// Copyright 2018-2023 Ingo Wald                                            //
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

#include "importers/importOBJ.h"
#include <cstring>
#include <set>

void usage(const std::string &msg)
{
  if (!msg.empty()) std::cerr << std::endl << "***Error***: " << msg << std::endl << std::endl;
  std::cout << "Usage: ./obj2brix inFile.pbf -o outfile.brx" << std::endl;
  std::cout << "Imports a OBJ+MTL file into brix's scene format.\n";
  std::cout << "(from where it can then be partitioned and/or rendered)\n";
  exit(msg != "");
}

int main(int ac, char **av)
{
  std::string inFileName = "";
  std::string outFileName = "";
    
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg == "-o") {
      outFileName = av[++i];
    } else if (arg[0] != '-')
      inFileName = arg;
    else
      usage("unknown cmd line arg '"+arg+"'");
  }
    
  if (inFileName.empty()) usage("no input file name specified");
  if (outFileName.empty()) usage("no output file name base specified");

  std::cout << MINI_TERMINAL_BLUE
            << "loading OBJ model from " << inFileName
            << MINI_TERMINAL_DEFAULT << std::endl;

  mini::Scene::SP scene = mini::loadOBJ(inFileName);
    
  std::cout << MINI_TERMINAL_DEFAULT
            << "done importing; saving to " << outFileName
            << MINI_TERMINAL_DEFAULT << std::endl;
  scene->save(outFileName);
  std::cout << MINI_TERMINAL_LIGHT_GREEN
            << "scene saved"
            << MINI_TERMINAL_DEFAULT << std::endl;
  return 0;
}
