// ======================================================================== //
// Copyright 2022-2023 Ingo Wald                                            //
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
//std
#include <set>
#include "happly.h"

namespace mini {
    
  Scene::SP loadPLY(const std::string &plyFile)
  {
    // number of triangles we drop due to invalid/malformed indices int he model
    size_t numDropped = 0;
    std::cout << "reading PLY file '" << plyFile << "'" << std::endl;
    happly::PLYData plyIn(plyFile.c_str());
    std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
    std::vector<std::vector<size_t>>   fInd = plyIn.getFaceIndices<size_t>();
    

    DisneyMaterial::SP dummyMaterial = std::make_shared<DisneyMaterial>();
    dummyMaterial->baseColor = vec3f(.7f);

    Mesh::SP mesh = std::make_shared<Mesh>();
    mesh->material = dummyMaterial;
    
    for (auto v : vPos)
      mesh->vertices.push_back({(float)v[0],(float)v[1],(float)v[2]});
    for (auto idx : fInd) {
      for (int i=2;i<3;i++) {
        const vec3i tri = {(int)idx[0],(int)idx[i-1],(int)idx[i]};
        bool dropThis = false;
        for (int d=0;d<3;d++) 
          if (tri[d] < 0 || tri[d] >= mesh->vertices.size()) 
            dropThis = true;
        
        if (dropThis) 
          numDropped++;
        else
          mesh->indices.push_back(tri);
      }
    }
    std::cout
      << "#mini.ply: imported " << prettyNumber(fInd.size()) << " ply faces;"
      << " created " << prettyNumber(mesh->indices.size()) << " triangles "
      << "(w/ " << prettyNumber(mesh->vertices.size()) << " vertices);"
      << " and dropped " << prettyNumber(numDropped) << " triangles due to out-of-bound indices"
      << std::endl;

    Object::SP model = std::make_shared<Object>();
    model->meshes.push_back(mesh);
    
    Scene::SP scene = std::make_shared<Scene>();
    scene->instances.push_back(std::make_shared<Instance>(model));
    return scene;
  }

  Scene::SP stitchStanford(const std::string &baseFileName, int numParts)
  {
    std::vector<Mesh::SP> meshes;
    for (int i=0;i<numParts;i++) {
      std::stringstream ss;
      ss << baseFileName << "_" << (i+1) << ".ply";
      Scene::SP part = loadPLY(ss.str());
      meshes.push_back(part->instances[0]->object->meshes[0]);
    }
    // the stanford models come with a "matches" file that specifies
    // which vertices in one mesh *should* be the same as those in the
    // previous one (but due to numerical issues, are not)
    std::cout << "read all parts ... doing overlaps" << std::endl;
    for (int i=1;i<numParts;i++) {
      std::stringstream ss;
      ss << baseFileName << "_" << i << "_" << (i+1) << ".matches";
      std::cout << "reading matches from " << ss.str() << std::endl;
      std::ifstream in(ss.str());
      if (!in.good())
        throw std::runtime_error("could not read "+ss.str());

      Mesh::SP prev = meshes[i-1];
      Mesh::SP curr = meshes[i];
      
      std::string line;
      while (std::getline(in,line)) {
        int match[2];
        if (line[0] == '#') continue;
        if (2 != sscanf(line.c_str(),"%i %i",&match[0],&match[1])) {
          std::cout << "could not parse line " << line << std::endl;
          continue;
        }
        
        // std::cout << "match " << match[0] << "/" << prev->vertices.size()
        //           << "  --  " << match[1] << "/" << curr->vertices.size()
        //           << std::endl;
        assert(match[1] <= curr->vertices.size());
        assert(match[0] <= prev->vertices.size());
        curr->vertices[match[1]] = prev->vertices[match[0]];
      }
    }

    Object::SP model = std::make_shared<Object>();
    model->meshes = meshes;
    
    Scene::SP scene = std::make_shared<Scene>();
    scene->instances.push_back(std::make_shared<Instance>(model));

    return scene;
  }
  
} // ::mini


void usage(const std::string &msg)
{
  if (!msg.empty()) std::cerr << std::endl << "***Error***: " << msg << std::endl << std::endl;
  std::cout << "Usage: ./ply2brix inFile.pbf -o outfile.mini [--stanford-stitch <N>]" << std::endl;
  std::cout << "Imports a PLY file into brix's scene format.\n";
  std::cout << "(from where it can then be partitioned and/or rendered)\n";
  std::cout << std::endl;
  std::cout << "Note: For scanned models from the stanford model repo, " << std::endl
            << "(ie, david, david v3, stmatthew, atlas, etc)" << std::endl
            << "use --stanford-stitch N, where N is num part files" << std::endl;
  exit(msg != "");
}

int main(int ac, char **av)
{
  std::string inFileName = "";
  std::string outFileName = "";

  int standordStitchParts = 0;
  
  for (int i=1;i<ac;i++) {
    const std::string arg = av[i];
    if (arg == "-o") {
      outFileName = av[++i];
    } else if (arg == "--stanford-stitch") {
      standordStitchParts = std::stoi(av[++i]);
    } else if (arg[0] != '-')
      inFileName = arg;
    else
      usage("unknown cmd line arg '"+arg+"'");
  }
    
  if (inFileName.empty()) usage("no input file name specified");
  if (outFileName.empty()) usage("no output file name base specified");
  
  std::cout << MINI_TERMINAL_BLUE
            << "loading PLY model from " << inFileName
            << MINI_TERMINAL_DEFAULT << std::endl;
  
  mini::Scene::SP scene
    = standordStitchParts
    ? mini::stitchStanford(inFileName,standordStitchParts)
    : mini::loadPLY(inFileName);
  
  std::cout << MINI_TERMINAL_DEFAULT
            << "done importing; saving to " << outFileName
            << MINI_TERMINAL_DEFAULT << std::endl;
  scene->save(outFileName);
  std::cout << MINI_TERMINAL_LIGHT_GREEN
            << "scene saved"
            << MINI_TERMINAL_DEFAULT << std::endl;
  return 0;
}
