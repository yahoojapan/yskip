/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#include <cassert>
#include "../src/util.h"


using namespace yskip;


int test_tokenize() {

  char text[] = "A BC DEF  G HI";
  std::vector<std::string> tokens = tokenize(text);

  assert(tokens.size() == 5);
  assert(tokens[0] == "A");
  assert(tokens[1] == "BC");
  assert(tokens[2] == "DEF");
  assert(tokens[3] == "G");
  assert(tokens[4] == "HI");
}


// int test_tokenize2() {

//   //
//   char text[] = " A BC DEF  G HI  ";
//   const char* s = text;
//   const char* first;
//   const char* last;

//   //
//   assert(strcmp(s, " A BC DEF  G HI  ") == 0);
//   assert(tokenize(s, first, last) == true);
//   assert(std::string(first, last) == "A");

//   assert(strcmp(s, " BC DEF  G HI  ") == 0);
//   assert(tokenize(s, first, last) == true);
//   assert(std::string(first, last) == "BC");

//   assert(strcmp(s, " DEF  G HI  ") == 0);
//   assert(tokenize(s, first, last) == true);
//   assert(std::string(first, last) == "DEF");

//   assert(strcmp(s, "  G HI  ") == 0);
//   assert(tokenize(s, first, last) == true);
//   assert(std::string(first, last) == "G");

//   assert(strcmp(s, " HI  ") == 0);
//   assert(tokenize(s, first, last) == true);
//   assert(std::string(first, last) == "HI");

//   assert(strcmp(s, "  ") == 0);
//   assert(tokenize(s, first, last) == false);
//   assert(first == NULL);
//   assert(last == NULL);
  
//   return SUCCESS;
// }



int main() {

  test_tokenize();
  return SUCCESS;
}
