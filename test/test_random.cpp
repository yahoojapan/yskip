/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#include <cassert>
#include "../src/random.h"


using namespace yskip;


int main(int argc, char **argv) {

  Random random(0);
  int rounddown_num = 0;
  int roundup_num = 0;
  for (int i = 0; i < 10000; ++i) {
    int r = random.round(1.7);
    assert(r == 1 || r == 2);
    if (r == 1) {
      ++rounddown_num;
    }else {
      ++roundup_num;
    }
  }
  std::fprintf(stderr, "round(1.7): #rounddown=%d, #roundup=%d\n", rounddown_num, roundup_num);
  
  return SUCCESS;
}
