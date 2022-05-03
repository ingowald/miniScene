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
project(MiniScene VERSION 0.0.1 LANGUAGES C CXX CUDA)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MINI_BUILD_IMPORTERS "Build Importers? (needs more dependencies)" ON)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.18)
  cmake_policy(SET CMP0104 NEW)
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
add_subdirectory(submodules/owl EXCLUDE_FROM_ALL)
message(STATUS "OWl configured")
# ------------------------------------------------------------------
# pbrtParser
# ------------------------------------------------------------------
add_subdirectory(submodules/pbrt-parser EXCLUDE_FROM_ALL)
message(STATUS "pbrtParser configured")

# ------------------------------------------------------------------
# disney ptex for importer
# ------------------------------------------------------------------
if (MINI_BUILD_IMPORTERS)
add_subdirectory(submodules/ptex EXCLUDE_FROM_ALL)
message(STATUS "PTex configured")
endif()

# ------------------------------------------------------------------
# cuda tools for pbrt scene
# ------------------------------------------------------------------
add_subdirectory(miniScene)
add_dependencies(miniScene pbrt2pbf)


