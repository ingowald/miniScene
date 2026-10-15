// ======================================================================== //
// Copyright 2022++ Ingo Wald                                               //
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

namespace mini {

  void usage(const std::string &error)
  {
    if (!error.empty()) std::cout << "Error: " << error << std::endl;
    std::cout << "usage: ./makeQuadLightsTunnel <args> -o out.mini" << std::endl;
    std::cout << "Args:" << std::endl;
    std::cout << " -p0 x y z" << std::endl;
    std::cout << " -p1 x y z" << std::endl;
    std::cout << " --radius r" << std::endl;
    std::cout << " --color r g b" << std::endl;
    std::cout << " --emission <floatValue>" << std::endl;
    std::cout << " --num-seggments <intValue>" << std::endl;
    exit(error.empty()?0:1);
  }

  void makeQuadLightsTunnel(int ac, char **av)
  {
    vec3f p0(0,0,-10);
    vec3f p1(0,0,+10);
    float radius = 4;
    float emission = 10.f;
    vec3f color(1,1,1);
    int numSegs = 16;
    std::string outFileName = "";
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg == "-o")
        outFileName = av[++i];
      else if (arg == "-h") {
        usage("");
      } else if (arg == "--num-segments") {
        numSegs = std::stoi(av[++i]);
      } else if (arg == "-p0" || arg == "--p0") {
        float x = std::stof(av[++i]);
        float y = std::stof(av[++i]);
        float z = std::stof(av[++i]);
        p0 = {x,y,z};
      } else if (arg == "-p1" || arg == "--p1") {
        float x = std::stof(av[++i]);
        float y = std::stof(av[++i]);
        float z = std::stof(av[++i]);
        p1 = {x,y,z};
      } else if (arg == "--color") {
        float r = std::stof(av[++i]);
        float g = std::stof(av[++i]);
        float b = std::stof(av[++i]);
        color = vec3f(r,g,b);
      } else if (arg == "--emission") {
        float e = std::stof(av[++i]);
        emission = e;
      } else if (arg == "--radius" || arg == "-r") {
        float e = std::stof(av[++i]);
        radius = e;
      } else
        throw std::runtime_error("unknown cmdline argument '"+arg+"'");
    }
    if (outFileName.empty())
      throw std::runtime_error("no input file specified");

    Scene::SP scene = Scene::create();
    linear3f f = frame(normalize(p1-p0));
    affine3f xfm;
    xfm.p = p0;
    xfm.l.vx = radius * f.vx;
    xfm.l.vy = radius * f.vy;
    xfm.l.vz = length(p1-p0) * f.vz;
    for (int i=0;i<numSegs;i++) {
      float phi0 = i*2.f*M_PI / numSegs;
      float phi1 = (i+1)*2.f*M_PI / numSegs;
      // PRINT(phi0);
      // PRINT(phi1);
      float x0 = cosf(phi0);
      float y0 = sinf(phi0);
      float x1 = cosf(phi1);
      float y1 = sinf(phi1);
      vec3f v00 = vec3f(x0,y0,0.f);
      vec3f v01 = vec3f(x1,y1,0.f);
      vec3f v10 = vec3f(x0,y0,1.f);
      vec3f v11 = vec3f(x1,y1,1.f);
      // PRINT(v00);
      // PRINT(v01);
      // PRINT(v10);
      // PRINT(v11);

      v00 = xfmPoint(xfm,v00);
      v01 = xfmPoint(xfm,v01);
      v10 = xfmPoint(xfm,v10);
      v11 = xfmPoint(xfm,v11);

      mini::QuadLight ql;
      ql.corner = v00;
      ql.edge1  = v01-v00;
      ql.edge0  = v10-v00;
      // PRINT(ql.edge0);
      // PRINT(ql.edge1);
      ql.normal = cross(ql.edge0,ql.edge1);
      ql.area   = sqrtf(dot(ql.normal,ql.normal));
      ql.normal = normalize(ql.normal);
      ql.emission = color * emission;

      std::cout << "emitting quad light"
                << " p="<<ql.corner
                << " dx=" << ql.edge0
                << " dy=" << ql.edge1
                << " pow=" << ql.emission
                << std::endl;
      scene->quadLights.push_back(ql);
    }
    scene->save(outFileName);
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::makeQuadLightsTunnel(ac,av); return 0; }

