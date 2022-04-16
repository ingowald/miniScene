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

#include "Material.h"

namespace mini {
    
  struct Material {
    typedef std::shared_ptr<Material> SP;

    Material() = default;
    Material(const Material &) = default;

    inline static SP create() { return std::make_shared<Material>(); }
    
    SP clone() const { return std::make_shared<Material>(*this); }
    
    bool isEmissive() const { return reduce_max(emission) != 0.f; }
    
    vec3f emission     { 0.f,0.f,0.f };
    /* default base color is what every object _without_ a proper base
       color in the input will get to lok like */
    vec3f baseColor    { .5f,.5f,.5f };
    float metallic     { 0.f };
    float roughness    { 0.f };
    float transmission { 0.f };
    float ior          { 1.45f };
    
    std::shared_ptr<Texture> colorTexture;
    std::shared_ptr<Texture> alphaTexture;
  };

  /*! a typical triangle mesh that mesh embree and optix mesh requirements */
  struct Mesh {
    typedef std::shared_ptr<Mesh> SP;
    
    Mesh(Material::SP material = {}) : material(material) {}

    inline static SP create(Material::SP material = {}) { return std::make_shared<Mesh>(material); }
    
    virtual bool   isEmissive() const { return material->isEmissive(); }
    virtual size_t getNumPrims() const { return indices.size(); }
    virtual box3f  getPrimBounds(size_t primID,
                                 const AffineSpace3f &xfm = affine3f()) const;
    
    box3f getBounds() const;
    
    std::vector<vec3f> vertices;
    std::vector<vec3f> normals;
    std::vector<vec2f> texcoords;
    std::vector<vec3i> indices;

    Material::SP       material;
  };

  /*! an object is a collection of one or more meshes. note it is
    _absoltely_ possible that some of the 'meshes' will be null;
    we do this intentionally to allow for maintaining a global
    indexing of all meshes even in cases where a scene gets split
    across multiple nodes, and seom of these nodes do not have all
    meshes. Ie, if a scene with meshes {A,B,C} is split to two
    nodes such that one node gets {A,C} and one gets {B}, then one
    will sore {A,null,C}, and the other {null,B,null} - this way
    both nodes will know exactly what it means if a hit indices
    that mesh "#1" was hit. */
  struct Object {
    typedef std::shared_ptr<Object> SP;
    
    inline static SP create(const std::vector<Mesh::SP> &meshes={})
    { return std::make_shared<Object>(meshes); }

    Object(const std::vector<Mesh::SP> &meshes={})
      : meshes(meshes)
    {}
    
    box3f getBounds() const;
    
    /*! list of all geometries in this object. if this object is in
      a partial scene / extracted sub-scene this array will
      *still* contain the same number of entries as the scene it
      was extracted from, just some of its elements might be
      empty */
    std::vector<Mesh::SP> meshes;
  };

  struct Instance {
    typedef std::shared_ptr<Instance> SP;

    inline Instance(Object::SP object = 0, const affine3f &xfm = affine3f())
      : object(object), xfm(xfm)
    {}

    inline static SP create(Object::SP object = 0, const affine3f &xfm = affine3f())
    { return std::make_shared<Instance>(object,xfm); }

    box3f getBounds() const;
    
    affine3f   xfm;
    Object::SP object;
  };

  struct
// #ifdef __CUDACC__
//   __align__(16)
// #endif
    QuadLight {
    vec3f corner, edge0, edge1, emission;
    /*! normal of this lights source; this could obviously be derived
        from cross(edge0,edge1), but is handle to have in a
        renderer */
    vec3f normal;
    /*! area of this lights source; this could obviously be derived
        from cross(edge0,edge1), but is handle to have in a
        renderer */
    float area;
  };

  /*! directional light at infinity, shining *into* given direction,
      with given radiance */
  struct DirLight {
    std::string toString();
    vec3f direction;
    vec3f radiance;
  };

  struct Film {
    typedef std::shared_ptr<Film> SP;
    vec2i resolution;
  };
      
  struct Camera {
    typedef std::shared_ptr<Camera> SP;
    affine3f    frame;
    float       fov;
    std::string name;
  };

  struct EnvMapLight {
    typedef std::shared_ptr<EnvMapLight> SP;

    static SP create() { return std::make_shared<EnvMapLight>(); }
    
    Texture::SP texture;
    affine3f    transform;
  };

  struct Scene {
    typedef std::shared_ptr<Scene> SP;

    Scene(const std::vector<Instance::SP> &instances={})
      : instances(instances)
    {}
    
    inline static SP create(const std::vector<Instance::SP> &instances={})
    { return std::make_shared<Scene>(instances); }
    
    box3f getBounds() const;
    
    static Scene::SP load(const std::string &fileName);
    void save(const std::string &fileName);
      
    std::vector<QuadLight>  quadLights;
    std::vector<DirLight>   dirLights;
    EnvMapLight::SP         envMapLight;
    
    std::vector<Instance::SP> instances;
  };

  inline box3f xfmBox(const affine3f &xfm, const box3f &box)
  {
    box3f result;
    for (int iz=0;iz<2;iz++)
      for (int iy=0;iy<2;iy++)
        for (int ix=0;ix<2;ix++) {
          vec3f v(ix ? box.upper.x : box.lower.x,
                  iy ? box.upper.y : box.lower.y,
                  iz ? box.upper.z : box.lower.z);
          result.extend(xfmPoint(xfm,v));
        }
    return result;
  }
  
} // ::mini
