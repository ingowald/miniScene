// SPDX-FileCopyrightText: Copyright (c) 2023++ Ingo Wald
// SPDX-License-Identifier: Apache-2.0

// Copyright 2024-2025 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

// #include "tsd/core/ColorMapUtil.hpp"
// #include "tsd/core/Logging.hpp"
// #include "tsd/core/TSDMath.hpp"
// #include "tsd/core/scene/objects/Array.hpp"
// #include "tsd/io/importers.hpp"
// #include "tsd/io/importers/detail/HDRImage.h"
// #include "tsd/io/importers/detail/importer_common.hpp"
// usd

#include "miniScene/Scene.h"
#include <type_traits>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdLux/diskLight.h>
#include <pxr/usd/usdLux/distantLight.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/usd/usdLux/rectLight.h>
#include <pxr/usd/usdLux/sphereLight.h>
#include <pxr/usd/usdShade/input.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/output.h>
#include <pxr/usd/usdShade/shader.h>
#include <pxr/usd/usdVol/volume.h>
// pxr
#include <pxr/usd/usdShade/output.h>
#include <pxr/usd/usdShade/connectableAPI.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/assetPath.h>
// std
#include <limits>
#include <string>
#include <vector>
#include <stack>

// #include "stb/stb_image.h"
#ifndef _WIN32
#include "tinyexr.h"
#endif

#define NO_INSTANCES 1


namespace hs {
  namespace loader{

    bool verbose = false;
    mini::Scene::SP protoScene = {};
    
    struct HDRImage
    {
      bool import(std::string fileName);

      unsigned width;
      unsigned height;
      unsigned numComponents;
      std::vector<float> pixel;
    };

    bool HDRImage::import(std::string fileName)
    {
#if 0
      if (fileName.size() < 4)
        return false;

      // check the extension
      std::string extension = std::string(strrchr(fileName.c_str(), '.'));
      std::transform(extension.data(),
                     extension.data() + extension.size(),
                     std::addressof(extension[0]),
                     [](unsigned char c) { return std::tolower(c); });

      if (extension != ".hdr" && extension != ".exr")
        return false;

      if (extension == ".hdr") {
        int w, h, n;
        stbi_set_flip_vertically_on_load(1);
        const float *imgData = stbi_loadf(fileName.c_str(), &w, &h, &n, STBI_rgb);
        stbi_set_flip_vertically_on_load(0); // Restore default top-down orientation
        width = w;
        height = h;
        numComponents = 3; // because of STBI_rgb
        if (width <= 0 || height <= 0 || n < 3) {
          stbi_image_free(const_cast<float*>(imgData));
          printf("import_HDRI] error importing HDR image: %s", fileName.c_str());
          return false;
        }

        pixel.resize(w * h * 3);
        std::memcpy(pixel.data(), imgData, w * h * 3 * sizeof(float));
        stbi_image_free(const_cast<float*>(imgData));
        return true;
      } else {
#ifdef _WIN32
#else
        int w, h;
        float *imgData;
        const char *err;
        int ret = LoadEXR(&imgData, &w, &h, fileName.c_str(), &err);
        if (ret != 0) {
          printf("import_HDRI] error importing EXR: %s", err);
          return false;
        }

        width = w;
        height = h;
        numComponents = 3;

        pixel.resize(w * h * 3);
        // LoadEXR returns a RGBA image, we want RGB.
        // Vertical flip the image.
        for (auto j = 0; j < h; ++j) {
          for (auto i = 0; i < w; ++i) {
            auto srcidx = 4 * (j * w + i);
            auto dstidx = 3 * ((h - j - 1) * w + i);
            pixel[dstidx + 0] = imgData[srcidx + 0];
            pixel[dstidx + 1] = imgData[srcidx + 1];
            pixel[dstidx + 2] = imgData[srcidx + 2];
          }
        }
        free(imgData);

        return true;
#endif
      }
#endif
      return false;
    }
  
    using namespace pxr;
    using namespace mini;
    
    std::string pathOf(const std::string &filepath);
  
    inline affine3f to_mini(const pxr::GfMatrix4d &m)
    {
      affine3f out;
      for (int i=0;i<3;i++) {
        out.l.vx[i] = m[0][i];
        out.l.vy[i] = m[1][i];
        out.l.vz[i] = m[2][i];
        out.p[i] = m[3][i];
      }
      return out;
    }

#ifdef _WIN32
    constexpr char path_sep = '\\';
#else
    constexpr char path_sep = '/';
#endif

    std::string pathOf(const std::string &filepath)
    {
      size_t pos = filepath.find_last_of(path_sep);
      if (pos == std::string::npos)
        return "";
      return filepath.substr(0, pos + 1);
    }

    std::string fileOf(const std::string &filepath)
    {
      size_t pos = filepath.find_last_of(path_sep);
      if (pos == std::string::npos)
        return "";
      return filepath.substr(pos + 1, filepath.size());
    }

    std::string extensionOf(const std::string &filepath)
    {
      size_t pos = filepath.rfind('.');
      if (pos == filepath.npos)
        return "";
      return filepath.substr(pos);
    }

    Texture::SP doLoadTexture(const std::string &fileName)
    {
#if 1
      std::cout << "SKIPPING textures" << std::endl;
      return {};
#else
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
        free(image);
        // STBI_FREE(image);
        std::cout << "#hs.USD: successfully loaded texture " << fileName << std::endl;
      } else {
        std::cout << "#hs.USD(!!!): could not load texture " << fileName << std::endl;
      }

      return texture;
#endif

      
    }
  
    struct USDScene {
      USDScene(const std::string &fileName)
        : fileName(fileName),
          basePath(pathOf(fileName)),
          miniScene(mini::Scene::create())
      {}

      mini::Scene::SP miniScene;
      const std::string fileName;
      const std::string basePath;
    
      std::map<std::string,mini::Material::SP> materials;
      std::map<std::string,mini::Texture::SP> textures;

      mini::Texture::SP getTexture(const std::string &name)
      {
        if (textures.find(name) != textures.end())
          return textures[name];

        std::string fileName = name;//basePath + name;
        if (!fileName.empty() && fileName[0] != '/')
          fileName = basePath+name;
        // std::string resolvedPath = metallicTexPath;
        // if (!resolvedPath.empty() && resolvedPath[0] != '/') {
        //   resolvedPath = basePath + metallicTexPath;
        // }
      
        Texture::SP tex = doLoadTexture(fileName);
        textures[name] = tex;
        return tex;
      }
    
      mini::Material::SP getMaterial(const std::string &name)
      {
        if (materials.find(name) == materials.end())
          return mini::DisneyMaterial::create();
        return materials[name];
      }
    
      // std::stack<affine3f> savedXFs;
      // affine3f currentXF;

      std::vector<mini::Mesh::SP> meshes;
      size_t trisInMeshes =0;
      
      void push(mini::Mesh::SP mesh) {
        /* meshes.push_back(mesh); */
        if (protoScene) {
          protoScene->instances[0]->object->meshes.push_back(mesh);
        } else {
          
          meshes.push_back(mesh);
          trisInMeshes += mesh->indices.size();
          PRINT(trisInMeshes);
        }
      }
    
      // void pushTransform(const affine3f &xf)
      // {
      //   savedXFs.push(currentXF);
      //   currentXF = xf;
      // }
    
      // void popTransform()
      // {
      //   currentXF = savedXFs.top();
      //   savedXFs.pop();
      // }
    };

    // Helper to get the bound material for a prim (USD or default)
    mini::Material::SP get_bound_material(USDScene &scene,
                                          const pxr::UsdPrim &prim)
    {
      // MaterialRef mat = scene.defaultMaterial();
      mini::Material::SP mat = mini::DisneyMaterial::create();
      pxr::UsdShadeMaterialBindingAPI binding(prim);
      pxr::UsdShadeMaterial usdMat = binding.ComputeBoundMaterial();
      std::string materialPath = usdMat.GetPath().GetString();
    
      return scene.getMaterial(materialPath);
    }
  
    // Helper to check if a GfMatrix4d is identity
    bool is_identity(const pxr::GfMatrix4d &m)
    {
      static const pxr::GfMatrix4d IDENTITY(1.0);
      return m == IDENTITY;
    }

    void import_usd_dome_light(USDScene &scene,
                               const pxr::UsdPrim &prim,
                               const pxr::GfMatrix4d &usdXform)
    {
      pxr::UsdLuxDomeLight usdLight(prim);
      // auto light = scene.createObject<Light>(tokens::light::hdri);
      // light->setName(prim.GetName().GetText());
      float intensity = 1.0f;
      usdLight.GetIntensityAttr().Get(&intensity);
      pxr::GfVec3f color(1.0f);
      usdLight.GetColorAttr().Get(&color);

      // (iw) TODO:
      // light->setParameter("color", float3(color[0], color[1], color[2]));
      // light->setParameter("scale", intensity);
    
      // Extract direction and up vectors from transformation matrix
      // ANARI defaults: direction=(1,0,0), up=(0,0,1)
      // USD dome lights use Z-up by default, matching ANARI
      auto xfm = pxr::GfMatrix4d(// clang-format off
                                 0.0, 1.0, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0,
                                 1.0, 0.0, 0.0, 0.0,
                                 0.0, 0.0, 0.0, 1.0
                                 // clang-format on
                                );
      xfm *= usdXform;
      pxr::GfVec3d dirVec = xfm.TransformDir(pxr::GfVec3d(0, 0, -1));
      pxr::GfVec3d upVec = xfm.TransformDir(pxr::GfVec3d(0, 1, 0));

      vec3f direction(dirVec[0], dirVec[1], dirVec[2]);
      vec3f up(upVec[0], upVec[1], upVec[2]);

      mini::EnvMapLight::SP light = mini::EnvMapLight::create();
      // light->setParameter("direction", direction);
      // light->setParameter("up", up);
      light->transform.l.vx = direction;
      light->transform.l.vy = cross(direction,up);
      light->transform.l.vz = up;
      // Load and set environment texture from usdLight.GetTextureFileAttr()
      pxr::SdfAssetPath textureAsset;
      std::string texFileName;
      if (!usdLight.GetTextureFileAttr().Get(&textureAsset)) {
        std::cout << "no texture asset on hdri light" << std::endl;
        return;
      }
      texFileName = textureAsset.GetResolvedPath();
      if (texFileName.empty()) {
        std::cout << "#hs.usd: hdr light texture asset has no path... reverting to getassetpath" << std::endl;
        texFileName = textureAsset.GetAssetPath();
      }
    
      if (texFileName.empty()) {
        std::cout << "could not find any hdri texture on hdri light" << std::endl;
        return;
      }
    
      HDRImage img;
      std::cout << "#hs.usd: hdri light " << texFileName << std::endl;
      if (!img.import(texFileName)) {
        std::cout << "had hdri texture path ('" << texFileName
                  << "') but could not load" << std::endl;
        return;
      }
      std::vector<vec3f> rgb(img.width * img.height);
      if (img.numComponents == 3) {
        memcpy(rgb.data(), img.pixel.data(), sizeof(rgb[0]) * rgb.size());
      } else if (img.numComponents == 4) {
        for (size_t i = 0; i < img.pixel.size(); i += 4) {
          rgb[i / 4] =
            vec3f(img.pixel[i], img.pixel[i + 1], img.pixel[i + 2]);
        }
      }
    
      // Handle color temperature if present
      float colorTemp = 0.0f;
      if (usdLight.GetColorTemperatureAttr().Get(&colorTemp)
          && colorTemp > 0.0f) {
        // Convert color temperature to RGB multiplier
        // Using approximation from Planckian locus
        auto kelvinToRGB = [](float kelvin) -> vec3f {
          // https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html
          float temp = kelvin / 100.0f;
          float red, green, blue;
        
          // Calculate red
          if (temp <= 66.0f) {
            red = 1.0f;
          } else {
            red = temp - 60.0f;
            red = 329.698727446f * std::pow(red, -0.1332047592f);
            red = std::clamp(red / 255.0f, 0.0f, 1.0f);
          }
        
          // Calculate green
          if (temp <= 66.0f) {
            green = temp;
            green = 99.4708025861f * std::log(green) - 161.1195681661f;
            green = std::clamp(green / 255.0f, 0.0f, 1.0f);
          } else {
            green = temp - 60.0f;
            green = 288.1221695283f * std::pow(green, -0.0755148492f);
            green = std::clamp(green / 255.0f, 0.0f, 1.0f);
          }
        
          // Calculate blue
          if (temp >= 66.0f) {
            blue = 1.0f;
          } else if (temp <= 19.0f) {
            blue = 0.0f;
          } else {
            blue = temp - 10.0f;
            blue = 138.5177312231f * std::log(blue) - 305.0447927307f;
            blue = std::clamp(blue / 255.0f, 0.0f, 1.0f);
          }
        
          return vec3f(red, green, blue);
        };
      
        vec3f tempColor = kelvinToRGB(colorTemp);
        for (auto &color : rgb) {
          color *= vec3f(tempColor.x, tempColor.y, tempColor.z);
        }
        if (verbose)
          printf("[import_USD] Applied dome light color temperature: %f K (%f %f %f)\n",
               colorTemp,
               tempColor.x,
               tempColor.y,
               tempColor.z);
      }
    
      // Apply exposure adjustment if present
      float exposure = 0.0f;
      if (usdLight.GetExposureAttr().Get(&exposure)) {
        // Convert exposure to linear scale: multiplier = 2^exposure
        float exposureScale = std::pow(2.0f, exposure);
        for (auto &color : rgb) {
          color *= exposureScale;
        }
        printf("[import_USD] Applied dome light exposure: %f (scale: %f)\n",
               exposure,
               exposureScale);
      }
    
      mini::Texture::SP tex = mini::Texture::create();
      tex->format = mini::Texture::FLOAT4;
      tex->size = { (int)img.width, (int)img.height };
      tex->data.resize(img.width*img.height*sizeof(vec4f));
      float *in = (float*)rgb.data();
      float *out = (float*)tex->data.data();
      for (int i=0;i<img.width*img.height;i++) {
        *out++ = *in++;
        *out++ = *in++;
        *out++ = *in++;
        *out++ = 0.f;
      }
      // radiance =
      //     scene.createArray(ANARI_FLOAT32_VEC3, img.width, img.height);
      //   radiance->setData(rgb.data());
      // }
      light->texture = tex;
      scene.miniScene->envMapLight = light;
      std::cout << "#hs.usd: done loading HDR light" << std::endl;
    }
  
    void import_usd_volume(USDScene &scene,
                           const pxr::UsdPrim &prim,
                           const pxr::GfMatrix4d &usdXform)
    {
      std::cout << __PRETTY_FUNCTION__ << " not implemented" << std::endl;
    }
  

    void import_usd_disk_light(USDScene &scene, const pxr::UsdPrim &prim)
    {
      std::cout << __PRETTY_FUNCTION__ << " not implemented" << std::endl;
    }

    void import_usd_sphere_light(USDScene &scene, const pxr::UsdPrim &prim)
    {
      std::cout << __PRETTY_FUNCTION__ << " not implemented" << std::endl;
    }


    void import_usd_rect_light(USDScene &scene, const pxr::UsdPrim &prim)
    {
      std::cout << __PRETTY_FUNCTION__ << " not implemented" << std::endl;
    }

    void import_usd_distant_light(USDScene &scene, const pxr::UsdPrim &prim)
    {
      std::cout << __PRETTY_FUNCTION__ << " not implemented" << std::endl;
    }

    void import_usd_cone(USDScene &scene,
                         const pxr::UsdPrim &prim,
                         const pxr::GfMatrix4d &usdXform)
    {
      std::cout << __PRETTY_FUNCTION__ << " not implemented" << std::endl;
    }

    void import_usd_sphere(USDScene &scene,
                           const pxr::UsdPrim &prim,
                           const pxr::GfMatrix4d &usdXform)
    {
      std::cout << __PRETTY_FUNCTION__ << " not implemented" << std::endl;
    }

    void import_usd_points(USDScene &scene,
                           const pxr::UsdPrim &prim,
                           const pxr::GfMatrix4d &usdXform)
    {
      std::cout << __PRETTY_FUNCTION__ << " not implemented" << std::endl;
    }


    std::vector<uint32_t>
    generate_triangle_indices(const pxr::VtArray<int> &faceVertexIndices,
                              const pxr::VtArray<int> &faceVertexCounts)
    {
      std::vector<uint32_t> triangleIndices;
      size_t faceVertexOffset = 0;

      for (size_t face = 0; face < faceVertexCounts.size(); ++face) {
        int vertsInFace = faceVertexCounts[face];
      
        // Tessellate polygon as triangle fan: (0,1,2), (0,2,3), (0,3,4), ...
        for (int v = 2; v < vertsInFace; ++v) {
          triangleIndices.push_back(faceVertexIndices[faceVertexOffset + 0]);
          triangleIndices.push_back(faceVertexIndices[faceVertexOffset + v - 1]);
          triangleIndices.push_back(faceVertexIndices[faceVertexOffset + v]);
        }
      
        faceVertexOffset += vertsInFace;
      }
    
      return triangleIndices;
    }
  
    // Helper: Tessellate faceVarying data from polygons to triangles
    // FaceVarying data has one value per face-vertex (corner)
    // Returns tessellated data matching the triangle fan pattern
    template <typename T>
    std::vector<T>
    tessellate_facevarying_data(const pxr::VtArray<T> &faceVaryingData,
                                const pxr::VtArray<int> &faceVertexCounts)
    {
      std::vector<T> triangleData;
      size_t faceVertexOffset = 0;

      for (size_t face = 0; face < faceVertexCounts.size(); ++face) {
        int vertsInFace = faceVertexCounts[face];
      
        // Tessellate as triangle fan: (0,1,2), (0,2,3), (0,3,4), ...
        for (int v = 2; v < vertsInFace; ++v) {
          triangleData.push_back(faceVaryingData[faceVertexOffset + 0]);
          triangleData.push_back(faceVaryingData[faceVertexOffset + v - 1]);
          triangleData.push_back(faceVaryingData[faceVertexOffset + v]);
        }
      
        faceVertexOffset += vertsInFace;
      }
    
      return triangleData;
    }
  
    // Helper: Tessellate uniform (per-face) data to per-triangle
    // Uniform data has one value per face
    // Returns replicated data with one value per generated triangle
    template <typename T>
    std::vector<T>
    tessellate_uniform_data(const pxr::VtArray<T> &uniformData,
                            const pxr::VtArray<int> &faceVertexCounts)
    {
      std::vector<T> triangleData;

      for (size_t face = 0; face < faceVertexCounts.size(); ++face) {
        int vertsInFace = faceVertexCounts[face];
        int numTriangles = vertsInFace - 2;
      
        // Each triangle from this face gets the same uniform value
        for (int t = 0; t < numTriangles; ++t) {
          triangleData.push_back(uniformData[face]);
        }
      }
    
      return triangleData;
    }
  
    void import_usd_mesh(USDScene &scene,
                         const pxr::UsdPrim &prim,
                         const pxr::GfMatrix4d &usdXform)
    {
      affine3f miniXform = to_mini(usdXform);
          
#if 0
      static int primID = 0;
      if (primID++ % 20) {
        std::cout << " - SKIPPING " << prim.GetName().GetText() << std::endl;
        return;;
      }
        
      std::cout << "- DOING " << prim.GetName().GetText() << std::endl;
#endif     
          
      
      pxr::UsdGeomMesh mesh(prim);
    
      // Get vertex positions
      pxr::VtArray<pxr::GfVec3f> points;
      mesh.GetPointsAttr().Get(&points);
    
      // Get face topology
      pxr::VtArray<int> faceVertexIndices;
      mesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices);
      pxr::VtArray<int> faceVertexCounts;
      mesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts);
    
      // Get normals and their interpolation
      pxr::VtArray<pxr::GfVec3f> normals;
      pxr::TfToken normalsInterpolation = pxr::UsdGeomTokens->vertex; // Default
      mesh.GetNormalsAttr().Get(&normals);
      if (!normals.empty()) {
        normalsInterpolation = mesh.GetNormalsInterpolation();
      }

      // Get UVs and their interpolation
      pxr::VtArray<pxr::GfVec2f> uvs;
      pxr::TfToken uvsInterpolation = pxr::UsdGeomTokens->vertex; // Default
      // USD stores UVs as primvars, typically "st" or "UVMap"
      pxr::UsdGeomPrimvarsAPI primvarsAPI(mesh);
      pxr::UsdGeomPrimvar stPrimvar = primvarsAPI.GetPrimvar(pxr::TfToken("st"));
      if (!stPrimvar) {
        stPrimvar = primvarsAPI.GetPrimvar(pxr::TfToken("UVMap"));
      }
      if (stPrimvar) {
        // USD primvars can be indexed - need to use ComputeFlattened to expand
        // indices
        stPrimvar.ComputeFlattened(&uvs);
        if (!uvs.empty()) {
          uvsInterpolation = stPrimvar.GetInterpolation();
        }
      }

      std::string primName = prim.GetPath().GetString();
      if (primName.empty())
        primName = "<unnamed_mesh>";

        if (verbose)
      printf("[import_USD] Mesh '%s': %zu points, %zu faces, %zu normals (interpolation: %s), %zu UVs (interpolation: %s)\n",
             prim.GetName().GetString().c_str(),
             points.size(),
             faceVertexCounts.size(),
             normals.size(),
             normalsInterpolation.GetText(),
             uvs.size(),
             uvsInterpolation.GetText());

      // Convert vertex positions to float3
      std::vector<vec3f> positions;
      positions.reserve(points.size());
      for (auto p : points) {
        vec3f mp(p[0], p[1], p[2]);
        positions.push_back(xfmPoint(miniXform,mp));
        // pxr::GfVec3d xfm_p = xfm.TransformPoint(p);
        // pxr::GfVec3d dirVec = xfm.TransformDir(pxr::GfVec3d(0, 0, -1));
        // pxr::GfVec3d upVec = xfm.TransformDir(pxr::GfVec3d(0, 1, 0));
        // positions.push_back(vec3f(xfm_p[0], xfm_p[1], xfm_p[2]));
      }
    
      // Generate triangle indices from polygon faces
      std::vector<uint32_t> indices =
        generate_triangle_indices(faceVertexIndices, faceVertexCounts);

      if (verbose)
      printf("[import_USD] Mesh '%s': Generated %zu triangle indices (%zu triangles)\n",
             prim.GetName().GetString().c_str(),
             indices.size(),
             indices.size() / 3);
    
      mini::Mesh::SP meshObj = mini::Mesh::create();
    
      meshObj->vertices = positions;
      meshObj->indices = std::vector<vec3i>(indices.size()/3);
      std::copy(indices.begin(),indices.end(),(int*)meshObj->indices.data());

      // Handle normals based on USD interpolation
      if (!normals.empty()) {
        if (normalsInterpolation == pxr::UsdGeomTokens->vertex) {
          // Vertex interpolation: normals are per-vertex, shared by all triangles
          // No tessellation needed - just convert to float3
          std::vector<vec3f> normalData;
          normalData.reserve(normals.size());
          for (const auto &n : normals) {
            vec3f nn(n[0], n[1], n[2]);
            normalData.push_back(xfmNormal(miniXform,nn));
          }
          meshObj->normals = normalData;

      if (verbose)
          printf("[import_USD] Mesh '%s': Set %zu normals on vertex.normal\n",
                 prim.GetName().GetString().c_str(),
                 normalData.size());
      
        } else if (normalsInterpolation == pxr::UsdGeomTokens->faceVarying) {
          // FaceVarying interpolation: normals are per face-vertex (corner)
          // Need to tessellate from polygon corners to triangle corners
          auto tessellatedNormals =
            tessellate_facevarying_data(normals, faceVertexCounts);

          std::vector<vec3f> normalData;
          normalData.reserve(tessellatedNormals.size());
          for (const auto &n : tessellatedNormals) {
            normalData.push_back(vec3f(n[0], n[1], n[2]));
          }

          meshObj->normals = normalData;
      if (verbose)
          printf("[import_USD] Mesh '%s': Set %zu normals on faceVarying.normal\n",
                 prim.GetName().GetString().c_str(),
                 normalData.size());
        
        } else if (normalsInterpolation == pxr::UsdGeomTokens->uniform) {
          // Uniform interpolation: one normal per face
          // Need to replicate for each triangle generated from that face
          auto tessellatedNormals =
            tessellate_uniform_data(normals, faceVertexCounts);
        
          std::vector<vec3f> normalData;
          normalData.reserve(tessellatedNormals.size());
          for (const auto &n : tessellatedNormals) {
            normalData.push_back(vec3f(n[0], n[1], n[2]));
          }
        
          meshObj->normals = normalData;
      if (verbose)
          printf("[import_USD] Mesh '%s': Set %zu normals on primitive.normal\n",
                 prim.GetName().GetString().c_str(),
                 normalData.size());
        }
      }
    
      // Handle UVs based on USD interpolation
      if (!uvs.empty()) {
        if (uvsInterpolation == pxr::UsdGeomTokens->vertex) {
          // Vertex interpolation: UVs are per-vertex, shared by all triangles
          // No tessellation needed - just convert to float2
          for (auto v : uvs)
            meshObj->texcoords.push_back({v[0],v[1]});
        } else if (uvsInterpolation == pxr::UsdGeomTokens->faceVarying) {
          // FaceVarying interpolation: UVs are per face-vertex (corner)
          // Need to tessellate from polygon corners to triangle corners
          auto tessellatedUVs = tessellate_facevarying_data(uvs, faceVertexCounts);
          for (auto v : tessellatedUVs)
            meshObj->texcoords.push_back({v[0],v[1]});
        } else if (uvsInterpolation == pxr::UsdGeomTokens->uniform) {
          // Uniform interpolation: one UV per face
          // Need to replicate for each triangle generated from that face
          auto tessellatedUVs = tessellate_uniform_data(uvs, faceVertexCounts);
          for (auto v : tessellatedUVs)
            meshObj->texcoords.push_back({v[0],v[1]});
        }
      }

      // Material binding
      pxr::UsdShadeMaterialBindingAPI binding(prim);
      pxr::UsdShadeMaterial usdMat = binding.ComputeBoundMaterial();
      std::string materialPath = usdMat.GetPath().GetString();
    
      meshObj->material = scene.getMaterial(materialPath);
      scene.push(meshObj);
    }

    void import_usd_cylinder(USDScene &scene,
                             const pxr::UsdPrim &prim,
                             const pxr::GfMatrix4d &usdXform)
    {
      std::cout << __PRETTY_FUNCTION__ << " not implemented" << std::endl;
    }

    Scene::SP rootInstances = std::make_shared<Scene>();
    std::map<std::string,Scene::SP> protoScenes;
    
    static
    void import_usd_prim_recursive(USDScene &scene,
                                   const pxr::UsdPrim &prim,
                                   pxr::UsdGeomXformCache &xformCache,
                                   const pxr::GfMatrix4d &parentWorldXform = pxr::GfMatrix4d(1.0))
    {
      if (prim.IsInstance()) {
        pxr::UsdPrim prototype = prim.GetPrototype();
        if (prototype) {

          bool resetsXformStack = false;
          pxr::GfMatrix4d usdLocalXform =
            xformCache.GetLocalTransformation(prim, &resetsXformStack);
          pxr::GfMatrix4d thisWorldXform =
            resetsXformStack ? usdLocalXform : parentWorldXform * usdLocalXform;
          // tsd::math::mat4 tsdXform = to_tsd_mat4(usdLocalXform);
          affine3f miniXform = to_mini(usdLocalXform);



#if NO_INSTANCES
          auto xf = thisWorldXform;
          import_usd_prim_recursive(scene,
                                    prototype,
                       // xformNode,
                                    xformCache,
                                    xf);
#else
          

#if 1
          std::string protoName = prototype.GetName().GetString();
          if (protoScene)
            std::cout << "PROTO SCENE ALREADY EXISTS!!!!" << std::endl;
          Scene::SP found = protoScenes[protoName];

          if (found) {
            std::cout << "FOUND instance for " << protoName << std::endl;
            PRINT(found->toString());
            PRINT(miniXform);

            Instance::SP inst = Instance::create(found->instances[0]->object,miniXform);
            scene.miniScene->instances.push_back(inst);
            return;
          }

          Object::SP protoObj = std::make_shared<Object>();
          Instance::SP instance = std::make_shared<Instance>(protoObj);
          protoScene = Scene::create({instance});
          protoScenes[protoName] = protoScene;

          Instance::SP inst = Instance::create(protoObj,miniXform);
          scene.miniScene->instances.push_back(inst);

          std::string name = ":";
          name = name + protoName;
          pxr::UsdPrim pp = prim;
          while (pp) {
            name = pp.GetName().GetString() + "::" + name;
            auto parent = pp.GetParent();
            if (pp == parent) { PING; break; }
            pp = parent;
          }
          PRINT(name);
#endif
          


          
          std::string primName = prim.GetName().GetString();
          if (primName.empty())
            primName = "<unnamed_instance>";
          // auto xformNode =
          //   scene.insertChildTransformNode(parent, tsdXform, primName.c_str());
          // Recursively import the prototype under this transform node
// #if 1
//           scene.pushTransform(affine3f());
// #else
//           scene.pushTransform(miniXform);
// #endif
          auto xf = thisWorldXform;
          import_usd_prim_recursive(scene,
                                    prototype,
                       // xformNode,
                                    xformCache,
                                    xf);
          // scene.popTransform();

          protoScene = {};
#endif
        } else {
          printf("[import_USD] Instance has no prototype: %s\n",
                 prim.GetName().GetString().c_str());
        }
        return;
      }

      // Only declare these in the main body (non-instance case)
      bool resetsXformStack = false;
      pxr::GfMatrix4d usdLocalXform =
        xformCache.GetLocalTransformation(prim, &resetsXformStack);
      pxr::GfMatrix4d thisWorldXform =
        resetsXformStack ? usdLocalXform : parentWorldXform * usdLocalXform;

      // Determine if this prim is a geometry or light
      bool isGeometry = prim.IsA<pxr::UsdGeomMesh>()
        || prim.IsA<pxr::UsdGeomPoints>() || prim.IsA<pxr::UsdGeomSphere>()
        || prim.IsA<pxr::UsdGeomCone>() || prim.IsA<pxr::UsdGeomCylinder>();
      bool isVolume = prim.IsA<pxr::UsdVolVolume>();
      bool isLight = prim.IsA<pxr::UsdLuxDistantLight>()
        || prim.IsA<pxr::UsdLuxRectLight>() || prim.IsA<pxr::UsdLuxSphereLight>()
        || prim.IsA<pxr::UsdLuxDiskLight>() || prim.IsA<pxr::UsdLuxDomeLight>();
      bool isDomeLight = prim.IsA<pxr::UsdLuxDomeLight>();
      bool isXform = prim.IsA<pxr::UsdGeomXform>() || prim.IsA<pxr::UsdGeomScope>();

      // Count children
      size_t numChildren = 0;
      for (const auto &child : prim.GetChildren())
        ++numChildren;

      // Only create a transform node if:
      // - The local transform is not identity
      // - The prim is geometry, light (not dome), or volume
      //   For the domelight, the rationale is the domelight can encode the
      //   transformation in
      //     its orientation axes and at least VisRTX and Barney do not correctly
      //     support transforming the HDRI lights.
      // - The prim resets the xform stack
      bool createNode = !is_identity(usdLocalXform) || isGeometry || isLight
        || isVolume || resetsXformStack;
      createNode = createNode && !isDomeLight;

      // tsd::math::mat4 tsdXform = to_tsd_mat4(usdLocalXform);
      affine3f miniXform = to_mini(usdLocalXform);;
      std::string primName = prim.GetName().GetString();
      if (primName.empty())
        primName = "<unnamed_xform>";

      // if (createNode) {
      //   thisNode =
      //     scene.insertChildTransformNode(parent, tsdXform, primName.c_str());
      // }

      // Import geometry for this prim (if any)
      if (prim.IsA<pxr::UsdGeomMesh>()) {
        import_usd_mesh(scene, prim, thisWorldXform);
      } else if (prim.IsA<pxr::UsdGeomPoints>()) {
        import_usd_points(scene, prim, thisWorldXform);
      } else if (prim.IsA<pxr::UsdGeomSphere>()) {
        import_usd_sphere(scene, prim, thisWorldXform);
      } else if (prim.IsA<pxr::UsdGeomCone>()) {
        import_usd_cone(scene, prim, thisWorldXform);
      } else if (prim.IsA<pxr::UsdGeomCylinder>()) {
        import_usd_cylinder(scene, prim, thisWorldXform);
      } else if (prim.IsA<pxr::UsdLuxDistantLight>()) {
        import_usd_distant_light(scene, prim);
      } else if (prim.IsA<pxr::UsdLuxRectLight>()) {
        import_usd_rect_light(scene, prim);
      } else if (prim.IsA<pxr::UsdLuxSphereLight>()) {
        import_usd_sphere_light(scene, prim);
      } else if (prim.IsA<pxr::UsdLuxDiskLight>()) {
        import_usd_disk_light(scene, prim);
      } else if (prim.IsA<pxr::UsdLuxDomeLight>()) {
        import_usd_dome_light(scene, prim, thisWorldXform);
      } else if (prim.IsA<pxr::UsdVolVolume>()) {
        import_usd_volume(scene, prim, thisWorldXform);
      }
      // Recurse into children
      for (const auto &child : prim.GetChildren()) {
        import_usd_prim_recursive(scene,
                                  child,
                                  xformCache,
                                  thisWorldXform);
      }
    }




    bool getShaderFloatInput(const pxr::UsdShadeShader &shader,
                             const char *inputName,
                             float &outValue)
    {
      pxr::UsdShadeInput input = shader.GetInput(pxr::TfToken(inputName));
      if (!input) {
        return false;
      }

      // Check if there's a connected source
      if (input.HasConnectedSource()) {
        pxr::UsdShadeConnectableAPI source;
        pxr::TfToken sourceName;
        pxr::UsdShadeAttributeType sourceType;
        if (input.GetConnectedSource(&source, &sourceName, &sourceType)) {
          // Check if this is a connection to a material interface input
          pxr::UsdPrim sourcePrim = source.GetPrim();
          if (sourcePrim.IsA<pxr::UsdShadeMaterial>()) {
            // This is a material interface connection - get the value from the material's input
            pxr::UsdShadeMaterial mat(sourcePrim);
            pxr::UsdShadeInput matInput = mat.GetInput(sourceName);
            if (matInput && matInput.Get(&outValue)) {
              return true;
            }
          } else {
            // This is a connection to another shader's output
            pxr::UsdShadeShader sourceShader(sourcePrim);
            if (sourceShader) {
              pxr::UsdShadeOutput output = sourceShader.GetOutput(sourceName);
              if (output) {
                pxr::UsdAttribute attr = output.GetAttr();
                if (attr && attr.Get(&outValue)) {
                  return true;
                }
              }
            }
          }
        }
      }

      // Fall back to direct value
      if (input.Get(&outValue)) {
        return true;
      }
      return false;
    }


    bool getShaderBoolInput(const pxr::UsdShadeShader &shader,
                            const char *inputName,
                            bool &outValue)
    {
      pxr::UsdShadeInput input = shader.GetInput(pxr::TfToken(inputName));
      if (!input) {
        return false;
      }

      // Check if there's a connected source
      if (input.HasConnectedSource()) {
        pxr::UsdShadeConnectableAPI source;
        pxr::TfToken sourceName;
        pxr::UsdShadeAttributeType sourceType;
        if (input.GetConnectedSource(&source, &sourceName, &sourceType)) {
          // Check if this is a connection to a material interface input
          pxr::UsdPrim sourcePrim = source.GetPrim();
          if (sourcePrim.IsA<pxr::UsdShadeMaterial>()) {
            // This is a material interface connection - get the value from the material's input
            pxr::UsdShadeMaterial mat(sourcePrim);
            pxr::UsdShadeInput matInput = mat.GetInput(sourceName);
            if (matInput && matInput.Get(&outValue)) {
              return true;
            }
          } else {
            // This is a connection to another shader's output
            pxr::UsdShadeShader sourceShader(sourcePrim);
            if (sourceShader) {
              pxr::UsdShadeOutput output = sourceShader.GetOutput(sourceName);
              if (output) {
                pxr::UsdAttribute attr = output.GetAttr();
                if (attr && attr.Get(&outValue)) {
                  return true;
                }
              }
            }
          }
        }
      }

      // Fall back to direct value
      if (input.Get(&outValue)) {
        return true;
      }
      return false;
    }

    bool getShaderColorInput(const pxr::UsdShadeShader &shader,
                             const char *inputName,
                             pxr::GfVec3f &outValue)
    {
      pxr::UsdShadeInput input = shader.GetInput(pxr::TfToken(inputName));
      if (!input) {
        return false;
      }

      // Check if there's a connected source
      if (input.HasConnectedSource()) {
        pxr::UsdShadeConnectableAPI source;
        pxr::TfToken sourceName;
        pxr::UsdShadeAttributeType sourceType;
        if (input.GetConnectedSource(&source, &sourceName, &sourceType)) {
          // Check if this is a connection to a material interface input
          pxr::UsdPrim sourcePrim = source.GetPrim();
          if (sourcePrim.IsA<pxr::UsdShadeMaterial>()) {
            // This is a material interface connection - get the value from the material's input
            pxr::UsdShadeMaterial mat(sourcePrim);
            pxr::UsdShadeInput matInput = mat.GetInput(sourceName);
            if (matInput && matInput.Get(&outValue)) {
              return true;
            }
          } else {
            // This is a connection to another shader's output
            pxr::UsdShadeShader sourceShader(sourcePrim);
            if (sourceShader) {
              pxr::UsdShadeOutput output = sourceShader.GetOutput(sourceName);
              if (output) {
                pxr::UsdAttribute attr = output.GetAttr();
                if (attr && attr.Get(&outValue)) {
                  return true;
                }
              }
            }
          }
        }
      }

      // Fall back to direct value
      if (input.Get(&outValue)) {
        return true;
      }
      return false;
    }

    bool getShaderTextureInput(const pxr::UsdShadeShader &shader,
                               const char *inputName,
                               std::string &outFilePath)
    {
      pxr::UsdShadeInput input = shader.GetInput(pxr::TfToken(inputName));
      if (!input) {
        return false;
      }

      // Check if there's a connected texture reader
      if (input.HasConnectedSource()) {
        pxr::UsdShadeConnectableAPI source;
        pxr::TfToken sourceName;
        pxr::UsdShadeAttributeType sourceType;
        input.GetConnectedSource(&source, &sourceName, &sourceType);

        // Check if this is a connection to a material interface input
        pxr::UsdPrim sourcePrim = source.GetPrim();
        if (sourcePrim.IsA<pxr::UsdShadeMaterial>()) {
          // This is a material interface connection - get the value from the material's input
          pxr::UsdShadeMaterial mat(sourcePrim);
          pxr::UsdShadeInput matInput = mat.GetInput(sourceName);
          if (matInput) {
            pxr::SdfAssetPath assetPath;
            if (matInput.Get(&assetPath)) {
              outFilePath = assetPath.GetResolvedPath();
              if (outFilePath.empty()) {
                outFilePath = assetPath.GetAssetPath();
              }
              return !outFilePath.empty();
            }
          }
        } else {
          // Check if this is a texture reader shader
          pxr::UsdShadeShader textureShader(sourcePrim);
          if (textureShader) {
            // Look for file input on the texture reader
            pxr::UsdShadeInput fileInput = textureShader.GetInput(pxr::TfToken("file"));
            if (fileInput) {
              pxr::SdfAssetPath assetPath;
              if (fileInput.Get(&assetPath)) {
                outFilePath = assetPath.GetResolvedPath();
                if (outFilePath.empty()) {
                  outFilePath = assetPath.GetAssetPath();
                }
                return !outFilePath.empty();
              }
            }
          }
        }
      }

      // Try direct asset path input
      pxr::SdfAssetPath assetPath;
      if (input.Get(&assetPath)) {
        outFilePath = assetPath.GetResolvedPath();
        if (outFilePath.empty()) {
          outFilePath = assetPath.GetAssetPath();
        }
        return !outFilePath.empty();
      }

      return false;
    }
  
  
    mini::Material::SP
    createOmniPBRMaterial(USDScene &scene,
                          const pxr::UsdShadeMaterial &usdMaterial,
                          const pxr::UsdShadeShader &usdShader)
    {
      std::cout << "======================================================= " << std::endl;
      std::cout << "creating new USD material" << std::endl;                                                                                    
      mini::ANARIMaterial::SP mat = mini::ANARIMaterial::create();
      mat->baseColor = mini::common::randomColor(scene.materials.size());
    
      std::string diffuseTexPath;
      if (getShaderTextureInput(usdShader, "diffuse_texture", diffuseTexPath)) {
        mat->baseColor = vec3f(1.f);
        mat->baseColor_texture = scene.getTexture(diffuseTexPath);
      }

      // Handle emissive with intensity
      pxr::GfVec3f emissiveColor(0, 0, 0);
      float emissiveIntensity = 1.0f;
      bool enableEmission = false;

      getShaderColorInput(usdShader, "emissive_color", emissiveColor);
      getShaderFloatInput(usdShader, "emissive_intensity", emissiveIntensity);
      getShaderBoolInput(usdShader, "enable_emission", enableEmission);

      if (enableEmission) {
        // Scale emissive color by intensity
        vec3f finalEmissive(emissiveColor[0] * emissiveIntensity,
                            emissiveColor[1] * emissiveIntensity,
                            emissiveColor[2] * emissiveIntensity
                           );
        mat->emissive = finalEmissive;
      }

      // Metallic - try texture first, then constant
      std::string metallicTexPath;
      if (getShaderTextureInput(usdShader, "metallic_texture", metallicTexPath)) {
        mini::Texture::SP sampler = scene.getTexture(metallicTexPath);
        mat->metallic_texture = sampler;
      } else {
        float metallic = 0.0f;
        if (getShaderFloatInput(usdShader, "metallic_constant", metallic)) {
          mat->metallic = metallic;
        } else {
          mat->metallic = 0.f;
        }
      }

      // Roughness - try texture first, then constant
      std::string roughnessTexPath;
      if (getShaderTextureInput(usdShader, "reflectionroughness_texture",
                                roughnessTexPath)) {
        mini::Texture::SP sampler = scene.getTexture(roughnessTexPath);
        mat->roughness_texture = sampler;
      } else {
        float roughness = 0.5f;  // Default to mid-range roughness
        if (getShaderFloatInput(usdShader, "reflection_roughness_constant",
                                roughness)) {
          mat->roughness = roughness;
        } else {
          mat->roughness = 0.5f;
        }
      }

      // Normal map
      std::string normalTexPath;
      if (getShaderTextureInput(usdShader, "normalmap_texture", normalTexPath)) {
        mini::Texture::SP sampler = scene.getTexture(normalTexPath);
        mat->normal_texture = sampler;
      }

      // Ambient Occlusion map
      std::string aoTexPath;
      if (getShaderTextureInput(usdShader, "ao_texture", aoTexPath)) {
        mini::Texture::SP sampler = scene.getTexture(aoTexPath);
        mat->occlusion_texture = sampler;
      }

      // Opacity - try texture first, then constant
      std::string opacityTexPath;
      bool enableOpacity = false;
      getShaderBoolInput(usdShader, "enable_opacity", enableOpacity);

      // if (enableOpacity) {
      if (getShaderTextureInput(usdShader, "opacity_texture", opacityTexPath)) {
        mini::Texture::SP sampler = scene.getTexture(opacityTexPath);
        mat->opacity_texture = sampler;
      } else {
        float opacityConstant = 1.0f;
        if (getShaderFloatInput(usdShader, "opacity_constant", opacityConstant)) {
          mat->opacity = opacityConstant;
        }
      }

      // IOR (index of refraction)
      float ior = 1.5f;
      if (getShaderFloatInput(usdShader, "ior_constant", ior)) {
        mat->ior = ior;
      }

      // Specular level
      float specularLevel = 0.5f;
      if (getShaderFloatInput(usdShader, "specular_level", specularLevel)) {
        mat->specular = specularLevel;
      }
      return mat;
    }


    void createMaterials(USDScene &scene,
                         const std::string &fileName)
    {
      // auto filepath = fileName.c_str();
      // std::string basePath = pathOf(filepath);
      pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(scene.fileName.c_str());
      // pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(fileName.c_str());
      for (pxr::UsdPrim const &prim : stage->Traverse()) {
        if (!prim) continue;
        if (prim.IsA<pxr::UsdShadeMaterial>()) {
          std::string materialPath = prim.GetPath().GetString();
          printf("[import_USD2] Found material: %s\n", materialPath.c_str());
          std::cout << "MATERIAL *DEF* " << materialPath << std::endl;
          auto usdMaterial = pxr::UsdShadeMaterial(prim);
          auto surfaceOutputs = usdMaterial.GetSurfaceOutputs();
          auto surfaceOutput = usdMaterial.GetSurfaceOutput(pxr::TfToken("mdl"));

          // MaterialRef mat;

          if (!surfaceOutput) continue;
          
          for (auto &connectionSourceInfo : surfaceOutput.GetConnectedSources()) {
            UsdShadeShader shader(connectionSourceInfo.source);
            TfToken subIdentifier;
            shader.GetSourceAssetSubIdentifier(&subIdentifier, TfToken("mdl"));

            if (subIdentifier == TfToken("OmniPBR")) {
              printf("OmniPBR material...\n");
              scene.materials[materialPath]
                = createOmniPBRMaterial(scene, usdMaterial, shader);
              // mat = materials::importOmniPBRMaterial(
              //                                        scene, usdMaterial, shader, basePath, textureCache);
              break;
            } else {
              printf("Don't know how to process %s\n", subIdentifier.GetText());
            }
          }
        }
      }
    }



    mini::Scene::SP loadUSD(const std::string &fileName)
    {
      USDScene scene(fileName);
      pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(scene.fileName.c_str());
      createMaterials(scene,fileName);
      if (!stage) {
        printf("[import_USD] failed to open stage '%s'", fileName.c_str());
        return {};
      }
      printf("[import_USD] Opened USD stage: %s\n", fileName.c_str());
      auto defaultPrim = stage->GetDefaultPrim();
      if (defaultPrim) {
        printf("[import_USD] Default prim: %s\n",
               defaultPrim.GetPath().GetString().c_str());
      } else {
        printf("[import_USD] No default prim set.\n");
      }
      size_t primCount = 0;
      for (auto _ : stage->Traverse())
        ++primCount;
      printf("[import_USD] Number of prims in stage: %zu\n", primCount);

      pxr::UsdGeomXformCache xformCache(pxr::UsdTimeCode::Default());

      // Traverse all prims in the USD file, but only import top-level prims
      for (pxr::UsdPrim const &prim : stage->Traverse()) {
        // if (prim.IsPrototype()) continue;

        if (prim.GetParent() && prim.GetParent().IsPseudoRoot()) {
          import_usd_prim_recursive(scene,
                                    prim,
                        // usd_root,
                                    xformCache);
        }
      }

      mini::Object::SP miniObj = mini::Object::create(scene.meshes);
      // scene.miniScene->instances = {mini::Instance::create(miniObj)};
      scene.miniScene->instances.push_back(mini::Instance::create(miniObj));
      return scene.miniScene;
    }

  }
}

int main(int ac, char **av)
{
  assert(ac == 4);
  mini::Scene::SP scene = hs::loader::loadUSD(av[1]);
  scene->save(av[3]);
}
