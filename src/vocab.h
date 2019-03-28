/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#pragma once
#include <stdio.h>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <string>
#include "util.h"


namespace yskip {

    
class Vocab {
 public:
  struct Item {
    std::string word;
    int         index;
    Item();
  };
  struct SortItem {
    bool operator()(const Item &x, const Item &y) const {
      return x.index < y.index;
    }
  };
  Vocab();
  Vocab(const int table_size);
  Vocab(const Vocab &other);
  ~Vocab() {};
  Vocab& operator=(const Vocab &other);
  void initialize(const int table_size);
  void clear();
  void reduce(const std::unordered_set<int> &reduced_vocab);
  int add(const std::string& word);
  int add(const char* word);
  int add(const char* begin, const char* end);
  int encode(const std::string& word) const;
  int encode(const char* word) const;
  int encode(const char* begin, const char* end) const;
  std::vector<std::string> all() const;
  uint32_t size() const;
  uint32_t table_size() const;
  const std::vector<Item>& table() const;
  int save(FILE* os) const;
  int load(FILE* os);
  
 private:  
  uint32_t                    size_;
  uint32_t                    table_size_;
  std::vector<Item>           table_;
  std::vector<Item>::iterator table_begin_;
  std::vector<Item>::iterator table_end_;
};


inline Vocab::Item::Item() {

  word  = "";
  index = -1;
}


inline Vocab::Vocab() {

  size_        = 0;
  table_size_  = 1e6;
  table_       = std::vector<Item>(table_size_, Item());
  table_begin_ = table_.begin();
  table_end_   = table_.end();
}


inline Vocab::Vocab(const int table_size) {
  
  size_        = 0;
  table_size_  = table_size;
  table_       = std::vector<Item>(table_size_, Item());
  table_begin_ = table_.begin();
  table_end_   = table_.end();
}


inline Vocab::Vocab(const Vocab &other) {

  size_        = other.size();
  table_size_  = other.table_size();
  table_       = other.table();
  table_begin_ = table_.begin();
  table_end_   = table_.end();
}
 

inline Vocab& Vocab::operator=(const Vocab &other) {

  if (this != &other) {
    size_        = other.size();
    table_size_  = other.table_size();
    table_       = other.table();
    table_begin_ = table_.begin();
    table_end_   = table_.end();
  }
  return *this;
}


inline void Vocab::initialize(const int table_size) {

  table_size_ = table_size;
  table_.resize(table_size_);
  table_begin_ = table_.begin();
  table_end_   = table_.end();
  clear();
}


inline void Vocab::clear() {

  size_ = 0;
  std::fill(table_.begin(), table_.end(), Item());
}


inline void Vocab::reduce(const std::unordered_set<int> &reduced_vocab) {

  //
  std::vector<std::string> words = all();
  
  //
  clear();
  for (int i = 0; i < words.size(); ++i) {
    if (reduced_vocab.find(i) != reduced_vocab.end()) {
      add(words[i].c_str());
    }
  }
}


inline int Vocab::add(const std::string& word) {

  return this->add(word.c_str());
}


inline int Vocab::add(const char* word) {

  return add(word, word + strlen(word));
}
 

inline int Vocab::add(const char* begin, const char* end) {
  
  std::vector<Item>::iterator it = table_begin_ + fnv1a(begin, end)%table_size_;
  do {
    if (mystrcmp(begin, end, it->word)) {
      return it->index;
    }else if (it->index == -1) {
      it->word.assign(begin, end);
      it->index = size_;
      ++size_;
      return it->index;
    }else {
      ++it;
      if (table_end_ == it) {
	it = table_begin_;
      }
    }
  }while(1);
}


inline int Vocab::encode(const std::string& word) const {

  return this->encode(word.c_str());
}
 

inline int Vocab::encode(const char* s) const {

  return this->encode(s, s + strlen(s));
}
 

inline int Vocab::encode(const char* begin, const char* end) const {

  std::vector<Item>::const_iterator it = table_begin_ + fnv1a(begin, end)%table_size_;
  do {
    if (mystrcmp(begin, end, it->word)) {
      return it->index;
    }else if (it->index == -1) {
      return -1;
    }else {
      ++it;
      if (table_end_ == it) {
	it = table_begin_;
      }
    }
  }while (1);
}


inline uint32_t Vocab::size() const {

  return size_;
}


inline uint32_t Vocab::table_size() const {

  return table_size_;
}
 

inline const std::vector<Vocab::Item>& Vocab::table() const {

  return table_;
}


// return all words in the ascending order of the indices
inline std::vector<std::string> Vocab::all() const {

  //
  std::vector<Item> tmp;
  for (int i = 0; i < table_.size(); ++i) {
    if (table_[i].index != -1) {
      tmp.push_back(table_[i]);
    }
  }
  std::sort(tmp.begin(), tmp.end(), SortItem());

  //
  std::vector<std::string> words;
  for (int i = 0; i < tmp.size(); ++i) {
    words.push_back(tmp[i].word);
  }
  return words;
}
 

inline int Vocab::save(FILE* os) const {

  //
  if (fwrite(&table_size_, sizeof(table_size_), 1, os) != 1) {
    return FAILURE;
  }

  std::vector<std::string> words = all();
  std::string buff;
  for (int i = 0; i < words.size(); ++i) {
    buff.append(words[i]);
    if (i != words.size() - 1) {
      buff.append(1, ' ');
    }
  }
  size_t buff_size = buff.size() + 1; // include null character
  if (fwrite(&buff_size, sizeof(buff_size), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(buff.data(), sizeof(char), buff_size, os) != buff_size) {
    return FAILURE;
  }
  
  return SUCCESS;
}


inline int Vocab::load(FILE* is) {

  // create empty hash table
  int table_size;
  if (fread(&table_size, sizeof(table_size), 1, is) != 1) {
    return FAILURE;
  }
  initialize(table_size);

  // add each word to the hash table
  size_t buff_size;
  if (fread(&buff_size, sizeof(buff_size), 1, is) != 1) {
    return FAILURE;
  }
  char* buff = new char[buff_size];
  if (fread(buff, sizeof(char), buff_size, is) != buff_size) {
    return FAILURE;
  }

  //
  std::vector<std::string> tokens = tokenize(buff);
  for (int i = 0; i < tokens.size(); ++i) {
    add(tokens[i]);
  }
  delete[] buff;
  return SUCCESS;
}


inline bool operator==(const Vocab::Item &item1, const Vocab::Item &item2) {

  return (item1.word == item2.word && item1.index == item2.index);
}
 

inline bool operator==(const Vocab &vocab1, const Vocab &vocab2) {

  return (vocab1.size() == vocab2.size() && vocab1.table_size() == vocab2.table_size() && vocab1.table() == vocab2.table());
}


}
