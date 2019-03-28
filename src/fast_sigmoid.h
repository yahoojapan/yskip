/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath> // std::exp
#include "util.h"


namespace yskip {


class FastSigmoid {
 public:
  FastSigmoid();
  ~FastSigmoid() {};
  real_t operator()(const real_t x) const;

 private:
  real_t              max_x_;
  real_t              bin_size_;
  int                 table_size_;
  std::vector<real_t> table_;
  DISALLOW_COPY_AND_ASSIGN(FastSigmoid);
};


inline FastSigmoid::FastSigmoid() {

  max_x_      = 6.0;
  table_size_ = 1000;
  table_.resize(table_size_);
  bin_size_   = 2.0 * max_x_ / static_cast<real_t>(table_size_);
  for (int i = 0; i < table_size_; ++i) {
    const real_t x = bin_size_ * static_cast<real_t>(i) - max_x_;
    table_[i] = std::exp(x)/(std::exp(x) + 1.0);
  }
}


inline real_t FastSigmoid::operator()(const real_t x) const {

  if (max_x_ <= x) {
    return 1.0;
  }else if (x <= -max_x_) {
    return 0.0;
  }else {
    return table_[std::max<int>(0, std::min<int>(table_size_, static_cast<int>((x + max_x_)/bin_size_)))];
  }
}


inline real_t sigmoid(const real_t x, const bool exact=false) {

  if (exact) {
    return 1.0/(1.0 + std::exp(-x));
  }else {
    static FastSigmoid sigma;
    return sigma(x);
  }
}
 


}
