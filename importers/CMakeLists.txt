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

  add_executable(lavai32f32toMini
    lavai32f32toMini.cpp
    )
  target_link_libraries(lavai32f32toMini
    miniScene
    miniScene_import_obj
    )
    
endif()







