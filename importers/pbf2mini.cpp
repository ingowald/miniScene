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
#include "cup_tools/tessellateCurves.h"
#include "cup_tools/Lights.h"
#include "cup_tools/removeAllNonMeshShapes.h"
#include "pbrtParser/Scene.h"
#include <set>
#include <map>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION 1
#include "stb/stb_image.h"
#if HAVE_OPENEXR
#include <OpenEXR/ImfNamespace.h>
# include "OpenEXR/ImfArray.h"
# include "OpenEXR/ImfRgba.h"
# include "OpenEXR/ImfRgbaFile.h"
#endif

namespace mini {

#if HAVE_OPENEXR
  using namespace Imf;
  using namespace OPENEXR_IMF_NAMESPACE;
#endif

  const int quadsPerRowOfPtexAtlas = 256;
  
  std::string texturePath;
  
  void usage(const std::string &msg)
  {
    if (!msg.empty()) std::cerr << std::endl << "***Error***: " << msg << std::endl << std::endl;
    std::cout << "Usage: ./pbf2brix inFile.pbf -o outfile.mini [-t <path-to-textures>]" << std::endl;
    std::cout << "Imports a pbrt-parser/PBF file into brix's scene format.\n";
    std::cout << "(from where it can then be partitioned and/or rendered)\n";
    exit(msg != "");
  }

  // std::map<pbrt::Texture::SP, Texture::SP>  knownTextures;
  std::map<pbrt::Object::SP,  Object::SP>   knownObjects;
  std::map<pbrt::Material::SP,Material::SP> knownMaterials;
  // std::map<std::string,std::pair<Texture::SP,int>> knownPtexes;

  Texture::SP loadEXR(const std::string &fileName)
  {
    std::cout << "trying to read EXR file " << fileName << std::endl;
#if HAVE_OPENEXR
    PING;
    Array2D<Rgba> pixels;
    RgbaInputFile file(fileName.c_str());
    Imath::Box2i dw = file.dataWindow();
    int width = dw.max.x - dw.min.x + 1;
    int height = dw.max.y - dw.min.y + 1;
    pixels.resizeErase(height, width);
    file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
    file.readPixels(dw.min.y, dw.max.y);
    Texture::SP texture = Texture::create();
    texture->size = { width, height };
    texture->format = Texture::FLOAT4;
    texture->data.resize(width*height*sizeof(vec4f));
    vec4f *v4 = (vec4f*)texture->data.data();
    for (int iy=0;iy<height;iy++) {
      for (int ix=0;ix<width;ix++) {
        auto texel = pixels[iy][ix];
        v4->x = texel.r;
        v4->y = texel.g;
        v4->z = texel.b;
        v4->w = texel.a;
        ++v4;
      }
    }
    return texture;
#else
    std::cout << OWL_TERMINAL_RED
              << "found EXR texture, but openexr support not compiled in ... skipping"
              << OWL_TERMINAL_DEFAULT << std::endl;
    return {};
#endif
  }

  Texture::SP getOrLoadTexture(const std::string &_fileName)
  {
    static std::map<std::string,Texture::SP>         knownTextures;
    const std::string fileName = texturePath+"/"+_fileName;
    if (knownTextures.find(fileName) != knownTextures.end())
      return knownTextures[fileName];

    Texture::SP texture = std::make_shared<Texture>();
    if (mini::common::endsWith(fileName,".png")) {
      vec2i  size;
      int    numChannels = 0;;
      unsigned char *texels = 0;
      texels = stbi_load(fileName.c_str(),
                         &size.x,
                         &size.y,
                         &numChannels,
                         STBI_rgb_alpha);
      if (texels) {
        std::cout << OWL_TERMINAL_GREEN
                  << "successfully read image file " << fileName
                  << OWL_TERMINAL_DEFAULT << std::endl;
      // std::cout << " -> read image texture " << fileName << std::endl;
      switch(numChannels) {
      case 1: {
        // note we're actually wasing a lot here - the "STBI_rgb_alpha"
        // will always load four bytes per texel, even though this would
        // only have the fourth channel set to anything other than 0
        texture->format = Texture::RGBA_UINT8;
        texture->size   = size;
        texture->data.resize(size.x*size.y*sizeof(uint32_t));
        uint32_t *in  = (uint32_t*)texels;
        uint32_t *out = (uint32_t*)texture->data.data();
        for (int y=0;y<size.y;y++) {
          for (int x=0;x<size.x;x++) {
            out[(y)*size.x+x] = in[y*size.x+x];
            // out[(size.y-1-y)*size.x+x] = in[y*size.x+x];
          }
        }
      } break;
      case 3: 
      case 4: {
        // note the 'numChannels=3' only means that the _input_ had only
        // three channels - the loader "STBI_rgb_alpha" actually loads
        // four, and automatically sets and alpha channel of 0xFF in
        // that fourth component
        texture->format = Texture::RGBA_UINT8;
        texture->size   = size;
        texture->data.resize(size.x*size.y*sizeof(uint32_t));
        uint32_t *in  = (uint32_t*)texels;
        uint32_t *out = (uint32_t*)texture->data.data();
        for (int y=0;y<size.y;y++) {
          for (int x=0;x<size.x;x++) {
            out[(y)*size.x+x] = in[y*size.x+x];
            // out[(size.y-1-y)*size.x+x] = in[y*size.x+x];
          }
        }
      } break;
      default:
        PING;
        PRINT(numChannels);
        throw std::runtime_error("unsupported number of channels in texture loader...");
      }}
      else {
        std::cout << OWL_TERMINAL_RED
                  << " -> FAILED trying to load image texture " << fileName
                  << OWL_TERMINAL_DEFAULT << std::endl;
      }
    } else if (mini::common::endsWith(fileName,".exr")) {
      texture = loadEXR(fileName);
    } else
      throw std::runtime_error("unsupported texture type "+fileName);
    
    if (!texture) {
      // throw std::runtime_error("could not load texture ....");
      std::cout << OWL_TERMINAL_RED
                << " -> FAILED trying to load image texture " << fileName
                << OWL_TERMINAL_DEFAULT << std::endl;
      return {};
    }
    
    knownTextures[fileName] = texture;
    return texture;
  }

  Texture::SP getOrLoadPtex(const std::string &_fileName)
  {
    static std::map<std::string,Texture::SP>         knownTextures;
    const std::string fileName = texturePath+"/"+_fileName;
    if (knownTextures.find(fileName) != knownTextures.end())
      return knownTextures[fileName];

    std::ifstream file(texturePath+_fileName,std::ios::binary);
    Texture::SP tex;
    if (file.good()) {
      file.seekg(0, std::ios_base::end); 
      size_t numBytes = file.tellg();
      file.seekg(0, std::ios_base::beg);
      tex = std::make_shared<Texture>();
      tex->format = Texture::EMBEDDED_PTEX;
      tex->size = 0;
      tex->data.resize(numBytes);
      file.read((char*)tex->data.data(),tex->data.size());
      std::cout << OWL_TERMINAL_GREEN
                << " -> read and *embedded* ptex texture " << _fileName
                << OWL_TERMINAL_DEFAULT << std::endl;
    }
    else {
      std::cout << OWL_TERMINAL_RED
                << " -> FAILED trying to read and embed ptex texture " << fileName
                << OWL_TERMINAL_DEFAULT << std::endl;
    }

    knownTextures[fileName] = tex;
    return tex;
  }
  
  void setImageMap(Material::SP material, pbrt::Texture::SP texture)
  {
    if (!texture) return;
    pbrt::ImageTexture::SP image
      = texture->as<pbrt::ImageTexture>();
    pbrt::ConstantTexture::SP constant
      = texture->as<pbrt::ConstantTexture>();
    pbrt::PtexFileTexture::SP ptex
      = texture->as<pbrt::PtexFileTexture>();
    if (image) {
      // ours->baseColor = (const vec3f&)image->value;
      std::cout << " -> got image texture " << image->fileName << std::endl;
      assert(image);
      material->colorTexture
        = getOrLoadTexture(image->fileName);
    } else if (constant) {
      material->baseColor = (const vec3f&)constant->value;
      std::cout << " -> got constant texture " << material->baseColor << std::endl;
    } else if (ptex) {
      material->colorTexture
        = getOrLoadTexture(image->fileName);
      std::cout << " -> got (embedded) PTEX texture " << material->baseColor << std::endl;
    } else
      std::cout << OWL_TERMINAL_RED << " unsupported texture ... "
                << texture->toString()
                << OWL_TERMINAL_DEFAULT << std::endl;
  }
  
  Material::SP parseAsUber(pbrt::Material::SP material)
  {
    pbrt::UberMaterial::SP uber = material->as<pbrt::UberMaterial>();
    if (!uber) return {};
    
    std::cout << "creating a UBER material (will only work for velvet-like stuff for now...." << std::endl;
    Material::SP ours = std::make_shared<Material>();
    ours->baseColor    = (const vec3f&)uber->kd;
    ours->metallic     = 0.f;
    ours->roughness    = uber->roughness;
    ours->transmission = 0.f;
    ours->ior          = 1.f;
    setImageMap(ours,uber->map_kd);
    // if (uber->map_kd)
    //   std::cout << "UBER MATERIAL: IMAGE MAP: " << uber->map_kd->toString() << std::endl;
    return ours;
  }

  Material::SP parseAsMix(pbrt::Material::SP material)
  {
    pbrt::MixMaterial::SP mix = material->as<pbrt::MixMaterial>();
    if (!mix) return {};
    
    std::cout << "creating a MIX material (will only work for landscape right now)...." << std::endl;

    /* parse this as a "landscape" material - in landscape, all mix
       materials are a combination of uber (for the reflective part)
       and translucent (for the translucent part), with nothing
       else */
    
    Material::SP lsUber = parseAsUber(mix->material0);
    pbrt::TranslucentMaterial::SP
      translucent = mix->material1->as<pbrt::TranslucentMaterial>();
    if (!lsUber || !translucent) {
      std::cout << OWL_TERMINAL_RED
                << "this is a mix material, but not one that mixes "
                << "uber+translucent (as in landscape .... huh!?" 
                << OWL_TERMINAL_DEFAULT << std::endl;
      return {};
    }
    
    vec3f mixAmount = (const vec3f&)mix->amount;
    if (mix->map_amount)
      std::cout << OWL_TERMINAL_RED
                << "mix material has map_amount texture - not handled!?"
                << OWL_TERMINAL_DEFAULT << std::endl;
    
    Material::SP ours = std::make_shared<Material>();
    ours->roughness = lsUber->roughness;
    ours->ior = lsUber->ior;
    ours->transmission = (mixAmount.x+mixAmount.y+mixAmount.z)/3.f;
    ours->emission = 0.f;
    ours->metallic = lsUber->metallic;
    ours->baseColor = lsUber->baseColor;
    ours->colorTexture = lsUber->colorTexture;
    return ours;
  }

  Material::SP importMaterial(pbrt::Material::SP material)
  {
    if (!material) {
      Material::SP dummyMat = std::make_shared<Material>();
      dummyMat->baseColor = vec3f(.6f);///randomColor(size_t(dummyMat.get()));
      return dummyMat;
    }

    pbrt::DisneyMaterial::SP disney = material->as<pbrt::DisneyMaterial>();
    if (disney) {
      static int count = 0;
      if (!count++)
        std::cout << "creating at least one DISNEY material ...." << std::endl;
      Material::SP ours = std::make_shared<Material>();
      ours->baseColor    = (const vec3f&)disney->color;
      ours->metallic     = disney->metallic;
      ours->roughness    = disney->roughness;
      ours->transmission = disney->specTrans;
      ours->ior          = disney->eta;
      return ours;
    }

    pbrt::GlassMaterial::SP glass = material->as<pbrt::GlassMaterial>();
    if (glass) {
      std::cout << "creating a GLASS material ...." << std::endl;
      Material::SP ours = std::make_shared<Material>();
      ours->baseColor    = (const vec3f&)glass->kt;
      ours->metallic     = 0.f;
      ours->roughness    = 0.f;
      ours->transmission = 1.f;
      ours->ior = glass->index;
      return ours;
    }

    pbrt::MetalMaterial::SP metal = material->as<pbrt::MetalMaterial>();
    if (metal) {
      std::cout << "creating a METAL material - making it all gold for now ...." << std::endl;
      Material::SP ours = std::make_shared<Material>();
      ours->baseColor    = vec3f(255,215,0)*(1.f/255.f);
      ours->metallic     = 1.f;
      ours->roughness    = metal->roughness;
      ours->transmission = 0.f;
      ours->ior          = 1.f;
      return ours;
    }

    pbrt::MatteMaterial::SP matte = material->as<pbrt::MatteMaterial>();
    if (matte) {
      std::cout << "creating a MATTE material ...." << std::endl;
      Material::SP ours = std::make_shared<Material>();
      ours->baseColor    = (const vec3f&)matte->kd;
      ours->metallic     = 0.f;
      ours->roughness    = 1.f;
      ours->transmission = 0.f;
      ours->ior          = 1.f;

      // pbrt ecosys: matte materials have image textures
      setImageMap(ours,matte->map_kd);
      return ours;
    }

    if (Material::SP asUber = parseAsUber(material)) {
      // asUber->alphaTexture = alphaTexture;
      return asUber;
    }

    if (Material::SP asMix = parseAsMix(material)) {
      // std::cout << "mix material w/ alpha tex " << alphaTexture << std::endl;
      // asMix->alphaTexture = alphaTexture;
      return asMix;
    }
    
    pbrt::PlasticMaterial::SP plastic = material->as<pbrt::PlasticMaterial>();
    if (plastic) {
      std::cout << "creating a PLASTIC material (will only work for velvet-like stuff for now...." << std::endl;
      Material::SP ours = std::make_shared<Material>();
      ours->baseColor    = (const vec3f&)plastic->kd;
      ours->metallic     = 0.f;
      ours->roughness    = plastic->roughness;
      ours->transmission = 0.f;
      ours->ior          = 1.f;

      // pbrt ecosys: plastic materials have constant textures...
      if (plastic->map_kd) {
        pbrt::ConstantTexture::SP constant
          = plastic->map_kd->as<pbrt::ConstantTexture>();
        if (constant) {
          ours->baseColor = (const vec3f&)constant->value;
          std::cout << " -> got constant tex kd = " << ours->baseColor << std::endl;
        } else
          std::cout << OWL_TERMINAL_RED << " unsupported texture ... "
                    << plastic->map_kd->toString()
                    << OWL_TERMINAL_DEFAULT << std::endl;
      }
      
      return ours;
    }
    
    pbrt::SubstrateMaterial::SP substrate = material->as<pbrt::SubstrateMaterial>();
    if (substrate) {
      std::cout << "creating a SUBSTRATE material (will only work for some things for now...." << std::endl;
      Material::SP ours = std::make_shared<Material>();
      ours->baseColor    = (const vec3f&)substrate->kd;
      ours->metallic     = 0.f;
      ours->roughness    = .5f*(substrate->uRoughness+substrate->vRoughness);
      ours->transmission = 0.f;
      ours->ior          = 1.f;
      return ours;
    }
    
    /* IGNORE FOR NOW */
    {
      static bool warned = false;
      if (!warned) {
        std::cout << OWL_TERMINAL_YELLOW
                  << "Warning: found at least one un-handled or empty material; "
                  << "creating dummy material!"
                  << OWL_TERMINAL_DEFAULT << std::endl;
        warned = true;
      }
    }
    
    Material::SP dummyMat = std::make_shared<Material>();
    // dummyMat->baseColor = randomColor(size_t(dummyMat.get()));
    return dummyMat;
  }


Texture::SP getShapeTexture(pbrt::Shape::SP pbrtShape, const std::string &key)
{
  // std::cout << "looking for shape texture '"+key+"'" << std::endl;
  auto it = pbrtShape->textures.find(key);
  if (it == pbrtShape->textures.end()) return {};

  // std::cout << "  FOUND texture " << it->second->toString() << std::endl;
  if (pbrt::ImageTexture::SP image
      = it->second->as<pbrt::ImageTexture>()) {
    auto tex = getOrLoadTexture(image->fileName);
    return tex;
  }
  
  if (pbrt::PtexFileTexture::SP ptex
      = it->second->as<pbrt::PtexFileTexture>()) {
    auto tex = getOrLoadPtex(ptex->fileName);
    return tex;
  }

  throw std::runtime_error("unknown kind of shape texture!?");
}
    


  /*! creates material for a given shape - but unlike for pbrt our
    materials do include textures and emission, so as input we need
    the entire shape, nor just the pbrt material */
  Material::SP importMaterial(pbrt::Shape::SP pbrtShape)
  {
    Material::SP material = importMaterial(pbrtShape->material);
    Texture::SP alphaTexture
      = material->alphaTexture
      ? material->alphaTexture
      : getShapeTexture(pbrtShape,"alpha");
    Texture::SP colorTexture
      = material->colorTexture
      ? material->colorTexture
      : getShapeTexture(pbrtShape,"color");
    typedef std::tuple<pbrt::Material::SP,Texture::SP,Texture::SP> key_t;
    static std::map<key_t,Material::SP>
      texturedMaterial;

    key_t key = {pbrtShape->material,colorTexture,alphaTexture};
    if (texturedMaterial.find(key) == texturedMaterial.end()) {
      Material::SP texMat = std::make_shared<Material>(*material);
      texMat->alphaTexture = alphaTexture;
      texMat->colorTexture = colorTexture;
      texturedMaterial[key] = texMat;
    }
    return texturedMaterial[key];
  }

  
  void importShape(pbrt::Shape::SP shape,
                   Object::SP ourObject)
  {
    if (!shape)
      return;
    
    pbrt::TriangleMesh::SP pbrtMesh = shape->as<pbrt::TriangleMesh>();
    if (!pbrtMesh) {
      std::cout
        << OWL_TERMINAL_RED
        << "#pbr2brx(warning): there seems to be a non-triangle mesh shape here!?"
        << OWL_TERMINAL_DEFAULT << std::endl;
      return;
    }

    if (pbrtMesh->index.empty()) return;
    
    Mesh::SP miniMesh = std::make_shared<Mesh>();
    miniMesh->material = importMaterial(pbrtMesh);
    
    for (auto idx : pbrtMesh->index)
      miniMesh->indices.push_back((const vec3i &)idx);
    for (auto vtx : pbrtMesh->vertex) 
      miniMesh->vertices.push_back((const vec3f &)vtx);
    for (auto nor : pbrtMesh->normal)
      miniMesh->normals.push_back((const vec3f &)nor);

    for (auto txt : pbrtMesh->texcoord) 
      miniMesh->texcoords.push_back((const vec2f &)txt);

    ourObject->meshes.push_back(miniMesh);
  }
  
  Object::SP importObject(pbrt::Object::SP object,
                          Scene::SP scene)
  {
    if (!object) return nullptr;
    if (knownObjects.find(object) != knownObjects.end())
      return knownObjects[object];
    
    Object::SP ourObject = std::make_shared<Object>();
    // std::cout << "num shapes in object " << object->shapes.size() << std::endl;
    for (auto shape : object->shapes)
      importShape(shape,ourObject);

    if (ourObject->meshes.empty()) return nullptr;

    knownObjects[object] = ourObject;
    return ourObject;
  }
  
  void importInstance(pbrt::Instance::SP inst,
                      Scene::SP scene)
  {
    if (!inst) return;
    if (!inst->object) return;

    Object::SP object = importObject(inst->object,scene);
    if (!object) return;

    Instance::SP ourInst = std::make_shared<Instance>();
    ourInst->object = object;
    ourInst->xfm   = (const owl::affine3f &)inst->xfm;
    scene->instances.push_back(ourInst);
  }


  mini::QuadLight transform(const mini::QuadLight &in, const affine3f &xfm)
  {
    mini::QuadLight out;
    out.emission = in.emission;
    out.corner = xfmPoint(xfm,in.corner);
    out.edge0 = xfmVector(xfm,in.edge0);
    out.edge1 = xfmVector(xfm,in.edge1);
    vec3f N = cross(out.edge0,out.edge1);
    out.area = length(N);
    if (out.area == 0.f) return in;
    out.normal = normalize(N);
    return out;
  }
  
  bool makeQuadLight(mini::QuadLight &quadLight,
                     pbrt::Shape::SP shape,
                     bool warn)
  {
    if (!shape) return false;
    
    pbrt::TriangleMesh::SP mesh = shape->as<pbrt::TriangleMesh>();
    if (!mesh) return false;

    if (!mesh->areaLight)
      return false;

    if (mesh->index.size() != 2) {
      if (warn)
        std::cout << OWL_TERMINAL_YELLOW
                  << "WARNING: found a triangle mesh area light source that is NOT"
                  << " a virtual quad light source (numtris != 2)!?"
                  << OWL_TERMINAL_DEFAULT << std::endl;
      return false;
    }
    
    // if (!mesh->material) {
    //   if (warn)
    //     std::cout << OWL_TERMINAL_YELLOW
    //               << "WARNING: found a triangle mesh area light source that is NOT"
    //               << " a virtual quad light source (no material)!?"
    //               << OWL_TERMINAL_DEFAULT << std::endl;
    //   return false;
    // }
    
    if (mesh->alpha != 0.f) {
      if (warn)
        std::cout << OWL_TERMINAL_YELLOW
                  << "WARNING: found a triangle mesh area light source that is NOT"
                  << " a virtual quad light source (alpha != 0)!?"
                  << OWL_TERMINAL_DEFAULT << std::endl;
      return false;
    }
    
    pbrt::DiffuseAreaLightRGB::SP light = mesh->areaLight->as<pbrt::DiffuseAreaLightRGB>();
    if (!light) {
      if (warn)
        std::cout << OWL_TERMINAL_YELLOW
                  << "WARNING: found a triangle mesh area light source that is NOT"
                  << " a virtual quad light source (alpha != 0)!?"
                  << OWL_TERMINAL_DEFAULT << std::endl;
      return false;
    }

    // ugh - we SHOULD actually test if this is a rectangle, but for now let's just assume it is...
    quadLight.corner = (const vec3f&)mesh->vertex[0];
    quadLight.edge0  = (const vec3f&)mesh->vertex[1] - quadLight.corner;
    quadLight.edge1  = (const vec3f&)mesh->vertex[3] - quadLight.corner;
    if (mesh->index[0].y == 2)
      // note this catches only ONE way of reverseing the quadlight-
      // works for moana, but probably not much else :-/
      std::swap(quadLight.edge0,quadLight.edge1);
    quadLight.emission = (const vec3f&)light->L;
    quadLight.area     = length(cross(quadLight.edge0,quadLight.edge1));
    if (quadLight.area == 0.f)
      return false;
    quadLight.normal   = normalize(cross(quadLight.edge0,quadLight.edge1));
    return true;
  }

  std::vector<mini::QuadLight> removeAllVirtualQuadLights(pbrt::Scene::SP g_scene)
  {
    std::vector<mini::QuadLight> quadLights;
    std::set<pbrt::Object::SP> objects;
    for (auto inst : g_scene->world->instances)
      if (inst && inst->object) {
        objects.insert(inst->object);
        for (auto shape : inst->object->shapes) {          
          mini::QuadLight quadLight;
          if (makeQuadLight(quadLight,shape,false))
            quadLights.push_back(transform(quadLight,(const affine3f &)inst->xfm));
        }
      }

    for (auto object : objects) {
      std::vector<pbrt::Shape::SP> noQuadLightShapes;
      for (auto shape : object->shapes) {
        mini::QuadLight quadLight;
        if (makeQuadLight(quadLight,shape,true))
          continue;
        else
          noQuadLightShapes.push_back(shape);
      }
      object->shapes = noQuadLightShapes;
    }
    return quadLights;
  }
  
  EnvMapLight::SP extractEnvMapLight(pbrt::Scene::SP inScene)
  {
    std::cout << "looking for envmap light in scene w/ " << inScene->world->lightSources.size() << " light sources in world object ..." << std::endl;
    for (auto light : inScene->world->lightSources) {
      std::cout << "- found light source " << light->toString() << std::endl;
      pbrt::InfiniteLightSource::SP asInf
        = light->as<pbrt::InfiniteLightSource>();
      if (!asInf) continue;

      EnvMapLight::SP ours = EnvMapLight::create();
      ours->transform = (const affine3f &)asInf->transform;
      ours->texture   = getOrLoadTexture(asInf->mapName);
      if (!ours->texture) {
        std::cout << OWL_TERMINAL_RED;
        std::cout << "******************************************************************" << std::endl;
        std::cout << "WARNING: COULD NOT READ ENV-MAP TEXTURE!!!" << std::endl;
        std::cout << "******************************************************************" << std::endl;
        std::cout << OWL_TERMINAL_DEFAULT;
        return {};
      }
      std::cout << OWL_TERMINAL_GREEN
                << " -> created env-map texture w/ embedded image texture " << asInf->mapName
                << OWL_TERMINAL_DEFAULT << std::endl;
      return ours;
    }
    return {};
  }
  
  std::vector<DirLight> extractDirLights(pbrt::Scene::SP inScene)
  {
    std::vector<DirLight> result;
    std::cout << "looking for directional light(s) in scene w/ " << inScene->world->lightSources.size() << " light sources in world object ..." << std::endl;
    for (auto light : inScene->world->lightSources) {
      std::cout << "- found light source " << light->toString() << std::endl;
      pbrt::DistantLightSource::SP asDir
        = light->as<pbrt::DistantLightSource>();
      if (!asDir) continue;

      DirLight ours;
      affine3f transform = (const affine3f &)asDir->transform;
      vec3f pbrtDir = (const vec3f&)asDir->to - (const vec3f&)asDir->from;
      ours.direction = xfmVector(transform,pbrtDir);
      ours.radiance  = (const vec3f&)asDir->L;
      if (ours.radiance == vec3f(0.f)) {
        std::cout << OWL_TERMINAL_YELLOW
                  << "light has zero radiance ... dropping"
                  << OWL_TERMINAL_DEFAULT << std::endl;
        continue;
      }
      std::cout << OWL_TERMINAL_GREEN
                << " -> created directional light w/ direction " << ours.direction
                << " and radiance " << ours.radiance 
                << OWL_TERMINAL_DEFAULT << std::endl;
      result.push_back(ours);
    }
    return result;
  }
  

  extern "C" int main(int ac, char **av)
  {
    std::string inFileName = "";
    std::string outFileName = "";
    
    for (int i=1;i<ac;i++) {
      const std::string arg = av[i];
      if (arg == "-o") {
        outFileName = av[++i];
      } else if (arg == "-t") {
        texturePath = av[++i];
      } else if (arg[0] != '-')
        inFileName = arg;
      else
        usage("unknown cmd line arg '"+arg+"'");
    }

    if (inFileName.empty()) usage("no input file name specified");
    if (outFileName.empty()) usage("no output file name base specified");

    std::cout << OWL_TERMINAL_BLUE
              << "trying to load PBRT scene from " << inFileName
              << OWL_TERMINAL_DEFAULT << std::endl;
    
    pbrt::Scene::SP inScene = pbrt::Scene::loadFrom(inFileName);
    inScene->makeSingleLevel();
    std::cout << OWL_TERMINAL_GREEN
              << "done loading PBRT scene; found "
              << inScene->world->instances.size() << " instances ..."
              << OWL_TERMINAL_DEFAULT << std::endl;

    std::cout << "tessellating curves (if applicable)" << std::endl;
    cup::tools::tessellateAllCurvesIn(inScene);
    std::cout << "tessellating all other non-mesh shapes (if applicable)" << std::endl;
    cup::tools::removeAllNonMeshShapes(inScene);

    // pbrt::Scene::SP masterScene = extractMasterScene();
    // cup::tools::removeAreaLightShapes(g_inScene);
    
    Scene::SP scene = std::make_shared<Scene>();
    scene->quadLights = removeAllVirtualQuadLights(inScene);
    std::cout << "#pbf2brx: found total of " << scene->quadLights.size() << " virtual quad lights"
              << std::endl;

    scene->envMapLight = extractEnvMapLight(inScene);
    scene->dirLights   = extractDirLights(inScene);
    for (auto inst : inScene->world->instances)
      importInstance(inst,scene);
    
    std::cout << OWL_TERMINAL_DEFAULT
              << "done importing; saving to " << outFileName
              << OWL_TERMINAL_DEFAULT << std::endl;
    scene->save(outFileName);
    std::cout << OWL_TERMINAL_LIGHT_GREEN
              << "scene saved"
              << OWL_TERMINAL_DEFAULT << std::endl;
    return 0;
  }

} // ::mini
