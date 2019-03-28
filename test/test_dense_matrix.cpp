/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#include <cassert>
#include "../src/dense_matrix.h"


using namespace yskip;


void test_reduce() {

  DenseMatrix m(5, 2);
  m[0][0] = 1.0; m[0][1] = 2.0;
  m[1][0] = 3.0; m[1][1] = 4.0;
  m[2][0] = 5.0; m[2][1] = 6.0;
  m[3][0] = 7.0; m[3][1] = 8.0;
  m[4][0] = 9.0; m[4][1] = 10.0;
  assert(m.row_num() == 5);
  assert(m.col_num() == 2);
  
  std::unordered_set<int> reserved_rows;
  reserved_rows.insert(2);
  reserved_rows.insert(4);
  m.reduce(reserved_rows);
  
  assert(m.row_num() == 5);
  assert(m.col_num() == 2);
  assert(m[0][0] == 5.0);
  assert(m[0][1] == 6.0);
  assert(m[1][0] == 9.0);
  assert(m[1][1] == 10.0);
}


int main() {

  DenseMatrix m(5, 2);

  assert(m.row_num() == 5);
  assert(m.col_num() == 2);
  assert(m[0][0] == 0.0);
  assert(m[0][1] == 0.0);
  assert(m[1][0] == 0.0);
  assert(m[1][1] == 0.0);
  assert(m[2][0] == 0.0);
  assert(m[2][1] == 0.0);
  m[0][0] = 1.0;
  m[0][1] = 1.0;
  m[1][0] = 1.0;
  m[1][1] = 1.0;
  m[2][0] = 1.0;
  m[2][1] = 1.0;

  assert(m[0][0] == 1.0);
  assert(m[0][1] == 1.0);
  assert(m[1][0] == 1.0);
  assert(m[1][1] == 1.0);
  assert(m[2][0] == 1.0);
  assert(m[2][1] == 1.0);
  assert(m[3][0] == 0.0);
  assert(m[3][1] == 0.0);
  assert(m[4][0] == 0.0);
  assert(m[4][1] == 0.0);
  assert(m.row_num() == 5);  

  test_reduce();
  
  return SUCCESS;
}
