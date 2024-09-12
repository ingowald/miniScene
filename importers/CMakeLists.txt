# ======================================================================== #
# Copyright 2018-2021 Ingo Wald                                            #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ======================================================================== #


# ==================================================================
# imports wavefront OBJ (w/ most material stuff) and textures; every
# file becomes one single unit-instance with single object that
# ocntains all meshes
# ==================================================================
if (MINI_BUILD_SIMPLE_IMPORTERS)
  add_executable(obj2mini
    obj2mini.cpp
    )
  target_link_libraries(obj2mini
    miniScene_import_obj
    )
  # ==================================================================
  # imports STL files
  # ==================================================================
  add_executable(stl2mini
    stl2mini.cpp
    )
  target_link_libraries(stl2mini
    miniScene
    )

  # ==================================================================
  # imports PLY models via the happly parser (in submodules/)
  # ==================================================================
  add_executable(ply2mini
    ply2mini.cpp
    )
  target_link_libraries(ply2mini
    miniScene
    )
  target_include_directories(ply2mini PUBLIC ${PROJECT_SOURCE_DIR}/submodules/)

  # ==================================================================
  # imports (my own) "binmesh" models of the form "size_t numVtx; vec3f
  # vtx[numVtx]; size_t numIdx; vec3i idx[numIdx]".
  # ==================================================================

  # IMporter from binmesh
  add_executable(binmesh2mini
    binmesh2mini.cpp
    )
  target_link_libraries(binmesh2mini
    miniScene
    )

  # EXporter to binmesh
  add_executable(mini2binmesh
    mini2binmesh.cpp
    )
  target_link_libraries(mini2binmesh
    miniScene
    )

  # IMporter from bobj
  add_library(miniScene_import_obj STATIC
    importOBJ.cpp
    )
  target_link_libraries(miniScene_import_obj PUBLIC miniScene)
  
  add_executable(bobj2mini
    bobj2mini.cpp
    )
  target_link_libraries(bobj2mini
    miniScene
    miniScene_import_obj
    )

endif()




#option(MINI_BUILD_XMLBIN_IMPORTER "Build Embree xml/bin importer" OFF)
#if (MINI_BUILD_XMLBIN_IMPORTER)
#find_package(LibXml2 REQUIRED)
find_package(LibXml2)
if (LibXml2_FOUND)
  add_executable(xmlbin2mini
    xmlbin2mini.cpp
    )
  target_link_libraries(xmlbin2mini
    PUBLIC miniScene LibXml2::LibXml2
    )
endif()




if (MINI_BUILD_ADVANCED_IMPORTERS)
  # TODO: make this importer/converter - and all the pbrt and cup stuff
  # that is only required for this tool - optional...
  add_executable(pbf2mini
    pbf2mini.cpp
    cup_tools/tessellateCurves.cpp
    cup_tools/removeAllNonMeshShapes.cpp
    )
  target_link_libraries(pbf2mini
    PUBLIC
    mini_owl_common
    miniScene
    pbrtParser
    )
  # add this as a dependency, to force its building even though the
  # pbrtparser is included via EXCLUDE_FROM_ALL
  add_dependencies(pbf2mini pbrt2pbf)

  if (OPENEXR_FOUND)
    target_link_libraries(pbf2mini
      PUBLIC #OpenEXR::OpenEXR
      ${OPENEXR_LIBRARIES}
      )
    target_include_directories(pbf2mini
      PUBLIC
      ${OPENEXR_INCLUDES}
      )
    target_compile_definitions(pbf2mini
      PUBLIC HAVE_OPENEXR=1
      )
  endif()



  # ==================================================================
  # scene tool to bake out moana pbrt+ptex into "baked ptex textures",
  # so the importers/pbrt importer can then replace ptex textures with
  # those bakes ones...
  # ==================================================================
  add_executable(miniBakePtex
    bakeOutPtex.cpp
    )
  target_include_directories(miniBakePtex
    PRIVATE
    ${PROJECT_SOURCE_DIR}/submodules/ptex/src/ptex/
    )
  target_link_libraries(miniBakePtex
    miniScene
    pbrtParser
    Ptex_static
    )
endif()


