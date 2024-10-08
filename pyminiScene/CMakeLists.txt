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

# Ensure that the Python interpreter and libraries are available
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# Find pybind11 (it will look for installed pybind11, either system-wide or via pip)
find_package(pybind11 REQUIRED)

# Add the source file(s) that will be compiled into the Python module
add_library(pyminiScene MODULE pyscene.cpp)

# Link against pybind11::module and Python3 components
target_link_libraries(pyminiScene PRIVATE pybind11::module Python3::Python miniScene)

# Optional: specify the output directory for the built Python module
#set_target_properties(pyminiScene PROPERTIES PREFIX "")

# Enable the position-independent code flag (PIC) which is required for shared libraries
set_target_properties(pyminiScene PROPERTIES CXX_VISIBILITY_PRESET hidden)

# For Windows: Ensure we get a .pyd instead of a .dll
if(WIN32)
    set_target_properties(pyminiScene PROPERTIES SUFFIX ".pyd")
endif()