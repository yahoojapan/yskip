/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#include <cassert>
#include "../src/vocab.h"


using namespace yskip;


// void test_add() {

//   Vocab vocab;
//   char text[] = "A BC D A";
//   const char* s = text;
//   int index;
  
//   //
//   assert(vocab.add(s, index) == true);
//   assert(index == 0);
//   assert(vocab.size() == 1);
//   assert(strcmp(s, " BC D A") == 0);

//   //
//   assert(vocab.add(s, index) == true);
//   assert(index == 1);
//   assert(vocab.size() == 2);
//   assert(strcmp(s, " D A") == 0);

//   //
//   assert(vocab.add(s, index) == true);
//   assert(index == 2);
//   assert(vocab.size() == 3);
//   assert(strcmp(s, " A") == 0);

//   //
//   assert(vocab.add(s, index) == true);
//   assert(vocab.size() == 3);
//   assert(index == 0);
//   assert(strcmp(s, "") == 0);

//   //
//   assert(vocab.add(s, index) == false);
// }


// void test_encode() {

//   Vocab vocab(100);
//   vocab.add("A");
//   vocab.add("BB");
//   vocab.add("CCC");
//   vocab.add("DD");
//   vocab.add("E");

//   int index;
//   char text[] = "A BB A  CCC EE DD ";
//   const char* s = text;

//   assert(vocab.encode(s, index) == true);
//   assert(index  == 0);
//   assert(strcmp(s, " BB A  CCC EE DD ") == 0);
  
//   assert(vocab.encode(s, index) == true);
//   assert(index  == 1);
//   assert(strcmp(s, " A  CCC EE DD ") == 0);
  
//   assert(vocab.encode(s, index) == true);
//   assert(index  == 0);
//   assert(strcmp(s, "  CCC EE DD ") == 0);
  
//   assert(vocab.encode(s, index) == true);
//   assert(index  == 2);
//   assert(strcmp(s, " EE DD ") == 0);
  
//   assert(vocab.encode(s, index) == true);
//   assert(index  == -1);
//   assert(strcmp(s, " DD ") == 0);
  
//   assert(vocab.encode(s, index) == true);
//   assert(index  == 3);
//   assert(strcmp(s, " ") == 0);

//   assert(vocab.encode(s, index) == false);
// }


void test_reduce() {

  Vocab vocab(100);
  vocab.add("A");
  vocab.add("B");
  vocab.add("C");
  vocab.add("D");
  vocab.add("E");

  std::unordered_set<int> reduced_vocab;
  reduced_vocab.insert(0);
  reduced_vocab.insert(2);
  reduced_vocab.insert(3);
  vocab.reduce(reduced_vocab);

  assert(vocab.size() == 3);
  assert(vocab.encode("A") == 0);
  assert(vocab.encode("B") == -1);
  assert(vocab.encode("C") == 1);
  assert(vocab.encode("D") == 2);
  assert(vocab.encode("E") == -1);
}


int main() {

  {
    Vocab vocab(100);
    assert(vocab.add("A")    == 0);
    assert(vocab.size()      == 1);
    assert(vocab.add("B")    == 1);
    assert(vocab.size()      == 2);
    assert(vocab.add("A")    == 0);
    assert(vocab.size()      == 2);
    assert(vocab.encode("C") == -1);
    assert(vocab.size()      == 2);
    assert(vocab.add("C")    == 2);
    assert(vocab.size()      == 3);

    std::vector<std::string> words = vocab.all();
    assert(vocab.size() == words.size());
    for (int i = 0; i < words.size(); ++i) {
      assert(vocab.encode(words[i].c_str()) == i);
    }
    
    FILE* os = fopen("tmp", "wb");
    assert(vocab.save(os) == SUCCESS);
    fclose(os);

    Vocab vocab2;
    FILE* is = fopen("tmp", "rb");
    assert(vocab2.load(is) == SUCCESS);
    fclose(is);

    assert(vocab.size() == vocab2.size());
    assert(vocab.table_size() == vocab2.table_size());
    assert(vocab.encode("A") == vocab2.encode("A"));
    assert(vocab.encode("B") == vocab2.encode("B"));
    assert(vocab.encode("C") == vocab2.encode("C"));
    assert(vocab.encode("D") == vocab2.encode("D")); 
  }

  //
  //test_add();
  //test_encode();
  test_reduce();
  
  return SUCCESS;
}
