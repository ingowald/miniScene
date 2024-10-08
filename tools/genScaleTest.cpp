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

#ifdef _WIN32
#   include "owl/common/math/random.h"
#endif

namespace mini {
  int texRes = 8;
  bool funnies = true;
  int sphereRes = 10;

#ifdef _WIN32
  DRand48 drand48;
#endif  
  
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
    texture->filterMode = Texture::FILTER_NEAREST;
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
    
  Mesh::SP genSphere(vec3f center, float radius)
  {
    DisneyMaterial::SP material  = DisneyMaterial::create();
    material->baseColor    = rng3f();
    material->colorTexture = makeTexture();
    Mesh::SP mesh = Mesh::create(material);
    for (int i=0;i<=sphereRes;i++)
      for (int j=0;j<2*sphereRes;j++) {
        float fu = j/(2.f*sphereRes);
        float fv = i/float(sphereRes); 
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
    for (int j=0;j<sphereRes;j++)
      for (int i=0;i<2*sphereRes;i++) {
        int i00 = j*(2*sphereRes) + i;
        int i01
          = funnies
          ? j*(2*sphereRes) + ((j+1)%(2*sphereRes))
          : j*(2*sphereRes) + ((i+1)%(2*sphereRes));
        int i10 = i00 + 2*sphereRes;
        int i11 = i01 + 2*sphereRes;
        mesh->indices.push_back({i00,i01,i11});
        mesh->indices.push_back({i00,i11,i10});
      }
    return mesh;
  }
    
  void genScaleTest(int ac, char **av)
  {
    std::string outFileName = "genScaleTest.mini";
    int numInstances = 1;
    int numInstantiableSpheres = 100;
    int numBaseSpheres = 100;
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg == "-o")
        outFileName = av[++i];
      else if (arg == "-nf")
        funnies = false;
      else if (arg == "--sphere-res" || arg == "-sr")
        sphereRes = std::stoi(av[++i]);
      else if (arg == "--texture-res" || arg == "-tr")
        texRes = std::stoi(av[++i]);
      else if (arg == "--num-instances" || arg == "-ni")
        numInstances = std::stoi(av[++i]);
      else if (arg == "--num-base-spheres" || arg == "-nbs")
        numBaseSpheres = std::stoi(av[++i]);
      else if (arg == "--num-instantiable-spheres" || arg == "-nis")
        numInstantiableSpheres = std::stoi(av[++i]);
      else
        throw std::runtime_error("unknown cmdline argument '"+arg+"'");
    }
    vec3f domainSize = vec3f(1000,100,1000);
    int numVisibleSpheres = numInstances + numBaseSpheres;
    float avgRadius  = 200.f*powf(numVisibleSpheres,-1.f/3.f);

    Scene::SP scene = Scene::create();

    // ==================================================================
    std::cout << MINI_TERMINAL_BLUE
              << "generating 1 'not-instanced' base object with " << numBaseSpheres
              << " base spheres"
              << MINI_TERMINAL_DEFAULT << std::endl;
    std::vector<Mesh::SP> baseMeshes;
    for (int i=0;i<numBaseSpheres;i++) {
      float rad = (.5f+rng())*avgRadius;
      vec3f center = rng3f()*domainSize;
      baseMeshes.push_back(genSphere(center,rad));
    }
    scene->instances.push_back(Instance::create(Object::create(baseMeshes)));
    std::cout << MINI_TERMINAL_GREEN
              << "done."
              << MINI_TERMINAL_DEFAULT << std::endl;

    // ==================================================================
    std::vector<Object::SP> instantiableSpheres;
    std::cout << MINI_TERMINAL_BLUE
              << "creating total of " << numInstantiableSpheres
              << " different (unit) spheres to pick from for instantiation"
              << MINI_TERMINAL_DEFAULT << std::endl;
    for (int i=0;i<numInstantiableSpheres;i++) {
      Mesh::SP mesh = genSphere(vec3f(0.f),1.f);
      instantiableSpheres.push_back(Object::create({mesh}));
    }
    std::cout << MINI_TERMINAL_GREEN
              << "done."
              << MINI_TERMINAL_DEFAULT << std::endl;

    // ==================================================================
    std::cout << MINI_TERMINAL_BLUE
              << "creating total of " << numInstances
              << " different instances of randomly selected spheres"
              << MINI_TERMINAL_DEFAULT << std::endl;
    for (int i=0;i<numInstances;i++) {
      int whichSphere
        = (i<(int)instantiableSpheres.size())
        ? i
        : (int)(rng()*instantiableSpheres.size());
      whichSphere = std::min(whichSphere,int(instantiableSpheres.size()));
      float rad = (.5f+rng())*avgRadius;
      vec3f center = rng3f()*domainSize;
      vec3f orientation;
      while(1) {
        orientation = vec3f(1.f)-2.f*rng3f();
        float len = dot(orientation,orientation);
        if (len > .01f && len <= 1.f) break;
      } 
      orientation = normalize(orientation);
      // affine3f xfm
      //   = affine3f::scale(vec3f(rad))
      //   * affine3f(frame(orientation))
      //   * affine3f::translate(center);
      affine3f xfm
        = affine3f::translate(center)
        * affine3f(frame(orientation))
        * affine3f::scale(vec3f(rad));
      scene->instances.push_back(Instance::create(instantiableSpheres[whichSphere],xfm));
    }
    std::cout << MINI_TERMINAL_GREEN
              << "done."
              << MINI_TERMINAL_DEFAULT << std::endl;

    // ==================================================================
    std::cout << MINI_TERMINAL_BLUE << "saving to " << outFileName
              << MINI_TERMINAL_DEFAULT << std::endl;
    scene->save(outFileName);
    std::cout << MINI_TERMINAL_GREEN << "done." << outFileName
              << MINI_TERMINAL_DEFAULT << std::endl;
  }
  
} // ::mini

int main(int ac, char **av)
{ mini::genScaleTest(ac,av); return 0; }

