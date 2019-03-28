/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#pragma once
#include <iostream>
#include <cstring> // strlen
#include <math.h> // fabs, pow, floor, ceil
#include <vector>
#include <string>


namespace yskip {


// constants
const int BUFF_SIZE = 1024*1024;
const int SUCCESS   = 0;
const int FAILURE   = 1;

// 
typedef float real_t;
typedef uint64_t count_t;
 
 
inline std::vector<std::string> tokenize(const char* s) {
  
  std::vector<std::string> tokens;
  const char* begin = s;
  while (1) {
    if (*s == ' ' or *s == '\0') {
      if (begin < s) { // ignore empty string
	tokens.push_back(std::string(begin, s));
      }
      if (*s == '\0') {
	break;
      }
      ++s;
      begin = s;
    }else {
      ++s;
    }
  }
  return tokens;
}



inline bool mystrcmp(const char* begin, const char* end, const std::string &str) {

  if (end - begin == str.length()) {
    const char* s1 = begin;
    const char* s2 = str.c_str();
    while (s1 != end) {
      if (*s1 == *s2) {
	++s1;
	++s2;
      }else {
	return false;
      }
    }
    return true;
  }else {
    return false;
  }
}


// FNV-1a hash algorithm
inline uint32_t fnv1a(const char* begin, const char* end) {

  uint32_t hash_val = 2166136261;
  for (const char* s = begin; s != end; ++s) {
    hash_val ^= *s;
    hash_val *= 16777619;
  }
  return hash_val;
}
 

// fast inverse square root
inline float invsqrt(float x) {
  
  uint32_t i;
  memcpy(&i, &x, sizeof(i)); // i = *(uint32_t*)&x;
  i = 0x5f3759df - (i >> 1);
  memcpy(&x, &i, sizeof(x));
  return x;
}


inline double get_sec(const timeval& tv){

  return ((double)(tv.tv_sec) + (double)(tv.tv_usec) * 0.001 * 0.001);
}


inline double interval(const timeval& tv1, const timeval& tv2){

  return get_sec(tv2) - get_sec(tv1);
}


inline bool approx_equal(real_t v1, real_t v2) {

  return fabs(v1 - v2) < 1.0e-5 || fabs(v1 - v2)/std::max<real_t>(fabs(v1), fabs(v2)) < 1.0e-4;
}


/*
 * macros
 */
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);		   \
  void operator=(const TypeName&)

#define _STR(x)      #x
#define _STR2(x)     _STR(x)
#define __SLINE__    _STR2(__LINE__)
#define HERE         __FILE__ "(" __SLINE__ ")"


}
