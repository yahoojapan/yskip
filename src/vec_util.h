/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#pragma once
#include <iostream>
#include <sstream>
#include <cstring> // strlen
#include <math.h> // fabs, pow, floor, ceil
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <stdexcept>
#include "util.h"


namespace yskip {


inline void mul_add(const real_t a, const real_t* first1, const real_t* last1, real_t* first2) {

  const real_t* it1 = first1;
  real_t* it2       = first2;
  
  while (it1 != last1) {
    *it2 += a * (*it1);
    ++it1;
    ++it2;
  }
}


inline void mul_add(const real_t* first1, const real_t* last1, const real_t* first2, real_t* first3) {

  const real_t* it1 = first1;
  const real_t* it2 = first2;
  real_t*       it3 = first3;
  while (it1 != last1) {
    *it3 += (*it1) * (*it2);
    ++it1;
    ++it2;
    ++it3;
  }
}


inline void mul_add(const real_t a, const real_t* first1, const real_t* last1, const real_t* first2, real_t* first3) {

  const real_t* it1 = first1;
  const real_t* it2 = first2;
  real_t*       it3 = first3;
  while (it1 != last1) {
    *it3 += a * (*it1) * (*it2);
    ++it1;
    ++it2;
    ++it3;
  }
}

 

/*
 *  eta: learning rate
 *  [first1, last1): grad
 *  first2, : squared-grad-sum
 *  first3, : parameter
 */
inline void adagrad(const real_t eta, const real_t* first1, const real_t* last1, const real_t* first2, real_t* first3) {

  const real_t* it1 = first1;
  const real_t* it2 = first2;
  real_t*       it3 = first3;
  while (it1 != last1) {
    *it3 -= eta * invsqrt(*it2) * (*it1);
    ++it1;
    ++it2;
    ++it3;
  }
}


inline std::string to_str(const real_t* v, const int size) {

  std::stringstream ss("");
   for (int i = 0; i < size; ++i) {
     if (0 < i) {
       ss << " ";
     }
     ss << v[i];
   }
   return ss.str();
}


inline void load(const char* s, real_t* it) {

  char* endptr;
  do {
    *it = static_cast<real_t>(strtod(s, &endptr));
    ++it;
#ifdef __YSKIP_DEBUG__
    assert(*endptr == '\0' || *endptr == ' ');
#endif
    if (*endptr == '\0') {
      break;
    }else {
      s = endptr + 1;
    }
  }while(1);
}


}
