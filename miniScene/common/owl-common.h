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

/* copied from OWL project, and put into new namespace to avoid naming conflicts.*/

#pragma once

#ifndef _USE_MATH_DEFINES
#  define _USE_MATH_DEFINES
#endif
#include <math.h> // using cmath causes issues under Windows

#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <assert.h>
#include <string>
#include <math.h>
#include <cmath>
#include <algorithm>
#include <sstream>
#ifdef __GNUC__
#include <execinfo.h>
#include <sys/time.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

#if !defined(WIN32)
#include <signal.h>
#endif

#if defined(_MSC_VER)
#  define MINI_DLL_EXPORT __declspec(dllexport)
#  define MINI_DLL_IMPORT __declspec(dllimport)
#elif defined(__clang__) || defined(__GNUC__)
#  define MINI_DLL_EXPORT __attribute__((visibility("default")))
#  define MINI_DLL_IMPORT __attribute__((visibility("default")))
#else
#  define MINI_DLL_EXPORT
#  define MINI_DLL_IMPORT
#endif

// #if 1
# define MINI_INTERFACE /* nothing - currently not building any special 'owl.dll' */
// #else
// //#if defined(MINI_DLL_INTERFACE)
// #  ifdef owl_EXPORTS
// #    define MINI_INTERFACE MINI_DLL_EXPORT
// #  else
// #    define MINI_INTERFACE MINI_DLL_IMPORT
// #  endif
// //#else
// //#  define MINI_INTERFACE /*static lib*/
// //#endif
// #endif

//#ifdef __WIN32__
//#define  __PRETTY_FUNCTION__ __FUNCTION__ 
//#endif
#if defined(_MSC_VER)
//&& !defined(__PRETTY_FUNCTION__)
#  define __PRETTY_FUNCTION__ __FUNCTION__
#endif


#ifndef PRINT
# define PRINT(var) std::cout << #var << "=" << var << std::endl;
#ifdef __WIN32__
# define PING std::cout << __FILE__ << "::" << __LINE__ << ": " << __FUNCTION__ << std::endl;
#else
# define PING std::cout << __FILE__ << "::" << __LINE__ << ": " << __PRETTY_FUNCTION__ << std::endl;
#endif
#endif

#if defined(__CUDA_ARCH__) || defined(__HIP__)
// #if defined(__CUDA_ARCH__) || defined(__HIP_DEVICE_COMPILE__)
# define __owl_device   __device__
# define __owl_host     __host__
#elif !defined(__owl_device)
# define __owl_device   /* ignore */
# define __owl_host     /* ignore */
#endif

# define __both__   __owl_host __owl_device


#ifdef __GNUC__
#define MAYBE_UNUSED __attribute__((unused))
#else
#define MAYBE_UNUSED
#endif

// namespace detail {
// inline static std::string backtrace()
// {
// #ifdef __GNUC__
//     static const int max_frames = 16;

//     void* buffer[max_frames] = { 0 };
//     int cnt = ::backtrace(buffer,max_frames);

//     char** symbols = backtrace_symbols(buffer,cnt);

//     if (symbols) {
//       std::stringstream str;
//       for (int n = 1; n < cnt; ++n) // skip the 1st entry (address of this function)
//       {
//         str << symbols[n] << '\n';
//       }
//       free(symbols);
//       return str.str();
//     }
//     return "";
// #else
//     return "not implemented yet";
// #endif
// }

// inline void owlRaise_impl(std::string str)
// {
//   fprintf(stderr,"%s\n",str.c_str());
// #ifdef WIN32
//   if (IsDebuggerPresent())
//     DebugBreak();
//   else
//     throw std::runtime_error(str);
// #else
// #ifndef NDEBUG
//   std::string bt = ::detail::backtrace();
//   fprintf(stderr,"%s\n",bt.c_str());
// #endif
//   raise(SIGINT);
// #endif
// }
// }

// #define MINI_RAISE(MSG) ::detail::owlRaise_impl(MSG);


#define MINI_NOTIMPLEMENTED throw std::runtime_error(std::string(__PRETTY_FUNCTION__)+" not implemented")

// #ifdef WIN32
// # define MINI_TERMINAL_RED ""
// # define MINI_TERMINAL_GREEN ""
// # define MINI_TERMINAL_LIGHT_GREEN ""
// # define MINI_TERMINAL_YELLOW ""
// # define MINI_TERMINAL_BLUE ""
// # define MINI_TERMINAL_LIGHT_BLUE ""
// # define MINI_TERMINAL_RESET ""
// # define MINI_TERMINAL_DEFAULT MINI_TERMINAL_RESET
// # define MINI_TERMINAL_BOLD ""

// # define MINI_TERMINAL_MAGENTA ""
// # define MINI_TERMINAL_LIGHT_MAGENTA ""
// # define MINI_TERMINAL_CYAN ""
// # define MINI_TERMINAL_LIGHT_RED ""
// #else
// # define MINI_TERMINAL_RED "\033[0;31m"
// # define MINI_TERMINAL_GREEN "\033[0;32m"
// # define MINI_TERMINAL_LIGHT_GREEN "\033[1;32m"
// # define MINI_TERMINAL_YELLOW "\033[1;33m"
// # define MINI_TERMINAL_BLUE "\033[0;34m"
// # define MINI_TERMINAL_LIGHT_BLUE "\033[1;34m"
// # define MINI_TERMINAL_RESET "\033[0m"
// # define MINI_TERMINAL_DEFAULT MINI_TERMINAL_RESET
// # define MINI_TERMINAL_BOLD "\033[1;1m"

// # define MINI_TERMINAL_MAGENTA "\e[35m"
// # define MINI_TERMINAL_LIGHT_MAGENTA "\e[95m"
// # define MINI_TERMINAL_CYAN "\e[36m"
// # define MINI_TERMINAL_LIGHT_RED "\033[1;31m"
// #endif

#ifdef _MSC_VER
# define MINI_ALIGN(alignment) __declspec(align(alignment)) 
#else
# define MINI_ALIGN(alignment) __attribute__((aligned(alignment)))
#endif



namespace mini {
  namespace common {

#ifdef __CUDA_ARCH__
    using ::min;
    using ::max;
    // inline __both__ float abs(float f)      { return fabsf(f); }
    // inline __both__ double abs(double f)    { return fabs(f); }
    using std::abs;
    // inline __both__ float sin(float f) { return ::sinf(f); }
    // inline __both__ double sin(double f) { return ::sin(f); }
    // inline __both__ float cos(float f) { return ::cosf(f); }
    // inline __both__ double cos(double f) { return ::cos(f); }

#else
    using std::min;
    using std::max;
    using std::abs;
    // inline __both__ double sin(double f) { return ::sin(f); }
    // inline __both__ float saturate(const float &f) { return min(1.f,max(0.f,f)); }
#endif

    // inline __both__ float abs(float f)      { return fabsf(f); }
    // inline __both__ double abs(double f)    { return fabs(f); }
    inline __both__ float rcp(float f)      { return 1.f/f; }
    inline __both__ double rcp(double d)    { return 1./d; }
  
    inline __both__ int divRoundUp(int a, int b) { return (a+b-1)/b; }
    inline __both__ unsigned int divRoundUp(unsigned int a, unsigned int b) { return (a+b-1)/b; }
    inline __both__ long divRoundUp(long a, long b) { return (a+b-1)/b; }
    inline __both__ unsigned long divRoundUp(unsigned long a, unsigned long b) { return (a+b-1)/b; }
    inline __both__ long long divRoundUp(long long a, long long b) { return (a+b-1)/b; }
    inline __both__ unsigned long long divRoundUp(unsigned long long a, unsigned long long b) { return (a+b-1)/b; }
  
// #ifdef __CUDA_ARCH__
//     using ::sin; // this is the double version
//     // inline __both__ float sin(float f) { return ::sinf(f); }
//     using ::cos; // this is the double version
//     // inline __both__ float cos(float f) { return ::cosf(f); }
// #else
    using ::sin; // this is the double version
    using ::cos; // this is the double version
// #endif

    /*! namespace that offers polymorphic overloads of functions like
        sqrt, rsqrt, sin, cos, etc (that vary based on float vs
        double), and that is NOT in a default namespace where ti
        would/could clash with cuda or system-defines of the same name
        - TODO: make sure that cos, sin, abs, etc are also properly
        handled here. */
    namespace polymorphic {
#ifdef __CUDA_ARCH__
      inline __both__ float sqrt(const float f)     { return ::sqrtf(f); }
      inline __both__ double sqrt(const double d)   { return ::sqrt(d); }
#else
      inline __both__ float sqrt(const float f)     { return ::sqrtf(f); }
      inline __both__ double sqrt(const double d)   { return ::sqrt(d); }
#endif
      
      inline __both__ float rsqrt(const float f)    { return 1.f/mini::common::polymorphic::sqrt(f); }
      inline __both__ double rsqrt(const double d)  { return 1./mini::common::polymorphic::sqrt(d); }
    }
    

#ifdef __WIN32__
#  define osp_snprintf sprintf_s
#else
#  define osp_snprintf snprintf
#endif
  
    /*! added pretty-print function for large numbers, printing 10000000 as "10M" instead */
    inline std::string prettyDouble(const double val) {
      const double absVal = abs(val);
      char result[1000];

      if      (absVal >= 1e+18f) osp_snprintf(result,1000,"%.1f%c",float(val/1e18f),'E');
      else if (absVal >= 1e+15f) osp_snprintf(result,1000,"%.1f%c",float(val/1e15f),'P');
      else if (absVal >= 1e+12f) osp_snprintf(result,1000,"%.1f%c",float(val/1e12f),'T');
      else if (absVal >= 1e+09f) osp_snprintf(result,1000,"%.1f%c",float(val/1e09f),'G');
      else if (absVal >= 1e+06f) osp_snprintf(result,1000,"%.1f%c",float(val/1e06f),'M');
      else if (absVal >= 1e+03f) osp_snprintf(result,1000,"%.1f%c",float(val/1e03f),'k');
      else if (absVal <= 1e-12f) osp_snprintf(result,1000,"%.1f%c",float(val*1e15f),'f');
      else if (absVal <= 1e-09f) osp_snprintf(result,1000,"%.1f%c",float(val*1e12f),'p');
      else if (absVal <= 1e-06f) osp_snprintf(result,1000,"%.1f%c",float(val*1e09f),'n');
      else if (absVal <= 1e-03f) osp_snprintf(result,1000,"%.1f%c",float(val*1e06f),'u');
      else if (absVal <= 1e-00f) osp_snprintf(result,1000,"%.1f%c",float(val*1e03f),'m');
      else osp_snprintf(result,1000,"%f",(float)val);

      return result;
    }
  

    /*! return a nicely formatted number as in "3.4M" instead of
        "3400000", etc, using mulitples of thousands (K), millions
        (M), etc. Ie, the value 64000 would be returned as 64K, and
        65536 would be 65.5K */
    inline std::string prettyNumber(const size_t s)
    {
      char buf[1000];
      if (s >= (1000LL*1000LL*1000LL*1000LL)) {
        osp_snprintf(buf, 1000,"%.2fT",s/(1000.f*1000.f*1000.f*1000.f));
      } else if (s >= (1000LL*1000LL*1000LL)) {
        osp_snprintf(buf, 1000, "%.2fG",s/(1000.f*1000.f*1000.f));
      } else if (s >= (1000LL*1000LL)) {
        osp_snprintf(buf, 1000, "%.2fM",s/(1000.f*1000.f));
      } else if (s >= (1000LL)) {
        osp_snprintf(buf, 1000, "%.2fK",s/(1000.f));
      } else {
        osp_snprintf(buf,1000,"%zi",s);
      }
      return buf;
    }

    /*! return a nicely formatted number as in "3.4M" instead of
        "3400000", etc, using mulitples of 1024 as in kilobytes,
        etc. Ie, the value 65534 would be 64K, 64000 would be 63.8K */
    inline std::string prettyBytes(const size_t s)
    {
      char buf[1000];
      if (s >= (1024LL*1024LL*1024LL*1024LL)) {
        osp_snprintf(buf, 1000,"%.2fT",s/(1024.f*1024.f*1024.f*1024.f));
      } else if (s >= (1024LL*1024LL*1024LL)) {
        osp_snprintf(buf, 1000, "%.2fG",s/(1024.f*1024.f*1024.f));
      } else if (s >= (1024LL*1024LL)) {
        osp_snprintf(buf, 1000, "%.2fM",s/(1024.f*1024.f));
      } else if (s >= (1024LL)) {
        osp_snprintf(buf, 1000, "%.2fK",s/(1024.f));
      } else {
        osp_snprintf(buf,1000,"%zi",s);
      }
      return buf;
    }
  
    inline double getCurrentTime()
    {
#ifdef _WIN32
      SYSTEMTIME tp; GetSystemTime(&tp);
      /*
         Please note: we are not handling the "leap year" issue.
     */
      size_t numSecsSince2020
          = tp.wSecond
          + (60ull) * tp.wMinute
          + (60ull * 60ull) * tp.wHour
          + (60ull * 60ul * 24ull) * tp.wDay
          + (60ull * 60ul * 24ull * 365ull) * (tp.wYear - 2020);
      return double(numSecsSince2020 + tp.wMilliseconds * 1e-3);
#else
      struct timeval tp; gettimeofday(&tp,nullptr);
      return double(tp.tv_sec) + double(tp.tv_usec)/1E6;
#endif
    }

    inline bool hasSuffix(const std::string &s, const std::string &suffix)
    {
      return s.substr(s.size()-suffix.size()) == suffix;
    }
    
  } // ::mini::common
} // ::owl
