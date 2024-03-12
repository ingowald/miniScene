// ======================================================================== //
// Copyright 2018++ Ingo Wald                                               //
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

#include "miniScene/Scene.h"
#include "miniScene/Serialized.h"
#include <fstream>
#include <random>
#include <cmath>

namespace mini {
  namespace scene {

    void addEnvLight(int ac, char **av)
    {
      std::string outFileName = "a.obj";
      std::string inFileName = "";
      std::string fileWithLightFileName = "";
      bool zup = false;
      for (int i=1;i<ac;i++) {
        std::string arg = av[i];
        if (arg == "-o") {
          outFileName = av[++i]; 
        } else if (arg == "--file-with-light") {
          fileWithLightFileName = av[++i]; 
        } else if (arg == "-zup") {
          zup = true;
        } else if (arg[0] != '-') {
          inFileName = arg;
        } else
          throw std::runtime_error("unknown cmdline argument '"+arg+"'");
      }
      if (inFileName.empty())
        throw std::runtime_error("no input file specified");
      if (fileWithLightFileName.empty())
        throw std::runtime_error("no file with env-light specified (./miniSetEnvLight inFile.mini --file-with-light fileWithLight.mini -o outFile.mini");

      Scene::SP model = Scene::load(inFileName);
      Scene::SP withLight = Scene::load(fileWithLightFileName);
      model->envMapLight = withLight->envMapLight;

      if (zup) {
        // std::swap(model->envMapLight->transform.l.vy,
        //           model->envMapLight->transform.l.vz);

        model->envMapLight->transform = owl::common::frame(vec3f(1,0,0));
        model->envMapLight->transform.l.vz = vec3f(1,0,0);// = rcp(owl::common::frame(vec3f(0,1,0)));
        model->envMapLight->transform.l.vx = vec3f(0,1,0);// = rcp(owl::common::frame(vec3f(0,1,0)));
        model->envMapLight->transform.l.vy = vec3f(0,0,1);// = rcp(owl::common::frame(vec3f(0,1,0)));
      }
      //srand48(128);

      model->save(outFileName);
    }
    
  }
}

int main(int ac, char **av)
{ mini::scene::addEnvLight(ac,av); return 0; }

