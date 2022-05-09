// ======================================================================== //
// Copyright 2018 Ingo Wald                                                 //
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
#include "miniScene/common.h"
#include <fstream>
#include <sstream>
#include <set>
#include <deque>
#include <queue>
#include "Ptexture.h"
#include <unistd.h>

namespace mini {

  inline bool endsWith(const std::string &s, const std::string &suffix)
  {
    return s.substr(s.size()-suffix.size(),suffix.size()) == suffix;
  }
  
#define LOCK(mtx) std::lock_guard<std::mutex> _lock(mtx)

  /*! references a shape _inside_ an instance. Eg, a instance of an
    object with three shapes will create three IGs */
  struct InstGeom {
    int instID;
    int geomID;
  };

  /*! the final "things" we are computing: a subset of a scene,
    consisting of all the IG IDs supposed to be in this part, plus a
    set of "bricks" that bound all the space that this part is
    responsible for */
  struct ScenePart {
    std::vector<box3f> bricks;
    std::set<int>      igIDs;
  };
  

  inline float linear_to_srgb(float x) {
    if (x <= 0.0031308f) {
      return 12.92f * x;
    }
    return 1.055f * pow(x, 1.f/2.4f) - 0.055f;
  }

  inline uint32_t make_8bit(const float f)
  {
    return min(255,max(0,int(f*256.f)));
  }

  inline uint32_t make_rgba(const vec3f color)
  {
    return
      (make_8bit(color.x) << 0) +
      (make_8bit(color.y) << 8) +
      (make_8bit(color.z) << 16) +
      (0xffU << 24);
  }
  inline uint32_t make_rgba(const vec4f color)
  {
    return
      (make_8bit(color.x) << 0) +
      (make_8bit(color.y) << 8) +
      (make_8bit(color.z) << 16) +
      (make_8bit(color.w) << 24);
  }

  inline unsigned short to5(const float channel)
  {
    int maxVal = (1<<5)-1;
    return max(0,min(maxVal,int(maxVal*channel+.5f)));
  }
  
  inline unsigned short to555(const vec3f color)
  {
    return
      (to5(color.z) << 10)+
      (to5(color.y) <<  5)+
      (to5(color.x) <<  0);
  }

  Texture::SP doBakeTexture(Texture::SP in_ptex,
                          int RES)
  {
    assert(in_ptex->format == Texture::EMBEDDED_PTEX);
    assert(!in_ptex->data.empty());

    // ==================================================================
    // step 1: turn it back into a single ptex file; then open using
    // ptex library
    // ==================================================================
    char tempFileName[1000] = "";
    tmpnam(tempFileName);
    std::ofstream file(tempFileName,std::ios::binary);
    file.write((char*)in_ptex->data.data(),in_ptex->data.size());
    file.close();

    std::string error = "";
    Ptex::PtexTexture *ptex = Ptex::PtexTexture::open(tempFileName,error);
    if (error != "")
      throw std::runtime_error("ptex error : "+error);
    
    Ptex::PtexFilter::Options defaultOptions;
    Ptex::PtexFilter *filter = Ptex::PtexFilter::getFilter(ptex,defaultOptions);
    
    // ==================================================================
    // step 2: create vector with RESxRES texels for each prim
    // ==================================================================
    std::cout << "baking out ptex " << (int*)in_ptex.get() << " with " << ptex->numFaces() << " faces..." << std::endl;
    std::vector<uint32_t> texels;
    for (int faceID=0;faceID<ptex->numFaces();faceID++) {
      for (int iy=0;iy<RES;iy++)
        for (int ix=0;ix<RES;ix++) {
          vec2f uv = vec2f(ix,iy)/float(RES-1);
          vec3f texel;
          filter->eval(&texel.x,0,3,faceID,uv.x,uv.y,0,0,0,0);
          texels.push_back(make_rgba(texel));
        }
    }
    unlink(tempFileName);
    filter->release();
    // ptex->release();
    std::cout << OWL_TERMINAL_GREEN
              << "successfully baked out ptex with " << ptex->numFaces() << " faces..."
              << OWL_TERMINAL_DEFAULT << std::endl;
    
    // ==================================================================
    // step 3: turn the linear list of RESxRES mini-textures into one
    // single texture atlas
    // ==================================================================
    const int bakeRes = RES;
    const int numTexels = texels.size();

    const int quadsPerRowOfPtexAtlas = 256;
    const vec2i numQuads = {
                            quadsPerRowOfPtexAtlas,
                            divRoundUp(ptex->numFaces(),quadsPerRowOfPtexAtlas)
    };
    
    Texture::SP texture = std::make_shared<Texture>();
    texture->format = Texture::RGBA_UINT8;
    texture->size   = { numQuads.x*bakeRes, numQuads.y*bakeRes };
    texture->data.resize(texture->size.x*texture->size.y*sizeof(uint32_t));
    
    uint32_t *in  = (uint32_t*)texels.data();
    uint32_t *out = (uint32_t*)texture->data.data();
    for (int i=0;i<numTexels;i++)
      out[i] = 0;
    for (int y=0;y<texture->size.y;y++) {
      for (int x=0;x<texture->size.x;x++) {
        vec2i quad    = { x / bakeRes, y / bakeRes };
        vec2i sub     = { x % bakeRes, y % bakeRes };
        int   quadID  = quad.x + numQuads.x*quad.y;
        int   texelID = quadID * bakeRes*bakeRes;
        if (texelID >= texels.size())
          out[y*texture->size.x+x] = 0;
        else
          out[y*texture->size.x+x] =
            in[quadID*bakeRes*bakeRes + sub.y*bakeRes + sub.x];
      }
    }
    return texture;
  }

  Texture::SP bakeTexture(Texture::SP in,
                          int RES)
  {
    static std::map<Texture::SP,Texture::SP> alreadyBaked;
    if (alreadyBaked.find(in) == alreadyBaked.end())
      alreadyBaked[in] = doBakeTexture(in,RES);
    return alreadyBaked[in];
  }

  /*! do baking of mesh (if applicable), do _not_ check if it's already been done */
  Mesh::SP doBakeMesh(Mesh::SP in, int ptexBakingWidth)
  {
    if (!in)
      return in;
    if (!in->material)
      return in;
    if (!in->material->colorTexture)
      return in;
    if (in->material->colorTexture->format != Texture::EMBEDDED_PTEX)
      return in;

    Mesh::SP out = std::make_shared<Mesh>();//doBakeMesh(in,ptexBakingWidth);
    out->material = in->material->clone();
    out->material->colorTexture = bakeTexture(in->material->colorTexture,ptexBakingWidth);
    if (!out->material->colorTexture)
      throw std::runtime_error("some error in baking....");

    std::cout << " -> re-meshing (for new texcoords) mesh with " << in->indices.size() << " triangles" << std::endl;
    for (int quadID=0;quadID<in->indices.size()/2;quadID++) {
      vec4i idx = {
                   in->indices[2*quadID+0].x,
                   in->indices[2*quadID+0].y,
                   in->indices[2*quadID+0].z,
                   in->indices[2*quadID+1].z
        };
      // PING;
      // PRINT(in->indices[2*quadID+0]);
      // PRINT(in->indices[2*quadID+1]);
      out->vertices.push_back(in->vertices[idx.x]);
      out->vertices.push_back(in->vertices[idx.y]);
      out->vertices.push_back(in->vertices[idx.z]);
      out->vertices.push_back(in->vertices[idx.w]);
      if (!in->normals.empty()) {
        out->normals.push_back(in->normals[idx.x]);
        out->normals.push_back(in->normals[idx.y]);
        out->normals.push_back(in->normals[idx.z]);
        out->normals.push_back(in->normals[idx.w]);
      }
      out->indices.push_back(4*quadID+vec3i(0,1,2));
      out->indices.push_back(4*quadID+vec3i(0,2,3));

      assert(out->material);
      assert(out->material->colorTexture);
      const vec2i texSize = out->material->colorTexture->size;
      
      const int quadsPerRowOfPtexAtlas
        = texSize.x
        / ptexBakingWidth;
      vec2i quad = {
                    quadID % quadsPerRowOfPtexAtlas,
                    quadID / quadsPerRowOfPtexAtlas
      };
      float u0 = (quad.x*ptexBakingWidth+.5f) / float(texSize.x);
      float u1 = (quad.x*ptexBakingWidth+ptexBakingWidth-.5f) / float(texSize.x);
      float v0 = (quad.y*ptexBakingWidth+.5f) / float(texSize.y);
      float v1 = (quad.y*ptexBakingWidth+ptexBakingWidth-.5f) / float(texSize.y);
      out->texcoords.push_back({u0,v0});
      out->texcoords.push_back({u1,v0});
      out->texcoords.push_back({u1,v1});
      out->texcoords.push_back({u0,v1});
    }
    
    return out;
  }
  
  /*! do baking of mesh, but only if not already done */
  Mesh::SP bakeMesh(Mesh::SP in, int ptexBakingWidth)
  {
    static std::map<Mesh::SP,Mesh::SP> bakedVersionOf;
    if (bakedVersionOf.find(in) == bakedVersionOf.end())
      bakedVersionOf[in] = doBakeMesh(in,ptexBakingWidth);
    return bakedVersionOf[in];
  }
    
  void usage(const std::string &msg)
  {
    if (!msg.empty()) std::cerr << std::endl << "***Error***: " << msg << std::endl << std::endl;
    std::cout << "Usage: ./brixBakePtex inFile.pbf <args>" << std::endl;
    std::cout << "Args:" << std::endl;
    std::cout << " --res <resolution> : resolution to bake out with" << std::endl;
    // std::cout << " -t texturePath   : base path for input" << std::endl;
    std::cout << " -o outFileName   : output file name (required)" << std::endl;
    exit(msg != "");
  }

  extern "C" int main(int ac, char **av)
  {
    std::string inFileName = "";
    std::string outFileName = "";
    int res = 8;
      
    for (int i=1;i<ac;i++) {
      const std::string arg = av[i];
      if (arg == "-o") {
        outFileName = av[++i];
      } else if (arg == "--res" || arg == "-r") {
        res = atoi(av[++i]);
      // } else if (arg == "-t") {
      //   texturePath = av[++i];
      } else if (arg[0] != '-')
        inFileName = arg;
      else
        usage("unknown cmd line arg '"+arg+"'");
    }
    
    if (inFileName == "") usage("no input file name specified");
    if (outFileName == "") usage("no output file name specified");
    
    std::cout << "==================================================================" << std::endl;
    std::cout << "loading scene..." << std::endl;
    std::cout << "==================================================================" << std::endl;
    
  /*! gloabl scene we're splitting */
    Scene::SP scene  = Scene::load(inFileName);

    std::set<Object::SP> objects;
    for (auto inst : scene->instances)
      if (inst->object)
        objects.insert(inst->object);
    for (auto obj : objects) {
      for (auto &mesh : obj->meshes)
        mesh = bakeMesh(mesh,res);
    }

    std::cout << "saving to " << outFileName << std::endl;
    scene->save(outFileName);
    
    std::cout << OWL_TERMINAL_GREEN << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << "done" << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << OWL_TERMINAL_DEFAULT << std::endl;
    
  }

} // ::mini
