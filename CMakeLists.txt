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
option(MINI_IMPORT_OWL "Import OWL from submodules? (needs more dependencies)" OFF)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.18)
  cmake_policy(SET CMP0104 NEW)
endif()

if (NOT (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR}))
  set(MINI_IS_SUBPROJECT ON)
else()
  set(MINI_IS_SUBPROJECT OFF)
endif()

# ------------------------------------------------------------------
# check if we have CUDA
# ------------------------------------------------------------------
if (MINI_IMPORT_OWL)
  option(MINI_DISABLE_CUDA "Disable CUDA" OFF)
else()
  set(MINI_DISABLE_CUDA ON)
endif()

include(CheckLanguage)
if (MINI_DISABLE_CUDA)
  message(STATUS "#mini: Disabling CUDA support upon user request")
  set(MINI_HAVE_CUDA OFF)
else()
  check_language(CUDA)
  if (CMAKE_CUDA_COMPILER)
    set(MINI_HAVE_CUDA ON)
    enable_language(CUDA)
  else()
    set(MINI_HAVE_CUDA OFF)
  endif()
endif()

# ------------------------------------------------------------------
# general cmake project configs
# ------------------------------------------------------------------
include(cmake/configure_build_type.cmake)
if (MINI_BUILD_IMPORTERS)
include(cmake/FindOpenEXR.cmake)
endif()

# ------------------------------------------------------------------
# owl:
# ------------------------------------------------------------------
if (MINI_HAVE_CUDA AND MINI_IMPORT_OWL)
  add_subdirectory(submodules/owl EXCLUDE_FROM_ALL)
  add_library(mini_owl_common
    INTERFACE
    )
  target_link_libraries(mini_owl_common INTERFACE owl::owl)
  target_compile_definitions(mini_owl_common INTERFACE -DMINI_HAVE_OWL_COMMON=1)
else()
  add_library(mini_owl_common
    INTERFACE
    )
  target_include_directories(mini_owl_common
    INTERFACE
    submodules/owl/owl/include/
    )
  if (MINI_BUILD_IMPORTERS)
    add_subdirectory(submodules/owl/3rdParty/stb_image EXCLUDE_FROM_ALL)
    target_link_libraries(miniScene
      mini_owl_common
      PUBLIC
      stb_image
  )
  endif()
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






