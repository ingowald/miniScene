// ======================================================================== //
// Copyright 2018-2022 Ingo Wald                                            //
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

#pragma once

/*! this library can be built in two ways; either with the 'owl'
    library available to this library, or without. In the first case,
    miniScene uses the owl::common classes for vec3f, affine3f, etc,
    and just imports those into the mini namespace; without owl, it
    includes 'its own' copy of the relevant header files, which are
    basically a copy of the owl ones (at a given point in time),
    frozen in time, and put into another namespace to avoid namespace
    conflicts if used as a submodule. This flag gets passed down from
    the CMakeLists.txt. */
#if MINI_HAVE_OWL_COMMON
#  include "owl/common/math/AffineSpace.h"
// #  include "owl/common/arrayND/array3D.h"
#  include "owl/common/parallel/parallel_for.h"
#else
#  include "miniScene/common/math/AffineSpace.h"
#  include "miniScene/common/parallel/parallel_for.h"
#endif
// std
#include <sstream>
#include <string>
#include <string.h>
#include <mutex>
#include <stdexcept>
#include <set>
#include <map>
#include <vector>
#include <queue>

#define NOTIMPLEMENTED throw std::runtime_error(std::string(__PRETTY_FUNCTION__)+" not implemented")

#ifdef WIN32
# define MINI_TERMINAL_RED ""
# define MINI_TERMINAL_GREEN ""
# define MINI_TERMINAL_YELLOW ""
# define MINI_TERMINAL_BLUE ""
# define MINI_TERMINAL_LIGHT_BLUE ""
# define MINI_TERMINAL_RESET ""
# define MINI_TERMINAL_DEFAULT MINI_TERMINAL_RESET
# define MINI_TERMINAL_LIGHT_GREEN ""
# define MINI_TERMINAL_BOLD ""

# define MINI_TERMINAL_MAGENTA ""
# define MINI_TERMINAL_LIGHT_MAGENTA ""
# define MINI_TERMINAL_CYAN ""
# define MINI_TERMINAL_LIGHT_RED ""
#else
# define MINI_TERMINAL_RED "\033[1;31m"
# define MINI_TERMINAL_GREEN "\033[1;32m"
# define MINI_TERMINAL_YELLOW "\033[1;33m"
# define MINI_TERMINAL_BLUE "\033[1;34m"
# define MINI_TERMINAL_LIGHT_BLUE "\033[1;34m"
# define MINI_TERMINAL_LIGHT_GREEN "\033[1;32m"
# define MINI_TERMINAL_RESET "\033[0m"
# define MINI_TERMINAL_DEFAULT MINI_TERMINAL_RESET
# define MINI_TERMINAL_BOLD "\033[1;1m"

# define MINI_TERMINAL_MAGENTA "\e[35m"
# define MINI_TERMINAL_LIGHT_MAGENTA "\e[95m"
# define MINI_TERMINAL_CYAN "\e[36m"
# define MINI_TERMINAL_LIGHT_RED "\033[1;31m"
#endif

namespace mini {
  
#if MINI_HAVE_OWL_COMMON
  using namespace owl;
  namespace common {
    using namespace owl::common;
  }
#endif
  using namespace mini::common;
  
  namespace common {
    
    inline bool endsWith(const std::string &s, const std::string &suffix)
    {
      return s.substr(s.size()-suffix.size(),suffix.size()) == suffix;
    }

    inline int iDivUp(int a, int b) { return (a+b-1)/b; }
  }
} // ::mini

