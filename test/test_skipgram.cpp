/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#include <cassert>
#include "../src/util.h"
#include "../src/skipgram.h"

using namespace yskip;


void test_save_load() {

  Random random(0);
  Skipgram::Option option;
  option.max_vocab_size     = 20;
  option.unigram_table_size = 10;
  option.subsampling_threshold = 1.0; // avoids words from being subsampled
  Skipgram sg(option);
  std::vector<std::string> text = tokenize("A B C C D B DE D");
  real_t* grad;
  posix_memalign((void**)&grad, 128, sizeof(real_t)*sg.vec_size());

  //
  sg.update_unigram_table(text, random);
  assert(sg.vocab().size() == 5);
  assert(text.size() == 8);
  sg.train(text, false, grad, random);
  assert(sg.save("tmp", false) == SUCCESS);
  
  //
  Skipgram sg2(option);
  assert(sg2.load("tmp", false) == SUCCESS);
  assert(sg.vocab() == sg2.vocab());
  assert(sg.counts() == sg2.counts());
  assert(approx_equal(sg.alpha(), sg2.alpha()));
  assert(approx_equal(sg.subsampling_threshold(), sg2.subsampling_threshold()));

  assert(sg.save("tmp", true) == SUCCESS);
  assert(sg2.load("tmp", true) == SUCCESS);
  assert(sg.vocab() == sg2.vocab());
  assert(sg.counts() == sg2.counts());
  assert(approx_equal(sg.alpha(), sg2.alpha()));
  assert(approx_equal(sg.subsampling_threshold(), sg2.subsampling_threshold()));
  free(grad);
}


void test_reduce_vocab() {

  Random random(0);
  Skipgram::Option option;
  option.subsampling_threshold = 100.0;
  option.max_vocab_size = 5;
  option.unigram_table_size = 10;
  Skipgram sg(option);

  //
  std::vector<std::string> text = tokenize("A B C D A C D");
  std::vector<std::string> text2 = tokenize("A A A C D C D E");
  
  //
  sg.update_unigram_table(text, random);
  assert(sg.vocab().size() == 4);
  assert(sg.vocab().encode("A") == 0);
  assert(sg.vocab().encode("B") == 1);
  assert(sg.vocab().encode("C") == 2);
  assert(sg.vocab().encode("D") == 3);
  assert(sg.counts().at(0) == 2);
  assert(sg.counts().at(1) == 1);
  assert(sg.counts().at(2) == 2);
  assert(sg.counts().at(3) == 2);
  assert(sg.total_count() == 7);

  //
  sg.update_unigram_table(text2, random);
  assert(sg.vocab().size() == 3);
  assert(sg.vocab().encode("A") == 0);
  assert(sg.vocab().encode("B") == -1);
  assert(sg.vocab().encode("C") == 1);
  assert(sg.vocab().encode("D") == 2);
  assert(sg.vocab().encode("E") == -1);
  assert(sg.counts().at(0) == 4);
  assert(sg.counts().at(1) == 3);
  assert(sg.counts().at(2) == 3);
  assert(sg.total_count() == 10);
}


int main(int argc, const char** argv) {  

  test_reduce_vocab();
  test_save_load();
   
  return SUCCESS;
}
