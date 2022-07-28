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
  Scene.cpp
  Serialized.cpp
  )
target_link_libraries(miniScene
  PUBLIC
  mini_owl_common
#  stb_image
  )
target_include_directories(miniScene
  PUBLIC
  ${PROJECT_SOURCE_DIR}
  )

set_target_properties(miniScene PROPERTIES POSITION_INDEPENDENT_CODE ON)


