/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>
#include <numeric> // accumulate
#include "util.h"
#include "random.h"

namespace yskip {


class UnigramTable {
 public:
  UnigramTable();
  explicit UnigramTable(const int max_size);
  ~UnigramTable() {};
  int max_size() const;
  int sample(Random& random) const;
  void initialize(const int max_size);
  void build(const std::vector<count_t>& counts, const real_t alpha, Random& random);
  void update(const int word, const real_t weight, Random& random);
  
 private:
  int              max_size_;
  int              size_;
  real_t           weight_sum_;
  std::vector<int> table_;
  DISALLOW_COPY_AND_ASSIGN(UnigramTable);
};


inline UnigramTable::UnigramTable() {

  max_size_   = 1e8;
  size_       = 0;
  weight_sum_ = 0.0;
}


inline UnigramTable::UnigramTable(const int max_size) {

  max_size_   = max_size;
  size_       = 0;
  weight_sum_ = 0.0;
  table_      = std::vector<int>(max_size_, 0);
}


inline int UnigramTable::sample(Random& random) const {

  assert(0 < size_);
  return table_[random.uniform(0, size_)];
}


inline int UnigramTable::max_size() const {

  return max_size_;
}


inline void UnigramTable::initialize(const int max_size) {

  max_size_   = max_size;
  size_       = 0;
  weight_sum_ = 0.0;
  table_      = std::vector<int>(max_size_, 0);
}


inline void UnigramTable::build(const std::vector<count_t>& counts, const real_t alpha, Random& random) {

  //
  int vocab_size = counts.size();
  real_t z = 0.0;
  for (int w = 0; w < vocab_size; ++w) {
    z += std::pow(static_cast<real_t>(counts[w]), alpha);
  }

  //
  std::vector<int> nums(vocab_size, 0);
  for (int w = 0; w < vocab_size; ++w) {
    nums[w] = random.round(static_cast<double>(max_size_)*std::pow(static_cast<real_t>(counts[w]), alpha)/z);
  }
  int sum = std::accumulate(nums.begin(), nums.end(), 0);
  while (max_size_ < sum) {
    int w = random.uniform(0, vocab_size);
    if (0 < nums[w]) {
      --nums[w];
      --sum;
    }
  }

  //
  weight_sum_ = z;
  size_ = 0;
  for (int w = 0; w < vocab_size; ++w) {
    for (int i = 0; i < nums[w]; ++i) {
      table_[size_] = w;
      ++size_;
    }
  }
}


inline void UnigramTable::update(const int word, const real_t weight, Random& random) {

  assert(0 <= word);
  assert(0.0 <= weight);


  weight_sum_ += weight;
  if (size_ < max_size_) {
    int new_size = std::min<int>(random.round(weight) + size_, max_size_);
    for (int i = size_; i < new_size; ++i) {
      table_[i] = word;
    }
    size_ = new_size;
  }else {
    int n = random.round((weight/weight_sum_)*static_cast<real_t>(max_size_));
    for (int i = 0; i < n; ++i) {
      table_[random.uniform(0, max_size_)] = word;
    }
  }
}


}
