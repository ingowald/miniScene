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

  std::string myPretty(size_t n)
  {
    if (n < 1024) return "  "+prettyNumber(n);

    return "  "+prettyNumber(n)+"\t("+std::to_string(n)+")";
  }
  
  void printInfo(Scene::SP scene)
  {
    SerializedScene serialized(scene.get());
    std::cout << "----" << std::endl;
    std::cout << "num instances\t\t: " << myPretty(scene->instances.size()) << std::endl;
    std::cout << "num objects\t\t: " << myPretty(serialized.objects.size()) << std::endl;

    size_t numUniqueMeshes = 0;
    size_t numUniqueTriangles = 0;
    size_t numUniqueVertices = 0;
    
    for (auto mesh : serialized.meshes.list) {
      numUniqueMeshes++;
      numUniqueTriangles += mesh->indices.size();
      numUniqueVertices  += mesh->vertices.size();
    }
    std::cout << "----" << std::endl;
    std::cout << "num *unique* meshes\t: "    << myPretty(numUniqueMeshes) << std::endl;
    std::cout << "num *unique* triangles\t: " << myPretty(numUniqueTriangles) << std::endl;
    std::cout << "num *unique* vertices\t: "  << myPretty(numUniqueVertices) << std::endl;

    size_t numActualMeshes = 0;
    size_t numActualTriangles = 0;
    size_t numActualVertices = 0;
    size_t numActualNormals = 0;
    size_t numActualTexcoords = 0;
    
    for (auto inst : scene->instances)
      if (inst && inst->object)
        for (auto mesh : inst->object->meshes) {
          numActualMeshes++;
          numActualTriangles += mesh->indices.size();
          numActualVertices  += mesh->vertices.size();
          numActualNormals  += mesh->normals.size();
          numActualTexcoords  += mesh->texcoords.size();
        }
    
    std::cout << "----" << std::endl;
    std::cout << "num *actual* meshes\t: "    << myPretty(numActualMeshes) << std::endl;
    std::cout << "num *actual* triangles\t: " << myPretty(numActualTriangles) << std::endl;
    std::cout << "num *actual* vertices\t: "  << myPretty(numActualVertices) << std::endl;
    std::cout << "num *actual* normals\t: "  << myPretty(numActualNormals) << std::endl;
    std::cout << "num *actual* texcoords\t: "  << myPretty(numActualTexcoords) << std::endl;
    


    std::cout << "----" << std::endl;
    size_t numTextures = 0;
    size_t numPtex = 0;
    size_t bytesPtex = 0;
    size_t bytesTexels = 0;
    for (auto tex : serialized.textures.list)
      if (tex) {
        numTextures++;
        switch (tex->format) {
        case mini::Texture::EMBEDDED_PTEX: 
          numPtex++;
          bytesPtex += tex->data.size();
          break;
        default:
          bytesTexels += tex->data.size();
          break;
        };
      }
    std::cout << "num textures\t\t: " << myPretty(numTextures) << std::endl;
    std::cout << " - num *ptex* textures\t: " << myPretty(numPtex) << std::endl;
    std::cout << " - num *image* textures\t: " << myPretty(serialized.textures.size()-numPtex) << std::endl;
    std::cout << "total size of textures\t: " << myPretty(bytesPtex+bytesTexels) << std::endl;
    std::cout << " - #bytes in ptex\t: " << myPretty(bytesPtex) << std::endl;
    std::cout << " - #byte in texels\t: " << myPretty(bytesTexels) << std::endl;
    std::cout << "num materials\t\t: " << myPretty(serialized.materials.size()) << std::endl;
    std::cout << "num quad lights\t\t: " << myPretty(scene->quadLights.size()) << std::endl;
    std::cout << "num dir lights\t\t: " << myPretty(scene->dirLights.size()) << std::endl;
    if (scene->envMapLight) {
      std::cout << "has env-map light?\t: yes"  << std::endl;
      if (scene->envMapLight->texture)
        std::cout << "has env-map light texture?\t: yes, with " 
                  << scene->envMapLight->texture->size.x
                  << "x"
                  << scene->envMapLight->texture->size.y
                  << " texels" << std::endl;
    }
    else
      std::cout << "has env-map light?\t: no"  << std::endl;
    
    std::cout << "bounding box\t: " << scene->getBounds() << std::endl;
  }
    
  void miniInfo(int ac, char **av)
  {
    std::string inFileName = "";
    for (int i=1;i<ac;i++) {
      std::string arg = av[i];
      if (arg[0] != '-')
        inFileName = arg;
      else
        throw std::runtime_error("unknown cmdline argument '"+arg+"'");
    }
    if (inFileName.empty())
      throw std::runtime_error("no input file specified");

    std::cout << MINI_TERMINAL_LIGHT_BLUE
              << "loading mini file from " << inFileName 
              << MINI_TERMINAL_DEFAULT << std::endl;
    Scene::SP scene = Scene::load(inFileName);
    std::cout << MINI_TERMINAL_LIGHT_GREEN
              << "#miniInfo: scene loaded."
              << MINI_TERMINAL_DEFAULT << std::endl;

    printInfo(scene);
  }
  
} // ::mini

int main(int ac, char **av)
{ 
    try {
        mini::miniInfo(ac, av);
    }
    catch (std::exception& e) {
            std::cerr << "*** Fatal error *** " << std::endl << e.what() << std::endl;
            exit(1);
    }
    return 0; 
}

