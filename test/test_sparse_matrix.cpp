#include <sys/time.h>
#include <cassert>
#include "../src/random.h"
#include "../src/sparse_matrix.h"


using namespace yskip;


void test_substitution_operator() {

  //
  std::mt19937 mt;
  SparseMatrix m1(9, 8);
  SparseMatrix m2(14, 15);
  for (int row = 0; row < m1.row_num(); ++row) {
    if (row%2 == 0) continue;
    for (int col = 0; col < m1.col_num(); ++col) {
      m1[row][col] = random(-1.0, 1.0, mt);
    }
  }
  for (int row = 0; row < m2.row_num(); ++row) {
    if (row%3 == 0) continue;
    for (int col = 0; col < m2.col_num(); ++col) {
      m2[row][col] = random(-1.0, 1.0, mt);
    }
  }

  //
  m1 = m2;
  assert(m1.row_num() == m2.row_num());
  assert(m1.col_num() == m2.col_num());
  for (int row = 0; row < m2.row_num(); ++row) {
    assert(m1[0].size() == m2.col_num());
    for (int col = 0; col < m2.col_num(); ++col) {
      if (row%3 == 0) {
	assert(m1[row][col] == 0.0);
      }else {
	assert(m1[row][col] == m2[row][col]);
      }
    }
  }
}


int main() {

  SparseMatrix m(5, 3);
  
  DenseVector v(3);
  v[0] = 1.0; v[1] = 2.0; v[2] = 1.0;
  m[2].plus(2.0, v);
  assert(m.row_num() == 5);
  assert(m.col_num() == 3);
  assert(m[0][0] == 0.0);
  assert(m[0][1] == 0.0);
  assert(m[0][2] == 0.0);
  assert(m[1][0] == 0.0);
  assert(m[1][1] == 0.0);
  assert(m[1][2] == 0.0);
  assert(m[2][0] == 2.0);
  assert(m[2][1] == 4.0);
  assert(m[2][2] == 2.0);

  m.clear();
  assert(m.row_num() == 5);
  assert(m.col_num() == 3);

  test_substitution_operator();
  
  return SUCCESS;
}
