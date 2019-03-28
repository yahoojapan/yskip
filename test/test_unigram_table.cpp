/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#include <cassert>
#include "../src/unigram_table.h"


using namespace yskip;


void test_build(const real_t alpha) {

  Random random(0);
  const int table_size = 1e5;
  const int sample_num = 1e5;
  
  std::vector<count_t> counts(3);
  counts[0] = 2;
  counts[1] = 3;
  counts[2] = 5;

  UnigramTable t(table_size);
  t.build(counts, alpha, random);

  std::vector<count_t> expected_nums(3);
  expected_nums[0] = static_cast<count_t>(static_cast<real_t>(sample_num)*std::pow(2.0, alpha)/(std::pow(2.0, alpha) + std::pow(3.0, alpha) + std::pow(5.0, alpha)));
  expected_nums[1] = static_cast<count_t>(static_cast<real_t>(sample_num)*std::pow(3.0, alpha)/(std::pow(2.0, alpha) + std::pow(3.0, alpha) + std::pow(5.0, alpha)));
  expected_nums[2] = static_cast<count_t>(static_cast<real_t>(sample_num)*std::pow(5.0, alpha)/(std::pow(2.0, alpha) + std::pow(3.0, alpha) + std::pow(5.0, alpha)));

  std::mt19937 mt(0);
  std::vector<count_t> sample_nums(3, 0);
  for (int i = 0; i < sample_num; ++i) {
    sample_nums[t.sample(random)] += 1;
  }
  std::cout << "test_build(alpha=" << alpha << ")";
  std::cout << "\tExpecetdNum=" << expected_nums[0] << "/" << expected_nums[1] << "/" << expected_nums[2];
  std::cout << "\tSampleNum=" << sample_nums[0] << "/" << sample_nums[1] << "/" << sample_nums[2] << std::endl;
}


void test_update(const real_t alpha) {

  Random random(0);
  const int table_size = 1e5;
  const int sample_num = 1e5;
  
  //
  UnigramTable t(table_size);

  //
  const int vocab_size = 3;
  std::vector<count_t> counts(vocab_size, 0);
  for (int i = 0; i < 1000000; ++i) {
    int w = random.uniform(0, 10);
    if (w < 2) {
      w = 0;
    }else if (w < 5) {
      w = 1;
    }else {
      w = 2;
    }
    counts[w] += 1;
    t.update(w, std::pow(static_cast<real_t>(counts[w]), alpha) - std::pow(static_cast<real_t>(counts[w]-1), alpha), random);
  }
  
  //
  std::vector<count_t> expected_nums(vocab_size);
  real_t z = 0.0;
  for (int i = 0; i < vocab_size; ++i) {
    z += std::pow(static_cast<real_t>(counts[i]), alpha);
  }
  for (int i = 0; i < vocab_size; ++i) {
    expected_nums[i] = static_cast<count_t>(static_cast<real_t>(sample_num)*std::pow(static_cast<real_t>(counts[i]), alpha)/z);
  }
  
  //
  std::vector<count_t> sample_nums(vocab_size, 0);
  for (int i = 0; i < sample_num; ++i) {
    sample_nums[t.sample(random)] += 1;
  }

  //
  std::cout << "test_update(alpha=" << alpha << ")";
  std::cout << "\tExpecetdNum";
  for (int i = 0; i < vocab_size; ++i) {
    std::cout << "/" << expected_nums[i];
  }
  std::cout << "\tSampleNum";
  for (int i = 0; i < vocab_size; ++i) {
    std::cout << "/" << sample_nums[i];
  }
  std::cout << std::endl;
}



int main() {

  test_build(1.0);
  test_build(0.75);
  test_update(1.0);
  test_update(0.75);
  
  return 0;
}
