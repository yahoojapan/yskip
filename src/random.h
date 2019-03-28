/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#pragma once
#include <math.h> // ceil, floor
#include <random> // mt19937
#include "util.h"


namespace yskip {


class Random {
 public:
  Random();
  Random(const int seed);
  ~Random() {};
  int uniform(const int min, const int max);
  template<class T> T uniform(const T min, const T max);
  template<class T> int round(const T x);
  
 private:
  DISALLOW_COPY_AND_ASSIGN(Random);
  std::mt19937 mt_;
};


inline Random::Random() {

  mt_ = std::mt19937(0);
}


inline Random::Random(const int seed) {

  mt_ = std::mt19937(seed);
}
 

inline int Random::uniform(const int min, const int max) {

#ifdef __YSKIP_DEBUG__
  assert(min < max);
#endif
  
  return min + mt_()%(max - min);
}


template<class T>
inline T Random::uniform(const T min, const T max) {

#ifdef __YSKIP_DEBUG__
  assert(min < max);
#endif

  return min + (max - min)*static_cast<T>(mt_())/static_cast<T>(mt_.max() - mt_.min());
}


template<class T>
inline int Random::round(const T x) {

  int c = ceil(x);
  int f = floor(x);
  if (uniform(0.0, 1.0) < x - static_cast<double>(f)) {
    return c;
  }else {
    return f;
  }
}

 
}
