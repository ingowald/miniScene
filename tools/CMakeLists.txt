# ======================================================================== #
# Copyright 2018-2022 Ingo Wald                                            #
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

# -----------------------------------------------------------------------------
# tool that reads a scene, and then prints some basic information
# -----------------------------------------------------------------------------
add_executable(miniInfo
  info.cpp
  )
target_link_libraries(miniInfo
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# 'lint' like tool that runs some basic sanity tests such as checking indices
# -----------------------------------------------------------------------------
add_executable(miniLint
  lint.cpp
  )
target_link_libraries(miniLint
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# tool that reads a scene, find all single-owner 'root' objects, and
# splits these objects into their constituent meshes; createing one
# independent instance per such mesh in this object
# -----------------------------------------------------------------------------
add_executable(miniSeparateRootMeshes
  separateRootMeshes.cpp
  )
target_link_libraries(miniSeparateRootMeshes
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# tool that takes a given scene, and creates a new scene that contains
# exactly the same triangles, but in a way that each object in the
# scene has only a certain maximum number of triangles. If a object
# has more triangles than desired we first try to split that object
# based on the possibly mulitple meshes it has; if splitting based on
# meshes is not enoguh we will also split large meshes into multiple
# smaller ones until the desired threshold is reached.
# -----------------------------------------------------------------------------
add_executable(miniBreakLargeMeshes
  breakLargeMeshes.cpp
  )
target_link_libraries(miniBreakLargeMeshes
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# tool primarily for debugging: given a mini scene, write out *flat*
# scene in OBJ file format (no materials or textures). Warning: files
# tend to get *very* large if mini scene contained instances.
# -----------------------------------------------------------------------------
add_executable(mini2obj
  mini2obj.cpp
  )
target_link_libraries(mini2obj
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# tool that takes some mini files, and merges them into one scene by
# simply appending the different scenes' instances (i.e., without
# flattening anything or merging instances)
# -----------------------------------------------------------------------------
add_executable(miniMerge
  merge.cpp
  )
target_link_libraries(miniMerge
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# subdivide
# -----------------------------------------------------------------------------
add_executable(miniSubdivide
  subdivide.cpp
  )
target_link_libraries(miniSubdivide
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# tool to take a scene, and replicate all its instances a given number
# of times
# -----------------------------------------------------------------------------
add_executable(miniReplicate
  miniReplicate.cpp
  )
target_link_libraries(miniReplicate
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# load a scene, create a (world space) bounding box for each triangle,
# and store list of those bbs (for some external bvh builder tests)
# -----------------------------------------------------------------------------
add_executable(miniDumpBBs
  dumpBBs.cpp
  )
target_link_libraries(miniDumpBBs
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# tool that reads a scene, and then prints some basic information
# -----------------------------------------------------------------------------
add_executable(miniGenScaleTest
  genScaleTest.cpp
  )
target_link_libraries(miniGenScaleTest
  PUBLIC
  miniScene
  )

# -----------------------------------------------------------------------------
# a trivially simple owl-based viewer, to sanity test ... don't expct
# much, this only shows flat triangles
# -----------------------------------------------------------------------------
if (TARGET owl::owl)
  add_subdirectory(viewer)
endif()



