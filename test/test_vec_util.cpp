/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#include <cassert>
#include "../src/random.h"
#include "../src/vec_util.h"
#include "../src/dense_matrix.h"


using namespace yskip;



void test_mul_add(const int size) {

  Random random(0);
  const real_t a = random.uniform(static_cast<real_t>(-1.0), static_cast<real_t>(1.0));
  real_t* x = NULL;
  real_t* y = NULL;
  real_t* z = NULL;
  real_t* w = NULL;
  posix_memalign((void **)&x, 32, sizeof(real_t)*size);
  posix_memalign((void **)&y, 32, sizeof(real_t)*size);
  posix_memalign((void **)&z, 32, sizeof(real_t)*size);
  posix_memalign((void **)&w, 32, sizeof(real_t)*size);
  for (int i = 0; i < size; ++i) {
    x[i] = random.uniform(-1.0, 1.0);
  }

  //
  for (int i = 0; i < size; ++i) {
    y[i] = a * x[i];
  }
  mul_add(a, x, x + size, z);
  for (int i = 0; i < size; ++i) {
    assert(approx_equal(y[i], z[i]));
  }

  //
  for (int i = 0; i < size; ++i) {
    z[i] = a * x[i] * y[i];
  }
  mul_add(a, x, x + size, y, w);
  for (int i = 0; i < size; ++i) {
    assert(approx_equal(z[i], w[i]));
  }
  
  free(x);
  free(y);
  free(z);
}


int main() {

  test_mul_add(0);
  test_mul_add(1);
  test_mul_add(3);
  test_mul_add(4);
  test_mul_add(8);
  test_mul_add(16);
  test_mul_add(23);

  return 0;
}
