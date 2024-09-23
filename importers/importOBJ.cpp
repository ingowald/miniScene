// ======================================================================== //
// Copyright 2018-2024 Ingo Wald                                            //
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
#include <cstring>
#include <set>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include "stb/stb_image.h"

namespace mini {

  struct index_less {
    inline bool operator()(const tinyobj::index_t &a,
                           const tinyobj::index_t &b) const
    {
      if (a.vertex_index < b.vertex_index) return true;
      if (a.vertex_index > b.vertex_index) return false;
    
      if (a.normal_index < b.normal_index) return true;
      if (a.normal_index > b.normal_index) return false;
    
      if (a.texcoord_index < b.texcoord_index) return true;
      if (a.texcoord_index > b.texcoord_index) return false;
    
      return false;
    }
  };

  /*! find vertex with given position, normal, texcoord, and return
    its vertex ID, or, if it doesn't exit, add it to the mesh, and
    its just-created index */
  int addVertex(Mesh::SP mesh,
                tinyobj::attrib_t &attributes,
                const tinyobj::index_t &idx,
                std::map<tinyobj::index_t,int,index_less> &knownVertices)
  {
    if (knownVertices.find(idx) != knownVertices.end())
      return knownVertices[idx];

    const vec3f *vertex_array   = (const vec3f*)attributes.vertices.data();
    const vec3f *normal_array   = (const vec3f*)attributes.normals.data();
    const vec2f *texcoord_array = (const vec2f*)attributes.texcoords.data();
    
    int newID = (int)mesh->vertices.size();
    knownVertices[idx] = newID;

    mesh->vertices.push_back(vertex_array[idx.vertex_index]);
    if (idx.normal_index >= 0) {
      while (mesh->normals.size() < mesh->vertices.size())
        mesh->normals.push_back(normal_array[idx.normal_index]);
    }
    if (idx.texcoord_index >= 0) {
      while (mesh->texcoords.size() < mesh->vertices.size())
        mesh->texcoords.push_back(texcoord_array[idx.texcoord_index]);
    }
    
    return newID;
  }



  Texture::SP doLoadTexture(const std::string &fileName)
  {
    Texture::SP texture;
    vec2i res;
    int   comp;
    unsigned char* image = stbi_load(fileName.c_str(),
                                     &res.x, &res.y, &comp, STBI_rgb_alpha);
    // int textureID = -1;
    if (image) {
      for (int y=0;y<res.y/2;y++) {
        uint32_t *line_y = ((uint32_t*)image) + y * res.x;
        uint32_t *mirrored_y = ((uint32_t*)image) + (res.y-1-y) * res.x;
        for (int x=0;x<res.x;x++) {
          std::swap(line_y[x],mirrored_y[x]);
        }
      }

      texture = std::make_shared<Texture>();
      texture->size = res;
      texture->data.resize(res.x*res.y*sizeof(int));
      texture->format = Texture::RGBA_UINT8;
      memcpy(texture->data.data(),image,texture->data.size());

      /* iw - actually, it seems that stbi loads the pictures
         mirrored along the y axis - mirror them here */
      
      STBI_FREE(image);
    }

    return texture;
  }

  /*! load a texture (if not already loaded), and return its ID in the
    model's textures[] vector. Textures that could not get loaded
    return -1 */
  Texture::SP loadTexture(std::map<std::string,Texture::SP> &knownTextures,
                          const std::string &inFileName,
                          const std::string &modelPath)
  {
    if (inFileName == "")
      return nullptr;
    
    if (knownTextures.find(inFileName) != knownTextures.end())
      return knownTextures[inFileName];

    std::string fileName = inFileName;
    // first, fix backspaces:
    for (auto &c : fileName)
      if (c == '\\') c = '/';

    Texture::SP texture = doLoadTexture(modelPath+"/"+fileName);

    // for our windows users, try the usual 'all upper' and 'all
    // lower' variants as well, in case users use one spelling in
    // material file while file system uses the other ...
    // if (!texture)
    //   texture = doLoadTexture(modelPath+"/"+to_upper(fileName));
    // if (!texture)
    //   texture = doLoadTexture(modelPath+"/"+to_lower(fileName));
    // if (!texture)
    //   // for casual effects sponza model ....
    //   texture = doLoadTexture(modelPath+"/"+to_lower_not_ext(fileName));

    if (!texture) {
      std::cout << MINI_TERMINAL_RED
                << "Could not load texture from " << modelPath+"/"+fileName << "!"
                << MINI_TERMINAL_DEFAULT << std::endl;
    }
      
    // Texture::SP texture;
    // vec2i res;
    // int   comp;
    // unsigned char* image = stbi_load(fileName.c_str(),
    //                                  &res.x, &res.y, &comp, STBI_rgb_alpha);
    // // int textureID = -1;
    // if (image) {
    //   for (int y=0;y<res.y/2;y++) {
    //     uint32_t *line_y = ((uint32_t*)image) + y * res.x;
    //     uint32_t *mirrored_y = ((uint32_t*)image) + (res.y-1-y) * res.x;
    //     for (int x=0;x<res.x;x++) {
    //       std::swap(line_y[x],mirrored_y[x]);
    //     }
    //   }

    //   texture = std::make_shared<Texture>();
    //   texture->size = res;
    //   texture->data.resize(res.x*res.y*sizeof(int));
    //   texture->format = Texture::RGBA_UINT8;
    //   memcpy(texture->data.data(),image,texture->data.size());

    //   /* iw - actually, it seems that stbi loads the pictures
    //      mirrored along the y axis - mirror them here */
      
    //   STBI_FREE(image);
    // } else {
    //   std::cout << MINI_TERMINAL_RED
    //             << "Could not load texture from " << fileName << "!"
    //             << MINI_TERMINAL_DEFAULT << std::endl;
    // }
    
    knownTextures[inFileName] = texture;
    return texture;
  }
  
  Scene::SP loadOBJ(const std::string &objFile)
  {
    Scene::SP scene = std::make_shared<Scene>();
    Object::SP model = std::make_shared<Object>();
    scene->instances.push_back(std::make_shared<Instance>(model));
    const std::string modelDir
      = objFile.substr(0,objFile.rfind('/')+1);
    
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err = "";

    std::cout << "reading OBJ file '" << objFile << " from directory '" << modelDir << "'" << std::endl;
    bool readOK
      = tinyobj::LoadObj(&attributes,
                         &shapes,
                         &materials,
                         &err,
                         &err,
                         objFile.c_str(),
                         modelDir.c_str(),
                         /* triangulate */true);
    if (!readOK) {
      throw std::runtime_error("Could not read OBJ model from "+objFile+" : "+err);
    }

    if (materials.empty())
      std::cout << MINI_TERMINAL_RED
                << "WARNING: NO MATERIALS (could not find/parse mtl file!?)"
                << MINI_TERMINAL_DEFAULT << std::endl;

    DisneyMaterial::SP dummyMaterial = std::make_shared<DisneyMaterial>();
    dummyMaterial->baseColor = randomColor(size_t(dummyMaterial.get()));

    std::vector<DisneyMaterial::SP> baseMaterials;
    tinyobj::material_t *objDefaultMaterial = 0;
    for (auto &objMat : materials) {
      DisneyMaterial::SP baseMaterial = std::make_shared<DisneyMaterial>();
      baseMaterial->baseColor =
        { float(objMat.diffuse[0]),
          float(objMat.diffuse[1]),
          float(objMat.diffuse[2]) };
      baseMaterial->emission =
        { float(objMat.emission[0]),
          float(objMat.emission[1]),
          float(objMat.emission[2]) };
      baseMaterial->ior = objMat.ior;
      baseMaterials.push_back(baseMaterial);

      if (objMat.name == "default")  {
        std::cout << "found material with name 'default' - will be using"
                  << " that for any material that cannot be otherwise resolved..."
                  << std::endl;
        objDefaultMaterial = &objMat;
        dummyMaterial = baseMaterial;
      }
    }

    std::map<std::pair<Material::SP,Texture::SP>,Material::SP>
      texturedMaterials;
      
    std::cout << "Done loading obj file - found "
              << shapes.size() << " shapes with "
              << materials.size() << " materials" << std::endl;
      
    std::map<std::string,Texture::SP> knownTextures;
    for (int shapeID=0;shapeID<(int)shapes.size();shapeID++) {
      tinyobj::shape_t &shape = shapes[shapeID];

      Mesh::SP mesh = std::make_shared<Mesh>();
        
      std::set<int> materialIDs;
      for (auto faceMatID : shape.mesh.material_ids)
        materialIDs.insert(faceMatID);
      
      
      for (int materialID : materialIDs) {
        std::map<tinyobj::index_t,int,index_less> knownVertices;
        Mesh::SP mesh = std::make_shared<Mesh>();
        // mesh->material
        //   = (materialID < ourMaterials.size())
        //   ? ourMaterials[materialID]
        //   : dummyMaterial;
          
        for (size_t faceID=0;faceID<shape.mesh.material_ids.size();faceID++) {
          if (shape.mesh.material_ids[faceID] != materialID) continue;
          if (shape.mesh.num_face_vertices[faceID] != 3)
            throw std::runtime_error("not properly tessellated");
          tinyobj::index_t idx0 = shape.mesh.indices[3*faceID+0];
          tinyobj::index_t idx1 = shape.mesh.indices[3*faceID+1];
          tinyobj::index_t idx2 = shape.mesh.indices[3*faceID+2];
          
          vec3i idx(addVertex(mesh, attributes, idx0, knownVertices),
                    addVertex(mesh, attributes, idx1, knownVertices),
                    addVertex(mesh, attributes, idx2, knownVertices));
          mesh->indices.push_back(idx);
        }
        Texture::SP diffuseTexture = {};
        DisneyMaterial::SP baseMaterial  = {};
        if (materialID >= 0 && materialID < materials.size()) {
          baseMaterial = baseMaterials[materialID];
          diffuseTexture = loadTexture(knownTextures,
                                       materials[materialID].diffuse_texname,
                                       modelDir);
        } else if (objDefaultMaterial) {
          diffuseTexture = loadTexture(knownTextures,
                                       objDefaultMaterial->diffuse_texname,
                                       modelDir);
          baseMaterial = dummyMaterial;
        } else {
          baseMaterial = dummyMaterial;
        }
        std::pair<Material::SP,Texture::SP> tuple = { baseMaterial,diffuseTexture };
        if (texturedMaterials.find(tuple) == texturedMaterials.end()) {
          DisneyMaterial::SP textured = std::make_shared<DisneyMaterial>();
          *textured = *baseMaterial;
          textured->colorTexture = diffuseTexture;
          mesh->material = textured;
          texturedMaterials[tuple] = textured;
        } else
          mesh->material = texturedMaterials[tuple];
          
        if (mesh->vertices.empty())
          /* ignore this mesh */;
        else {
          model->meshes.push_back(mesh);
        }
      }
    }

    return scene;
  }

} // ::mini


