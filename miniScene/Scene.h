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
      
    typedef enum : uint16_t
      {
       UNDEFINED=0,
       EMBEDDED_PTEX,
       FLOAT4,
       FLOAT1,
       RGBA_UINT8, BYTE4=RGBA_UINT8, 
      } Format;
    typedef enum : uint16_t
      {
       /*! default filter mode - bilinear */
       FILTER_BILINEAR=0,
       /*! explicitly request nearest-filtering */
       FILTER_NEAREST
      } FilterMode;
    
    std::string toString() const;

    /* for embedded ptex, size is always {0,0}, since data vector
       containst the raw ptex fiel content; for all others it is the
       number of pixels in x and y */
    vec2i                size       { 0, 0 };

    /*! format to be used - _must_ be set */
    Format               format     { UNDEFINED };

    /*! filter mode that _should_ be used; usually we would exptect
      textures to use bilinear filtering, but in some cases it might
      be required to use nearest filtering, so let the app express
      that here */
    FilterMode           filterMode { FILTER_BILINEAR };

    /*! the actual raw texture data; what exactly that is depends on
      the format value */
    std::vector<uint8_t> data;
  };

  struct Material : public std::enable_shared_from_this<Material> {
    typedef std::shared_ptr<Material> SP;

    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Material() = default;

    virtual std::string toString() const = 0;
    
    virtual void write(std::ofstream &out,
                       const std::map<Texture::SP,int> &textures) = 0;
    virtual void read(std::ifstream &in,
                      const std::vector<Texture::SP> &textures) = 0;
    virtual Material::SP clone() const = 0;

    template<typename ActualMaterial>
    inline std::shared_ptr<ActualMaterial> as() 
    { return std::dynamic_pointer_cast<ActualMaterial>(shared_from_this()); }
    
    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Material(const Material &) = default;
  };
    
  /* blender style principled material - currently filled in with
     nvisii style material, which _should_ be fairly similar (nvisii
     source code says its material is based off blender's */
  struct BlenderMaterial : public Material {
    typedef std::shared_ptr<BlenderMaterial> SP;

    /*! constructs a new Material - note you _probably_ want to use
      Material::create() instead */
    BlenderMaterial() = default;
    
    /*! constructs a new Material - note you _probably_ want to use
      Material::create() instead */
    BlenderMaterial(const BlenderMaterial &) = default;
    
    /*! constructs a new Material and returns a Material::SP to that
      created material */
    inline static SP create() { return std::make_shared<BlenderMaterial>(); }
    
    /*! constructs a new Material that is a identical clone of the
      current material */
    Material::SP clone() const override { return std::make_shared<BlenderMaterial>(*this); }
    void write(std::ofstream &out,
               const std::map<Texture::SP,int> &textures) override;
    void read(std::ifstream &in,
              const std::vector<Texture::SP> &textures) override;
    std::string toString() const override { return "BlenderMaterial"; }
    
    vec3f baseColor              = { .8f, .8f, .8f };
    float roughness              = .5f;
    float metallic               = 0.f;
    float specular               = .5f;
    float specularTint           = 0.f;
    float transmission           = 0.f;
    float transmissionRoughness  = 0.f;
    float ior                    = 1.45f;
    float alpha                  = 1.0f;
    vec3f subsurfaceRadius       = { 1.f, .2f, .1f };
    vec3f subsurfaceColor        = { 0.8f, 0.8f, 0.8f };
    float subsurface             = 0.f;
    float anisotropic            = 0.f;
    float anisotropicRotation    = 0.f;
    float sheen                  = 0.f;
    float sheenTint              = 0.5f;
    float clearcoat              = 0.f;
    float clearcoatRoughness     = .03f;

    Texture::SP baseColorTexture;
    Texture::SP alphaTexture;
  };

    /* a Disney-style material that can represent both metallic,
     plastic, and dielectric materials. Not nearly as powerful as some
     of the more advanced material models out there, but can actually
     represent quite a bit of different content reasonably well - and
     is much easier to use than 20 different mroe specialized
     models */
  struct DisneyMaterial : public Material {
    typedef std::shared_ptr<DisneyMaterial> SP;

    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    DisneyMaterial() = default;
    
    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    DisneyMaterial(const DisneyMaterial &) = default;

    /*! constructs a new Material and returns a Material::SP to that
        created material */
    inline static SP create() { return std::make_shared<DisneyMaterial>(); }
    
    /*! constructs a new Material that is a identical clone of the
        current material */
    Material::SP clone() const override { return std::make_shared<DisneyMaterial>(*this); }
    void write(std::ofstream &out,
               const std::map<Texture::SP,int> &textures) override;
    void read(std::ifstream &in,
              const std::vector<Texture::SP> &textures) override;
    std::string toString() const override { return "DisneyMaterial"; }
    
    
    // bool isEmissive() const { return reduce_max(emission) != 0.f; }
    
    vec3f emission     { 0.f,0.f,0.f };
    /* default base color is what every object _without_ a proper base
       color in the input will get to lok like */
    vec3f baseColor    { .5f,.5f,.5f };
    float metallic     { 0.f };
    float roughness    { 0.f };
    float transmission { 0.f };
    float ior          { 1.45f };

    /*! color texture to be applied to the surface(s) that this
        material is being applied to; may be empty. If specified, this
        is supposed to replace the `baseColor` value */
    std::shared_ptr<Texture> colorTexture;
    
    /*! alpha texture to be applied to the surface(s) that this
        material is being applied to; may be empty. If specified, the
        'w' coordinate of the tex2D<float4> sample from this texture
        is supposed to replace the `transmission` value. Note that for
        some models this texture _can_ absoltely be the same texture
        as the colorTexture, in which case this will be a float4
        texturew with the xyz value going in as color value, and the
        'w' value as alpha value; other models may use a float3
        texture for color, and a separate float1 texture for alpha. */
    std::shared_ptr<Texture> alphaTexture;
  };

  struct Plastic : public Material {
    typedef std::shared_ptr<Plastic> SP;

    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Plastic() = default;
    
    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Plastic(const Plastic &) = default;

    /*! constructs a new Material and returns a Material::SP to that
        created material */
    inline static SP create() { return std::make_shared<Plastic>(); }
    
    /*! constructs a new Material that is a identical clone of the
        current material */
    Material::SP clone() const override { return std::make_shared<Plastic>(*this); }
    void write(std::ofstream &out,
               const std::map<Texture::SP,int> &textures) override;
    void read(std::ifstream &in,
              const std::vector<Texture::SP> &textures) override;
    std::string toString() const override { return "Plastic"; }


    vec3f Ks = { 1.f, 1.f, 1.f };
    float eta = 1.5f;
    vec3f pigmentColor { 0.05f, 0.05f, 0.05f };
    float roughness = 0.001f;
  };

  struct Metal : public Material {
    typedef std::shared_ptr<Metal> SP;

    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Metal() = default;
    
    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Metal(const Metal &) = default;

    /*! constructs a new Material and returns a Material::SP to that
        created material */
    inline static SP create() { return std::make_shared<Metal>(); }
    
    /*! constructs a new Material that is a identical clone of the
        current material */
    Material::SP clone() const override { return std::make_shared<Metal>(*this); }
    void write(std::ofstream &out,
               const std::map<Texture::SP,int> &textures) override;
    void read(std::ifstream &in,
              const std::vector<Texture::SP> &textures) override;
    std::string toString() const override { return "Metal"; }

    vec3f eta { 2.485f,2.485f,2.485f };
    vec3f k { 3.43f,3.43f,3.43f };
    float roughness { .1f };
  };

  struct Velvet : public Material {
    typedef std::shared_ptr<Velvet> SP;

    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Velvet() = default;
    
    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Velvet(const Velvet &) = default;

    /*! constructs a new Material and returns a Material::SP to that
        created material */
    inline static SP create() { return std::make_shared<Velvet>(); }
    
    /*! constructs a new Material that is a identical clone of the
        current material */
    Material::SP clone() const override { return std::make_shared<Velvet>(*this); }
    void write(std::ofstream &out,
               const std::map<Texture::SP,int> &textures) override;
    void read(std::ifstream &in,
              const std::vector<Texture::SP> &textures) override;
    std::string toString() const override { return "Velvet"; }

    vec3f reflectance { 0.55f, 0.0f, 0.0f };
    vec3f horizonScatteringColor { 0.75f, 0.2f, 0.2f };
    float horizonScatteringFallOff = 7.f;
    float backScattering = .5f;
  };

  struct Dielectric : public Material {
    typedef std::shared_ptr<Dielectric> SP;

    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Dielectric() = default;
    
    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Dielectric(const Dielectric &) = default;

    /*! constructs a new Material and returns a Material::SP to that
        created material */
    inline static SP create() { return std::make_shared<Dielectric>(); }
    
    /*! constructs a new Material that is a identical clone of the
        current material */
    Material::SP clone() const override { return std::make_shared<Dielectric>(*this); }
    void write(std::ofstream &out,
               const std::map<Texture::SP,int> &textures) override;
    void read(std::ifstream &in,
              const std::vector<Texture::SP> &textures) override;
    std::string toString() const override { return "Dielectric"; }

    float etaInside = 1.45f;
    float etaOutside = 1.f;
    vec3f transmission = .95;
  };
  struct ThinGlass : public Material {
    typedef std::shared_ptr<ThinGlass> SP;

    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    ThinGlass() = default;
    
    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    ThinGlass(const ThinGlass &) = default;

    /*! constructs a new Material and returns a Material::SP to that
        created material */
    inline static SP create() { return std::make_shared<ThinGlass>(); }
    
    /*! constructs a new Material that is a identical clone of the
        current material */
    Material::SP clone() const override { return std::make_shared<ThinGlass>(*this); }
    void write(std::ofstream &out,
               const std::map<Texture::SP,int> &textures) override;
    void read(std::ifstream &in,
              const std::vector<Texture::SP> &textures) override;
    std::string toString() const override { return "ThinGlass"; }
    
    float eta = 1.45f;
    float thickness = 1.f;
    vec3f transmission = .95;
  };
  struct MetallicPaint : public Material {
    typedef std::shared_ptr<MetallicPaint> SP;

    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    MetallicPaint() = default;
    
    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    MetallicPaint(const MetallicPaint &) = default;

    /*! constructs a new Material and returns a Material::SP to that
        created material */
    inline static SP create() { return std::make_shared<MetallicPaint>(); }
    
    /*! constructs a new Material that is a identical clone of the
        current material */
    Material::SP clone() const override { return std::make_shared<MetallicPaint>(*this); }
    void write(std::ofstream &out,
               const std::map<Texture::SP,int> &textures) override;
    void read(std::ifstream &in,
              const std::vector<Texture::SP> &textures) override;
    std::string toString() const override { return "MetallicPaint"; }
    
    float eta = 1.45f;
    vec3f glitterColor { 0.055f, 0.16f, 0.25f };
    float glitterSpread = 0.025f;
    vec3f shadeColor { 0.f, 0.03f, 0.07f };
  };

  struct Matte : public Material {
    typedef std::shared_ptr<Matte> SP;

    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Matte() = default;
    
    /*! constructs a new Material - note you _probably_ want to use
        Material::create() instead */
    Matte(const Matte &) = default;

    /*! constructs a new Material and returns a Material::SP to that
        created material */
    inline static SP create() { return std::make_shared<Matte>(); }
    
    /*! constructs a new Material that is a identical clone of the
        current material */
    Material::SP clone() const override { return std::make_shared<Matte>(*this); }
    void write(std::ofstream &out,
               const std::map<Texture::SP,int> &textures) override;
    void read(std::ifstream &in,
              const std::vector<Texture::SP> &textures) override;
    std::string toString() const override { return "Matte"; }
    
    vec3f reflectance { 0.5f,0.5f,0.5f };
  };

  
  /*! a typical triangle mesh that mesh embree and optix mesh requirements */
  struct Mesh {
    typedef std::shared_ptr<Mesh> SP;
    
    Mesh(Material::SP material = {})
      : material(material ? material : DisneyMaterial::create())
    {}

    inline static SP create(Material::SP material = {})
    { return std::make_shared<Mesh>(material); }
    
    // bool   isEmissive() const { return material->isEmissive(); }
    size_t getNumPrims() const { return indices.size(); }

    /*! computes a bounding box over all the triangles in this mesh */
    box3f getBounds() const;

    /*! array of vertices */
    std::vector<vec3f> vertices;

    /*! one vertex normal per vertex; or empty */
    std::vector<vec3f> normals;
    
    /*! one texture coordinate per vertex; or empty */
    std::vector<vec2f> texcoords;
    
    /*! the vector containing the triangles' vertex indices */
    std::vector<vec3i> indices;

    /*! the material to be applied to this mesh */
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

    /*! creates a new Object for given set of meshes, and returns a
        Object::SP for that object */
    inline static SP create(const std::vector<Mesh::SP> &meshes={})
    { return std::make_shared<Object>(meshes); }

    /*! constructs a new Object for given set of meshes - note you
        _probably_ want to use Object::create() instead */
    Object(const std::vector<Mesh::SP> &meshes={})
      : meshes(meshes)
    {}
    
    /*! computes and returns the bounding box of this object, which is
        the bounding box over all the mshes that this object
        contains */
    box3f getBounds() const;
    
    /*! list of all geometries in this object. if this object is in
      a partial scene / extracted sub-scene this array will
      *still* contain the same number of entries as the scene it
      was extracted from, just some of its elements might be
      empty */
    std::vector<Mesh::SP> meshes;
  };

  /*! represents instances of objects, with an affine transformation matrix */
  struct Instance {
    typedef std::shared_ptr<Instance> SP;

    inline Instance(Object::SP object = 0, const affine3f &xfm = affine3f())
      : object(object), xfm(xfm)
    {}

    inline static SP create(Object::SP object = 0, const affine3f &xfm = affine3f())
    { return std::make_shared<Instance>(object,xfm); }

    /*! computes and returns the world-space bounding box of this instance */
    box3f getBounds() const;
    
    affine3f   xfm;
    Object::SP object;
  };

  /*! a quadrilateral area light that emits light into the direction
      pointed to by the normal. The light shape is given by an
      "anchor" point describing one of the corners of the light
      soruces, and two edges spanning the quadrilateral from that
      point */
  struct QuadLight {
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

  /*! a directional light at infinity, shining *into* given direction,
      with a given radiance */
  struct DirLight {
    std::string toString();
    vec3f direction;
    vec3f radiance;
    
    // /*! cone angle (in radian) in which the light has full radiance */
    // float angleInner = 0.f;
    
    // /*! cone angle (in radian) up to which light linearly falls off to
    //     zero (must be >= angleInner) */
    // float angleOuter = 0.f;
  };

#if 0
  /*! not currently supported in this version */
  struct Film {
    typedef std::shared_ptr<Film> SP;
    vec2i resolution;
  };
      
  /*! not currently supported in this version */
  struct Camera {
    typedef std::shared_ptr<Camera> SP;
    affine3f    frame;
    float       fov;
    std::string name;
  };
#endif

  /*! A "environment light" texture that in many models represents
      some kind of scanned sky dome. Is often of HDR float4 type, and
      _may_ contain a transform to properly align the scanned texture
      to the model */
  struct EnvMapLight {
    typedef std::shared_ptr<EnvMapLight> SP;

    static SP create() { return std::make_shared<EnvMapLight>(); }
    
    Texture::SP texture;
    affine3f    transform;
  };

  /*! a complete scene, consisting of a list of instances (may be a
      single one if the scene doesn't use instantiation), and some
      light sources */
  struct Scene {
    typedef std::shared_ptr<Scene> SP;

    /*! constructor that creates a new scene object from the given
        vector of instances - note you _probably_ want to use Scene::create()
        instead */
    Scene(const std::vector<Instance::SP> &instances={})
      : instances(instances)
    {}

    /*! creates a new scene object from the given vector of instances */
    inline static SP create(const std::vector<Instance::SP> &instances={})
    { return std::make_shared<Scene>(instances); }
  
    /*! computes and returns the world space bounding box of this
        scene. Note that for scene with many instances that can take a
        while */
    box3f getBounds() const;

    /*! loads a ".mini" file from the given file */
    static Scene::SP load(const std::string &fileName);

    /*! saves the model in file with given name, using a binary file
        format that can be loaded with Scene::load() */
    void save(const std::string &fileName);
      
    std::vector<QuadLight>  quadLights;
    std::vector<DirLight>   dirLights;
    EnvMapLight::SP         envMapLight;
    
    std::vector<Instance::SP> instances;
  };

  /*! helper function for computing the bounding box of an affinely
      transformed box3f; usually used to compute the world-space
      bounding box of an instance (given that instance's affine
      transform matrix and the object-space bounding box of the object
      being instantiated) */
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
