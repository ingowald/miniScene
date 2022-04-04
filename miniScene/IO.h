// ======================================================================== //
// Copyright 2018-2020 Ingo Wald                                            //
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

#include "miniScene/common.h"
// std
#include <fstream>

namespace mini {
    namespace io {
      
      template<typename T>
      inline bool safe_to_copy_binary()
      { return std::is_trivially_copyable<T>::value; }

      template<> inline bool safe_to_copy_binary<vec3i>() { return true; }
      template<> inline bool safe_to_copy_binary<vec3f>() { return true; }
      template<> inline bool safe_to_copy_binary<vec4i>() { return true; }
      template<> inline bool safe_to_copy_binary<vec4f>() { return true; }

      template<typename T>
      inline void readElement(std::istream &in, T &t)
      {
        assert(in.good());
        in.read((char*)&t,sizeof(t));
        if (!in.good())
          throw std::runtime_error("partial read");
      }

      template<typename T>
      inline void readArray(std::istream &in, T *t, size_t N)
      {
        assert(in.good());
        size_t numBytes = N*sizeof(T);
        in.read((char*)t,numBytes);
        if (!in.good())
          throw std::runtime_error("partial read");
      }

      template<typename T>
      inline void readVector(std::istream &in,
                             std::vector<T> &t,
                             const std::string &description="<no description>")
      {
        size_t N;
        readElement(in,N);
        t.resize(N);
        if (safe_to_copy_binary<T>())
          in.read((char*)t.data(),N*sizeof(t[0]));
        else
          for (size_t i=0;i<N;i++)
            readElement(in,t[i]);
      }

      template<typename T>
      inline void writeElement(std::ostream &out, const T &t)
      {
        assert(out.good());
        out.write((char*)&t,sizeof(t));
        assert(out.good());
      }

      template<typename T>
      inline void writeArray(std::ostream &out, const T *t, size_t N)
      {
        if (safe_to_copy_binary<T>())
          out.write((char*)t,N*sizeof(t[0]));
        else
          for (size_t i=0;i<N;i++)
            writeElement(out,t[i]);
        assert(out.good());
      }
    
      template<typename T>
      void writeVector(std::ostream &out, const std::vector<T> &vt)
      {
        size_t N = vt.size();
        writeElement(out,N);
        if (safe_to_copy_binary<T>())
          out.write((char*)vt.data(),N*sizeof(vt[0]));
        else
          for (auto &v : vt)
            writeElement(out,v);
        assert(out.good());
      }

      template<typename T>
      inline T readElement(std::istream &in)
      {
        T t;
        io::readElement(in,t);
        return t;
      }
    
      inline void readString(std::istream &in, std::string &s)
      {
        int size;
        readElement(in,size);
        s = std::string(size,' ');
        readArray(in,s.data(),size);
      }
    
      inline void writeString(std::ostream &out, const std::string &s)
      {
        int size = (int)s.size();
        writeElement(out,size);
        writeArray(out,s.data(),size);
      }
    
      inline std::string readString(std::istream &in)
      {
        std::string s;
        readString(in,s);
        return s;
      }

    } // ::mini::io
} // ::mini
