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
#include "miniScene/Serialized.h"

namespace mini {
  int texRes = 8;
  bool funnies = true;
  
  float rng()
  {
    return drand48();
    
  }
  vec3f rng3f() { return vec3f(rng(),rng(),rng()); }

  Texture::SP makeTexture()
  {
    if (texRes <= 0) return {};
    
    Texture::SP texture = Texture::create();
    texture->format = Texture::FLOAT4;
    texture->size = { texRes,texRes };
    texture->data.resize(texRes*texRes*sizeof(vec4f));
    vec4f *texel   = (vec4f*)texture->data.data();
    vec4f onColor  = vec4f(1.f);
    vec4f offColor = vec4f(rng(),rng(),rng(),1.f);
    for (int i=0;i<texRes;i++)
      for (int j=0;j<texRes;j++)
        *texel++
          = ((i^j)&1)
          ? onColor
          : offColor;
    return texture;
  }
    
  Mesh::SP genSphere(vec3f center, float radius, int nSegs)
  {
    Material::SP material  = Material::create();
    material->baseColor    = rng3f();
    material->colorTexture = makeTexture();
    Mesh::SP mesh = Mesh::create(material);
    for (int i=0;i<=nSegs;i++)
      for (int j=0;j<2*nSegs;j++) {
        float fu = j/(2.f*nSegs);
        float fv = i/float(nSegs); 
        float u = fu*2.f*M_PI;;
        float v = fv*M_PI;
        vec3f n;
        n.x = cos(u)*sin(v);
        n.y = sin(u)*sin(v);
        n.z = cos(v);
        mesh->normals.push_back(n);
        mesh->texcoords.push_back({fu,fv});
        mesh->vertices.push_back(center + n*radius);
      }
    for (int j=0;j<nSegs;j++)
      for (int i=0;i<2*nSegs;i++) {
        int i00 = j*(2*nSegs) + i;
        int i01
          = funnies
          ? j*(2*nSegs) + ((j+1)%(2*nSegs))
          : j*(2*nSegs) + ((i+1)%(2*nSegs));
        int i10 = i00 + 2*nSegs;
        int i11 = i01 + 2*nSegs;
        mesh->indices.push_back({i00,i01,i11});
        mesh->indices.push_back({i00,i11,i10});
      }
    return mesh;
  }
    
  void genScaleTest(int ac, char **av)
  {
    std::string outFileName = "genScaleTest.mini";
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg == "-o")
        outFileName = av[++i];
      else if (arg == "-nf")
        funnies = false;
      else if (arg == "-tr")
        texRes = std::stoi(av[++i]);
      else
        throw std::runtime_error("unknown cmdline argument '"+arg+"'");
    }
    int numSpheres   = 100;
    vec3f domainSize = vec3f(1000,20,1000);
    float avgRadius  = 500.f/sqrtf(numSpheres);

    std::vector<Mesh::SP> meshes;
    for (int i=0;i<numSpheres;i++) {
      float rad = (.5f+rng())*avgRadius;
      vec3f center = rng3f()*domainSize;
      meshes.push_back(genSphere(center,
                                 rad,10));
    }
    Object::SP object = Object::create(meshes);
    Scene::SP scene = Scene::create();
    scene->instances.push_back(Instance::create(object));

    std::cout << MINI_TERMINAL_BLUE << "saving to " << outFileName
              << MINI_TERMINAL_DEFAULT << std::endl;
    scene->save(outFileName);
    std::cout << MINI_TERMINAL_GREEN << "done." << outFileName
              << MINI_TERMINAL_DEFAULT << std::endl;
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::genScaleTest(ac,av); return 0; }

