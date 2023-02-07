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
#include "miniScene/IO.h"
#include <sstream>

namespace mini {

  enum { FORMAT_VERSION = 11 };
    
  const size_t expected_magic = 4321000000ULL+FORMAT_VERSION;
  const size_t oldFormat_magic = 4321000000ULL+FORMAT_VERSION;

  std::string DirLight::toString()
  {
    std::stringstream ss;
    ss << "DirLight{dir=" << direction << ", rad=" << radiance << "}";
    return ss.str();
  }
  
  box3f Mesh::getBounds() const
  {
    box3f bounds;
    for (auto vtx : vertices)
      bounds.extend(vtx);
    return bounds;
  }
    
  box3f Object::getBounds() const
  {
    box3f bounds;
    for (auto mesh : meshes)
      bounds.extend(mesh->getBounds());
    return bounds;
  }
    
  box3f Instance::getBounds() const
  {
    box3f bounds;
    box3f box = object->getBounds();
    for (int i=0;i<8;i++) {
      vec3f corner((i&1?box.lower:box.upper).x,
                   (i&1?box.lower:box.upper).y,
                   (i&1?box.lower:box.upper).z);
      bounds.extend(xfmPoint(xfm,corner));
    }
    return bounds;
  }
  box3f Scene::getBounds() const
  {
    box3f bounds;
    for (auto inst : instances)
      bounds.extend(inst->getBounds());
    return bounds;
  }

  enum {
        ENTITY_NULL = 0x1234567,
        ENTITY_INFOS,
        ENTITY_TEXTURES,
        ENTITY_IMAGE_TEXTURE
  } EntityType;
  
  size_t beginEntity(std::ostream &out, int tag);
  void endEntity(std::ostream &out, size_t save);
  
  void Scene::save(const std::string &baseName)
  {
    std::ofstream out(baseName,std::ios::binary);
    if (!out.good())
      throw std::runtime_error("could not open file '"+baseName+"'");
    SerializedScene serialized(this);

    io::writeElement(out,expected_magic);
    
    // ------------------------------------------------------------------
    // infos
    // ------------------------------------------------------------------
    {
      size_t ent = beginEntity(out,ENTITY_INFOS);
      io::writeElement(out,int(scene->infos.size()));
      for (auto info : infos)
        io::writeElement(info);
      endEntity(ent);
    }
    
    // ------------------------------------------------------------------
    // textures
    // ------------------------------------------------------------------
    {
      size_t ent = beginEntity(out,ENTITY_TEXTURES);
      io::writeElement(out,serialized.textures.list.size());
      for (auto tex : serialized.textures.list) {
        if (/* only first one may/will be null */!tex) {
          size_t ent = beginEntity(out,ENTITY_NULL);
          endEntity(ent);
          // io::writeElement(out,int(0));
        } else {
          size_t ent = beginEntity(out,ENTITY_IMAGE_TEXTURE);
          io::writeElement(out,int(1));
          io::writeElement(out,tex->size);
          io::writeElement(out,tex->format);
          io::writeElement(out,tex->filterMode);
          io::writeVector(out,tex->data);
          endEntity(ent);
        }
      }
      endEntity(ent);
    }
    // ------------------------------------------------------------------
    // lights
    // ------------------------------------------------------------------
    {
      io::writeVector(out,quadLights);
      endEntity(ent);
    }
    {
      io::writeVector(out,dirLights);
      endEntity(ent);
    }
    if (envMapLight) {
      io::writeElement(out,int(1));
      io::writeElement(out,envMapLight->transform);
      Texture::SP tex = envMapLight->texture;
      assert(tex);
      io::writeElement(out,tex->size);
      io::writeElement(out,tex->format);
      io::writeElement(out,tex->filterMode);
      io::writeVector(out,tex->data);
      endEntity(ent);
    } 
         
#if 0
    // ------------------------------------------------------------------
    // materials
    // ------------------------------------------------------------------
    io::writeElement(out,serialized.materials.list.size());
    for (auto mat : serialized.materials.list) {
      // io::writeElement(out,(MaterialData&)*mat);

      io::writeElement(out,mat->emission);
      io::writeElement(out,mat->baseColor);
      io::writeElement(out,mat->metallic);
      io::writeElement(out,mat->roughness);
      io::writeElement(out,mat->transmission);
      io::writeElement(out,mat->ior);

      io::writeElement(out,serialized.getID(mat->colorTexture));
      io::writeElement(out,serialized.getID(mat->alphaTexture));
    }
      
    // ------------------------------------------------------------------
    // objects and meshes
    // ------------------------------------------------------------------
    io::writeElement(out,serialized.objects.size());
    for (auto &obj : serialized.objects.list) {

      io::writeElement(out,obj->meshes.size());
      for (auto mesh : obj->meshes) {
        if (!mesh) { io::writeElement(out,int(0)); continue; }

        io::writeElement(out,int(1));
        io::writeVector(out,mesh->indices);
        io::writeVector(out,mesh->vertices);
        io::writeVector(out,mesh->normals);
        io::writeVector(out,mesh->texcoords);
        int matID = serialized.getID(mesh->material);
        assert(matID >= 0);
        io::writeElement(out,matID);
      }
    }

    // ------------------------------------------------------------------
    // instances
    // ------------------------------------------------------------------
    io::writeElement(out,instances.size());
    for (auto &inst : instances) {
      if (!inst) { io::writeElement(out,int(0)); continue; }

      io::writeElement(out,int(1));
      io::writeElement(out,inst->xfm);
      io::writeElement(out,int(serialized.getID(inst->object)));
    }
      
    // ------------------------------------------------------------------
    // proxies and owner masks
    // ------------------------------------------------------------------
    // io::writeVector(out,proxies);
    // io::writeVector(out,ownedOn);

    // ------------------------------------------------------------------
    // wrap-up: write end-of file marker
    // ------------------------------------------------------------------
#endif
    io::writeElement(out,expected_magic);
    if (!out.good())
      throw std::runtime_error("some error happened while writing '"+baseName+"'");
  }
    
  Scene::SP loadOldFormat(std::ifstream &in)
  {
    Scene::SP scene = std::make_shared<Scene>();
    // ------------------------------------------------------------------
    // textures
    // ------------------------------------------------------------------
    std::vector<Texture::SP> textures;
    size_t numTextures = io::readElement<size_t>(in);
    for (int i=0;i<numTextures;i++) {
      // if (i==0)
      //   textures.push_back(0); // first one is always 0
      // else {
      int valid;
      io::readElement(in,valid);
      if (!valid) {
        textures.push_back({});
      } else {
        Texture::SP tex = std::make_shared<Texture>();
        io::readElement(in,tex->size);
        io::readElement(in,tex->format);
        io::readElement(in,tex->filterMode);
        io::readVector(in,tex->data);
        textures.push_back(tex);
      }
    }

    // ------------------------------------------------------------------
    // lights
    // ------------------------------------------------------------------
    io::readVector(in,scene->quadLights);
    io::readVector(in,scene->dirLights);
    const int hasEnvMap = io::readElement<int>(in);
    if (hasEnvMap) {
      scene->envMapLight = std::make_shared<EnvMapLight>();
      io::readElement(in,scene->envMapLight->transform);
      Texture::SP tex = scene->envMapLight->texture = std::make_shared<Texture>();
      io::readElement(in,tex->size);
      io::readElement(in,tex->format);
      io::readElement(in,tex->filterMode);
      io::readVector(in,tex->data);
    }
    
    // ------------------------------------------------------------------
    // materials
    // ------------------------------------------------------------------
    std::vector<Material::SP> materials;
    size_t numMaterials = io::readElement<size_t>(in);
    for (int i=0;i<numMaterials;i++) {
      Material::SP mat = std::make_shared<Material>();
      // io::readElement(in,(MaterialData&)*mat);
      io::readElement(in,mat->emission);
      io::readElement(in,mat->baseColor);
      io::readElement(in,mat->metallic);
      io::readElement(in,mat->roughness);
      io::readElement(in,mat->transmission);
      io::readElement(in,mat->ior);
      {
        int texID = io::readElement<int>(in);
        assert(texID >= 0);
        assert(texID < textures.size());
        mat->colorTexture = textures[texID];
      }
      {
        int texID = io::readElement<int>(in);
        assert(texID >= 0);
        assert(texID < textures.size());
        mat->alphaTexture = textures[texID];
      }
      materials.push_back(mat);
    }

    // ------------------------------------------------------------------
    // objects and meshes
    // ------------------------------------------------------------------
    size_t numObjects = io::readElement<size_t>(in);
    std::vector<Object::SP> objects;
    for (int objID=0;objID<numObjects;objID++) {
      size_t numMeshes = io::readElement<size_t>(in);
      Object::SP object = std::make_shared<Object>();

      for (int meshID=0;meshID<(int)numMeshes;meshID++) {
        int isValid = io::readElement<int>(in);
        if (!isValid) {
          continue;
        }
        Mesh::SP mesh = std::make_shared<Mesh>();
        io::readVector(in,mesh->indices);
        io::readVector(in,mesh->vertices);
        io::readVector(in,mesh->normals);
        io::readVector(in,mesh->texcoords);
        int matID = io::readElement<int>(in);
        assert(matID >= 0);
        assert(matID < materials.size());
        mesh->material = materials[matID];
        object->meshes.push_back(mesh);
      }
      objects.push_back(object);
    }

    // ------------------------------------------------------------------
    // instances
    // ------------------------------------------------------------------
    size_t numInstances = io::readElement<size_t>(in);
    for (int instID=0;instID<numInstances;instID++) {
      int isValid = io::readElement<int>(in);
      if (!isValid) {
        scene->instances.push_back(0);
        continue;
      }
      Instance::SP inst = std::make_shared<Instance>();
      io::readElement(in,inst->xfm);
      inst->object = objects[io::readElement<int>(in)];
      scene->instances.push_back(inst);
    }

    // ------------------------------------------------------------------
    // wrap-up
    // ------------------------------------------------------------------

    size_t magicAtEnd = io::readElement<size_t>(in);
    if (magicAtEnd != oldFormat_magic)
      throw std::runtime_error("incomplete or incompatible mini file - cannot load");
      
    return scene;
  }

  Scene::SP Scene::load(const std::string &baseName)
  {
    std::ifstream in(baseName,std::ios::binary);
    if (!in.good())
      throw std::runtime_error("could not open Scene{"+baseName+"}");

    size_t magic = io::readElement<size_t>(in);

    if (magic == oldFormat_magic)
      return loadOldFormat(in);

    if (magic != expected_magic)
      throw std::runtime_error("invalid or incompatible 'mini' scene file (wrong file magic) - cannot load");
      
    Scene::SP scene = std::make_shared<Scene>();
    // ------------------------------------------------------------------
    // textures
    // ------------------------------------------------------------------
    std::vector<Texture::SP> textures;
    size_t numTextures = io::readElement<size_t>(in);
    for (int i=0;i<numTextures;i++) {
      int valid;
      io::readElement(in,valid);
      if (!valid) {
        textures.push_back({});
      } else {
        Texture::SP tex = std::make_shared<Texture>();
        io::readElement(in,tex->size);
        io::readElement(in,tex->format);
        io::readElement(in,tex->filterMode);
        io::readVector(in,tex->data);
        textures.push_back(tex);
      }
    }

    // ------------------------------------------------------------------
    // lights
    // ------------------------------------------------------------------
    io::readVector(in,scene->quadLights);
    io::readVector(in,scene->dirLights);
    const int hasEnvMap = io::readElement<int>(in);
    if (hasEnvMap) {
      scene->envMapLight = std::make_shared<EnvMapLight>();
      io::readElement(in,scene->envMapLight->transform);
      Texture::SP tex = scene->envMapLight->texture = std::make_shared<Texture>();
      io::readElement(in,tex->size);
      io::readElement(in,tex->format);
      io::readElement(in,tex->filterMode);
      io::readVector(in,tex->data);
    }
    
    // ------------------------------------------------------------------
    // materials
    // ------------------------------------------------------------------
    std::vector<Material::SP> materials;
    size_t numMaterials = io::readElement<size_t>(in);
    for (int i=0;i<numMaterials;i++) {
      Material::SP mat = std::make_shared<Material>();
      // io::readElement(in,(MaterialData&)*mat);
      io::readElement(in,mat->emission);
      io::readElement(in,mat->baseColor);
      io::readElement(in,mat->metallic);
      io::readElement(in,mat->roughness);
      io::readElement(in,mat->transmission);
      io::readElement(in,mat->ior);
      {
        int texID = io::readElement<int>(in);
        assert(texID >= 0);
        assert(texID < textures.size());
        mat->colorTexture = textures[texID];
      }
      {
        int texID = io::readElement<int>(in);
        assert(texID >= 0);
        assert(texID < textures.size());
        mat->alphaTexture = textures[texID];
      }
      materials.push_back(mat);
    }

    // ------------------------------------------------------------------
    // objects and meshes
    // ------------------------------------------------------------------
    size_t numObjects = io::readElement<size_t>(in);
    std::vector<Object::SP> objects;
    for (int objID=0;objID<numObjects;objID++) {
      size_t numMeshes = io::readElement<size_t>(in);
      Object::SP object = std::make_shared<Object>();

      for (int meshID=0;meshID<(int)numMeshes;meshID++) {
        int isValid = io::readElement<int>(in);
        if (!isValid) {
          continue;
        }
        Mesh::SP mesh = std::make_shared<Mesh>();
        io::readVector(in,mesh->indices);
        io::readVector(in,mesh->vertices);
        io::readVector(in,mesh->normals);
        io::readVector(in,mesh->texcoords);
        int matID = io::readElement<int>(in);
        assert(matID >= 0);
        assert(matID < materials.size());
        mesh->material = materials[matID];
        object->meshes.push_back(mesh);
      }
      objects.push_back(object);
    }

    // ------------------------------------------------------------------
    // instances
    // ------------------------------------------------------------------
    size_t numInstances = io::readElement<size_t>(in);
    for (int instID=0;instID<numInstances;instID++) {
      int isValid = io::readElement<int>(in);
      if (!isValid) {
        scene->instances.push_back(0);
        continue;
      }
      Instance::SP inst = std::make_shared<Instance>();
      io::readElement(in,inst->xfm);
      inst->object = objects[io::readElement<int>(in)];
      scene->instances.push_back(inst);
    }

    // ------------------------------------------------------------------
    // wrap-up
    // ------------------------------------------------------------------

    size_t magicAtEnd = io::readElement<size_t>(in);
    if (magicAtEnd != expected_magic)
      throw std::runtime_error("incomplete or incompatible .mini file - cannot load");
      
    return scene;
  }

} // ::brix

