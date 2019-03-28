/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#include <cassert>
#include "../src/fast_sigmoid.h"


using namespace yskip;


void test(const real_t x) {

  std::cout << "x=" << x << "\t" << "approx=" << sigmoid(x, false) << "\t" << "exact=" << sigmoid(x, true) << std::endl;
}


int main() {

  test(15.0);
  test(10.0);
  test(6.0);
  test(3.0);
  test(-0.5);
  test(-6.0);
  test(-9.0);
  return 0;
}
