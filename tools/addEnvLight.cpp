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

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


namespace mini {
  namespace scene {

    Scene::SP loadEnvMap(const std::string &fileName)
    {
      if (fileName.substr(fileName.size()-5) == ".mini")
        return Scene::load(fileName);

      Scene::SP scene = Scene::create();
      vec2i dims = -1;
      int numChannels = -1;
      vec3f *texels = (vec3f*)stbi_loadf(fileName.c_str(),
                                         &dims.x, &dims.y, &numChannels, 0);

      Texture::SP texture = Texture::create();
      texture->format = Texture::FLOAT4;
      texture->size   = dims;
      texture->data.resize(dims.x*dims.y*sizeof(vec4f));
      for (int i=0;i<dims.x*dims.y;i++) 
        ((vec4f*)texture->data.data())[i] =
          vec4f(texels[i].x,
                texels[i].y,
                texels[i].z,
                0.f);
      EnvMapLight::SP envMapLight = EnvMapLight::create();
      envMapLight->texture = texture;
      scene->envMapLight = envMapLight;

      return scene;
    }

    void usage(const std::string error)
    {
      std::cout << "Error: " << error << std::endl;
      std::cout << "Usage: ./miniAddEnvLight -l lightFile.hdr [-yup] [-m existing.mini] -o out.mini" << std::endl;
      exit(1);
    }
    
    void addEnvLight(int ac, char **av)
    {
      vec3f up { 0.f, 0.f, 1.f };
      vec3f direction { 1.f, 0.f, 0.f };
      std::string outFileName = "a.mini";
      std::string inMiniFileName = "";
      std::string inLightFileName = "";
      // std::string fileWithLightFileName = "";
      for (int i=1;i<ac;i++) {
        std::string arg = av[i];
        if (arg == "-o") {
          outFileName = av[++i]; 
        // } else if (arg == "--file-with-light") {
        //   fileWithLightFileName = av[++i]; 
        } else if (arg == "-y" || arg == "-yup") {
          direction = {-1.f, 0.f, 0.f };
          up        = { 0.f, 1.f, 0.f };
        } else if (arg == "-Y" || arg == "-Yup") {
          direction = { 1.f, 0.f, 0.f };
          up        = { 0.f, 1.f, 0.f };
        } else if (arg == "-m") {
          inMiniFileName = av[++i];
        } else if (arg == "-l") {
          inLightFileName = av[++i];
        } else
          usage("unknown cmdline argument '"+arg+"'");
      }
      // if (fileWithLightFileName.empty())
      //   throw std::runtime_error("no file with env-light specified (./miniSetEnvLight inFile.mini --file-with-light fileWithLight.mini -o outFile.mini");

      if (inLightFileName.empty())
        usage("no light specified");
      
      Scene::SP model
        = inMiniFileName.empty()
        ? mini::Scene::create()
        : Scene::load(inMiniFileName);
      Scene::SP withLight
        = loadEnvMap(inLightFileName);
        // = Scene::load(fileWithLightFileName);
      model->envMapLight = withLight->envMapLight;
      auto &toWorld = model->envMapLight->transform.l;
      toWorld.vz = normalize(up);
      toWorld.vy = normalize(cross(toWorld.vz,direction));
      toWorld.vx = normalize(cross(toWorld.vy,toWorld.vz));

      model->save(outFileName);
    }
    
  }
}

int main(int ac, char **av)
{ mini::scene::addEnvLight(ac,av); return 0; }

