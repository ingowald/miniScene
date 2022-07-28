# ======================================================================== #
# Copyright 2021-2022 Ingo Wald                                            #
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

cmake_policy(SET CMP0048 NEW)
project(MiniScene VERSION 0.0.1 LANGUAGES C CXX)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MINI_BUILD_IMPORTERS "Build Importers? (needs more dependencies)" OFF)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.18)
  cmake_policy(SET CMP0104 NEW)
endif()

if (NOT (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR}))
  set(MINI_IS_SUBMODULE ON)
else()
  set(MINI_IS_SUBMODULE OFF)
endif()

# ------------------------------------------------------------------
# general cmake project configs
# ------------------------------------------------------------------
include(cmake/configure_build_type.cmake)
if (MINI_BUILD_IMPORTERS)
include(cmake/FindOpenEXR.cmake)
endif()

# ------------------------------------------------------------------
# owl: there are two ways of using OWL: either as a submodule (if
# building miniScene stand-alone), or, if miniScene itself is a
# submodule, simply include owl _before_ including miniScene, then it
# will pick up the target
# ------------------------------------------------------------------
if(EXISTS "${ROOT}/submodules/owl/CMakeLists.txt")
  message(STATUS "miniScene: found owl as submodule - using it")
  add_subdirectory(submodules/owl EXCLUDE_FROM_ALL)
  set(MINI_HAVE_OWL ON)
elseif(MINI_IS_SUBMODULE AND (TARGET owl::owl))
  message(STATUS "miniScene: owl target exists from another sibling submodule; using it")
  set(MINI_HAVE_OWL ON)
else ()
  message(STATUS "miniScene: owl not available; building host-only version")
  set(MINI_HAVE_OWL OFF)
endif()

if (MINI_HAVE_OWL)
  add_library(mini_owl_common
    INTERFACE
    )
  target_link_libraries(mini_owl_common INTERFACE owl::owl)
  target_compile_definitions(mini_owl_common INTERFACE -DMINI_HAVE_OWL_COMMON=1)
else()
  add_library(mini_owl_common
    INTERFACE
    )
  target_link_libraries(mini_owl_common INTERFACE owl::owl)
  target_include_directories(mini_owl_common INTERFACE ${CMAKE_PROJECT_ROOT}/miniScene)
  target_compile_definitions(mini_owl_common INTERFACE -DMINI_HAVE_OWL_COMMON=1)
endif()

if (MINI_BUILD_IMPORTERS)
  add_subdirectory(submodules/owl/3rdParty/stb_image EXCLUDE_FROM_ALL)
  target_link_libraries(
    mini_owl_common
    INTERFACE
    stb_image
    )
endif()
  

# ------------------------------------------------------------------
# pbrtParser
# ------------------------------------------------------------------
if (MINI_BUILD_IMPORTERS)
  add_subdirectory(submodules/pbrt-parser EXCLUDE_FROM_ALL)
  message(STATUS "pbrtParser configured")
endif()

# ------------------------------------------------------------------
# disney ptex for importer
# ------------------------------------------------------------------
if (MINI_BUILD_IMPORTERS)
  add_subdirectory(submodules/ptex EXCLUDE_FROM_ALL)
  message(STATUS "PTex configured")
endif()

# ------------------------------------------------------------------
# the miniScene library itself
# ------------------------------------------------------------------
add_subdirectory(miniScene)

if (MINI_BUILD_IMPORTERS)
  # ------------------------------------------------------------------
  # various importers from other file formats
  # ------------------------------------------------------------------
  add_subdirectory(importers)
  # add this as a dependecy, to force its building even though the
  # pbrtparser is includedvia EXCLUDE_FROM_ALL
  add_dependencies(pbf2mini pbrt2pbf)
endif()

# ------------------------------------------------------------------
# various small mini-apps to inscpect/modify miniScenes
# ------------------------------------------------------------------
add_subdirectory(tools)






