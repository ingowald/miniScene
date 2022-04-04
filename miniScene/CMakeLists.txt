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

# ------------------------------------------------------------------
# the mini "scene graph" library itself; similar to the one in the
# pbrtparser, but using a single, simpler material model, and not all
# the different variations of lights, materials, film, etc, that pbrt
# has.
# ------------------------------------------------------------------
add_library(miniScene STATIC
  LocalScene.cpp
  SplitSpecs.cpp
  Scene.cpp
  Serialized.cpp
  )
target_link_libraries(miniScene
  PUBLIC
  owl::owl
  stb_image
  )
target_include_directories(miniScene
  PUBLIC
  ${PROJECT_SOURCE_DIR}
  )

# ------------------------------------------------------------------
# tool that reads a scene, and then prints some basic information
# ------------------------------------------------------------------
add_executable(miniInfo
  info.cpp
  )
target_link_libraries(miniInfo
  PUBLIC
  miniScene
  )

# ------------------------------------------------------------------
# tool that reads a scene, find all single-owner 'root' objects, and
# splits these objects into their constituent meshes; createing one
# independent instance per such mesh in this object
# ------------------------------------------------------------------
add_executable(miniSeparateRootMeshes
  separateRootMeshes.cpp
  )
target_link_libraries(miniSeparateRootMeshes
  PUBLIC
  miniScene
  )

# ------------------------------------------------------------------
# tool primarily for debugging: given a mini scene, write out *flat*
# scene in OBJ file format (no materials or textures). Warning: files
# tend to get *very* large if mini scene contained instances.
# ------------------------------------------------------------------
add_executable(mini2obj
  mini2obj.cpp
  )
target_link_libraries(mini2obj
  PUBLIC
  miniScene
  )

# ------------------------------------------------------------------
# tool to take a scene, and replicate all its instances a given number
# of times
# ------------------------------------------------------------------
add_executable(miniReplicate
  miniReplicate.cpp
  )
target_link_libraries(miniReplicate
  PUBLIC
  miniScene
  )

# ------------------------------------------------------------------
# importers from pbrt, obj, and whatever else gets added later
# ------------------------------------------------------------------

add_subdirectory(importers)

# ------------------------------------------------------------------
# a viewer, to sanity test ...
# ------------------------------------------------------------------
add_subdirectory(viewer)


