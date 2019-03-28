/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#pragma once
#include <stdio.h>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <numeric> // inner_product
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <string>
#include "util.h"
#include "vec_util.h"
#include "random.h"
#include "vocab.h"
#include "dense_matrix.h"
#include "fast_sigmoid.h"
#include "unigram_table.h"


namespace yskip {

  
struct Parameter {
  DenseMatrix input;
  DenseMatrix output;
  Parameter() {};
  Parameter(const int max_vocab_size, const int vec_size, const real_t val=0.0) : input(max_vocab_size, vec_size, val), output(max_vocab_size, vec_size, val) {};
};


class Skipgram {
 public:
  struct Option {
    int    vec_size;
    int    window_size;
    int    neg_sample_num;
    real_t alpha;
    real_t subsampling_threshold;
    real_t eta;
    int    unigram_table_size;
    int    max_vocab_size;
    Option();
  };
  Skipgram();
  Skipgram(const Option& option);
  Skipgram(const Option& option, Random& random);
  ~Skipgram() {};
  
  // skip-gram model options
  int vec_size() const;
  int window_size() const;
  int neg_sample_num() const;
  int max_vocab_size() const;
  real_t alpha() const;
  real_t subsampling_threshold() const;
  real_t eta() const;

  // vocabulary
  const Vocab& vocab() const;

  // embedding
  const Parameter& vec() const;

  // word statistics
  count_t total_count() const;
  const std::vector<count_t>& counts() const;

  // 
  void initialize(const Option& option, Random& random);
  void update_unigram_table(const std::vector<std::string>& text, Random& random);
  void update_unigram_table(const std::string& word, Random& random);
  void train(const std::vector<std::string>& text, bool incremental, real_t* grad, Random& random);
  void sgd(const int target, const int context, const std::vector<int>& neg_samples, real_t* grad);
  void rebuild_unigram_table(Random& random);

  // outdated
  //void update_vocab(const char* raw_text, Random& random);
  //void encode_text(const char* raw_text, std::vector<int>& text, Random& random) const;  
  //void sgd(const std::vector<int>& text, real_t* grad, Random& random);
  //void sgd(const std::vector<int>& text, const int target, real_t* grad, Random& random);
  
  // load model file
  int load(const char* filename, const bool binary_mode);
  int load_bin(FILE* is);
  int load_bin(const char* filename);
  int load_text(const char* filename);

  // save model file
  int save(const char* filename, const bool binary_mode) const;
  int save_bin(const char* filename) const;
  int save_bin(FILE* os) const;
  int save_text(const char* filename) const;
  
 private:
  // options
  int          vec_size_;
  int          window_size_;
  int          neg_sample_num_;
  real_t       alpha_;
  real_t       subsampling_threshold_;
  real_t       eta_;
  int          unigram_table_size_;
  int          max_vocab_size_;

  // embeddings
  Vocab     vocab_;
  Parameter vec_;
  Parameter squared_grad_;
  
  // word count
  count_t              total_count_;
  std::vector<count_t> counts_;

  // unigram table
  UnigramTable unigram_table_;

  void reduce_vocab(Random& random);
  DISALLOW_COPY_AND_ASSIGN(Skipgram);
};


inline Skipgram::Option::Option() {

  vec_size              = 100;
  alpha                 = 0.75;
  window_size           = 5;
  neg_sample_num        = 5;
  subsampling_threshold = 1e-3;
  eta                   = 0.1;
  unigram_table_size    = 1e8;
  max_vocab_size        = 1e6;
}


inline Skipgram::Skipgram() {

  Option option;
  Random random;
  this->initialize(option, random);
}


inline Skipgram::Skipgram(const Option& option) {

  Random random;
  this->initialize(option, random);
}


inline Skipgram::Skipgram(const Option& option, Random& random) {

   this->initialize(option, random);
}

 
inline void Skipgram::initialize(const Option& option, Random& random) {

  // training options
  vec_size_              = option.vec_size;
  window_size_           = option.window_size;
  neg_sample_num_        = option.neg_sample_num;
  subsampling_threshold_ = option.subsampling_threshold;
  eta_                   = option.eta;
  alpha_                 = option.alpha;
  max_vocab_size_        = option.max_vocab_size;
  unigram_table_size_    = option.unigram_table_size;  
  
  // vocabulary
  vocab_ = Vocab(max_vocab_size_*2);

  // word embedding
  vec_ = Parameter(max_vocab_size_, vec_size_);
  const real_t min = static_cast<real_t>(-0.5)/static_cast<real_t>(vec_size_);
  const real_t max = static_cast<real_t>(0.5)/static_cast<real_t>(vec_size_);
  for (int w = 0; w < max_vocab_size_; ++w) {
    for (int i = 0; i < vec_size_; ++i) {
      vec_.input[w][i]  = random.uniform(min, max);
      vec_.output[w][i] = random.uniform(min, max);
    }
  }

  // accumulated gradient
  squared_grad_ = Parameter(max_vocab_size_, vec_size_, 1.0e-8);
  
  // word counts
  total_count_ = 0;
  counts_ = std::vector<count_t>(max_vocab_size_, 0);

  // unigram table
  unigram_table_.initialize(unigram_table_size_);
}



inline void Skipgram::train(const std::vector<std::string>& text, bool incremental, real_t* grad, Random& random) {

  int n = text.size();
  std::vector<int> neg_samples(neg_sample_num_);
  for (int target = 0; target < n; ++target) {
    //  incrementally update unigram table
    if (incremental) {
      update_unigram_table(text[target], random);
    }

    //
    int target_index = vocab_.encode(text[target]);
    if (target_index == -1) continue; // ignore unknown word
    
    //
    int random_window_size = random.uniform(1, window_size_ + 1);
    for (int offset = -random_window_size; offset < random_window_size; ++offset) {
      if (offset == 0 || target + offset < 0) continue;
      if (target + offset == n) break;

      //
      int context_index = vocab_.encode(text[target + offset]);
      if (context_index == -1) continue;  // ignore unknown word

      // perform subsampling
      if (0 < counts_[context_index] && sqrt(subsampling_threshold_*static_cast<real_t>(total_count_)/static_cast<real_t>(counts_[context_index])) < random.uniform(0.0, 1.0)) continue;

      // collecting negative samples
      for (int k = 0; k < neg_sample_num_; ++k) {
	neg_samples[k] = unigram_table_.sample(random);
	if (neg_samples[k] < 0 || max_vocab_size_ <= neg_samples[k]) {
	  fprintf(stderr, "%d\n", neg_samples[k]);
	}
      }

      // perform SGD
      sgd(target_index, context_index, neg_samples, grad);
    }
  }
}


/* inline void Skipgram::sgd(const std::vector<int>& text, real_t* grad, Random& random) { */
  
/*   int n = text.size(); */
/*   for (int i = 0; i < n; ++i) { */
/*       sgd(text, i, grad, random); */
/*   } */
/* } */


/* inline void Skipgram::sgd(const std::vector<int>& text, const int target, real_t* grad, Random& random) { */

/*   int n = text.size(); */
/*   int random_window_size = random.uniform(1, window_size_ + 1); */
/*   std::vector<int> neg_samples(neg_sample_num_); */
/*   for (int offset = -random_window_size; offset < random_window_size; ++offset) { */
/*     if (offset == 0 || target + offset < 0) continue; */
/*     if (target + offset == n) break; */
/*     for (int k = 0; k < neg_sample_num_; ++k) { */
/*       neg_samples[k] = unigram_table_.draw(random); */
/*     } */
/*     sgd(text[target], text[target + offset], neg_samples, grad); */
/*   } */
/* } */


//
// t: target word index
// c: context word index
// neg_samples: negative context word indices
// grad: buffer to accumulate gradient
//
inline void Skipgram::sgd(const int t, const int c, const std::vector<int>& neg_samples, real_t* grad) {

  // positive example
  real_t sigma = sigmoid(std::inner_product(vec_.input[t], vec_.input[t] + vec_size_, vec_.output[c], 0.0));
  std::fill(grad, grad + vec_size_, 0.0);
  mul_add(sigma - 1.0, vec_.output[c], vec_.output[c] + vec_size_, grad);
  
  // SGD for vec_.output[c]
  mul_add((sigma - 1.0)*(sigma - 1.0), vec_.input[t], vec_.input[t] + vec_size_, vec_.input[t], squared_grad_.output[c]);
  adagrad(eta_*(sigma - 1.0), vec_.input[t], vec_.input[t] + vec_size_, squared_grad_.output[c], vec_.output[c]);
  
  // negative examples
  for (int k = 0; k < neg_sample_num_; ++k) {
    int v = neg_samples[k];
    real_t sigma = sigmoid(std::inner_product(vec_.input[t], vec_.input[t] + vec_size_, vec_.output[v], 0.0));
    mul_add(sigma, vec_.output[v], vec_.output[v] + vec_size_, grad);

    // SGD for vec_.output[v]
    mul_add(sigma*sigma, vec_.input[t], vec_.input[t] + vec_size_, vec_.input[t], squared_grad_.output[v]);  
    adagrad(eta_*sigma, vec_.input[t], vec_.input[t] + vec_size_, squared_grad_.output[v], vec_.output[v]);
  }
  
  mul_add(grad, grad + vec_size_, grad, squared_grad_.input[t]);
  adagrad(eta_, grad, grad + vec_size_, squared_grad_.input[t], vec_.input[t]);
}


/* inline void Skipgram::encode_text(const char* raw_text, std::vector<int>& text, Random& random) const { */

/*   text.clear(); */
/*   int word_index; */
/*   while (vocab_.encode(raw_text, word_index)) { */
/*     if (word_index == -1) continue; */
/*     if (0 < counts_[word_index] && sqrt(subsampling_threshold_*static_cast<real_t>(total_count_)/static_cast<real_t>(counts_[word_index])) < random.uniform(0.0, 1.0)) continue; */
/*     text.push_back(word_index); */
/*   } */
/* } */


/* inline void Skipgram::update_vocab(const char* raw_text, Random& random) { */

/*   int word_index; */
/*   while (vocab_.add(raw_text, word_index)) { */
/*       total_count_ += 1; */
/*       counts_[word_index] += 1; */
/*       unigram_table_.update(word_index, std::pow(static_cast<real_t>(counts_[word_index]), alpha_) - std::pow(static_cast<real_t>(counts_[word_index]-1), alpha_), random); */
    
/*       if (max_vocab_size_ == vocab_.size()) { */
/* 	reduce_vocab(random); */
/* 	rebuild_unigram_table(random); */
/*       } */
/*   } */
/* } */


inline void Skipgram::update_unigram_table(const std::vector<std::string>& text, Random& random) {

  for (std::vector<std::string>::const_iterator it = text.begin(); it != text.end(); ++it) {
    update_unigram_table(*it, random);
  }
}


inline void Skipgram::update_unigram_table(const std::string& word, Random& random) {
  // update vocabulary
  int word_index = vocab_.add(word);
  total_count_ += 1;
  counts_[word_index] += 1;

  // update unigram table
  unigram_table_.update(word_index, std::pow(static_cast<real_t>(counts_[word_index]), alpha_) - std::pow(static_cast<real_t>(counts_[word_index]-1), alpha_), random);

  // reduce vocabulary if its size reaches the maximum value
  if (max_vocab_size_ == vocab_.size()) {
    reduce_vocab(random);
    rebuild_unigram_table(random);
  }
}


// Use Misra-Gries algorithm to limit the vocabulary size
inline void Skipgram::reduce_vocab(Random& random) {

  //
  int reduced_vocab_size = 0;
  std::unordered_set<int> reserved_word_indices;
  for (int i = 0; i < vocab_.size(); ++i) {
    if (1 < counts_[i]) {
      ++reduced_vocab_size;
      reserved_word_indices.insert(i);
    }
  }

  //
  total_count_ = 0;
  int new_word_index = 0;
  for (int i = 0; i < vocab_.size(); ++i) {
    if (1 < counts_[i]) {
      counts_[new_word_index] = counts_[i] - 1;
      total_count_ += counts_[new_word_index];
      ++new_word_index;
    }
  }
  for (int i = reduced_vocab_size; i < max_vocab_size_; ++i) {
    counts_[i] = 0;
  }

  //
  vocab_.reduce(reserved_word_indices);
  vec_.input.reduce(reserved_word_indices);
  vec_.output.reduce(reserved_word_indices);
  squared_grad_.input.reduce(reserved_word_indices);
  squared_grad_.output.reduce(reserved_word_indices);
  for (int w = reduced_vocab_size; w < max_vocab_size_; ++w) {
    for (int i = 0; i < vec_size_; ++i) {
      vec_.input[w][i]  = random.uniform(static_cast<real_t>(-0.5)/static_cast<real_t>(vec_size_), static_cast<real_t>(0.5)/static_cast<real_t>(vec_size_));
      vec_.output[w][i] = random.uniform(static_cast<real_t>(-0.5)/static_cast<real_t>(vec_size_), static_cast<real_t>(0.5)/static_cast<real_t>(vec_size_));
      squared_grad_.input[w][i]  = 1.0e-8;
      squared_grad_.output[w][i] = 1.0e-8;
    }
  }
}


inline void Skipgram::rebuild_unigram_table(Random& random) {

  unigram_table_.build(counts_, alpha_, random);
}


inline int Skipgram::load(const char* filename, const bool binary_mode) {

  if (binary_mode) {
    return load_bin(filename);
  }else {
    return load_text(filename);
  }
}


inline int Skipgram::save(const char* filename, const bool binary_mode) const {

  if (binary_mode) {
    return save_bin(filename);
  }else {
    return save_text(filename);
  }
}


inline int Skipgram::load_text(const char* filename) {

  //
  FILE* is = NULL;
  if (strcmp(filename, "-") == 0) {
    is = stdin;
  }else {
    is = fopen(filename, "r");
  }
  setvbuf(is, NULL, _IOFBF, BUFF_SIZE);
  if (is == NULL) {
    std::fprintf(stderr, "failed to open %s\n", filename);
    return FAILURE;
  }
  
  // read header
  char line[BUFF_SIZE];
  if (fgets(line, BUFF_SIZE, is) == NULL) {
    std::fprintf(stderr, "%s is empty\n", filename);
    return FAILURE;
  }
  line[strlen(line)-1] = '\0';
  int vocab_size;
  if (sscanf(line, "%d\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%d\n", &vocab_size, &max_vocab_size_, &vec_size_, &window_size_, &neg_sample_num_, &alpha_, &subsampling_threshold_, &eta_, &unigram_table_size_) != 9) {
    std::fprintf(stderr, HERE "invalid format (%s): %s\n", filename, line);
    return FAILURE;
  }
    
  //
  vocab_.initialize(max_vocab_size_*2);
  vec_ = Parameter(max_vocab_size_, vec_size_, 0.0);
  squared_grad_ = Parameter(max_vocab_size_, vec_size_, 1.0e-8);
  total_count_ = 0;
  counts_ = std::vector<count_t>(max_vocab_size_, 0);

  //
  char word[BUFF_SIZE], s1[BUFF_SIZE], s2[BUFF_SIZE], s3[BUFF_SIZE], s4[BUFF_SIZE];
  uint64_t count;
  while (fgets(line, BUFF_SIZE, is) != NULL) {
    line[strlen(line)-1] = '\0';
    if (sscanf(line, "%s %lld %[^\t] %[^\t] %[^\t] %[^\t]", word, &count, s1, s2, s3, s4) != 6) {
      std::fprintf(stderr, HERE "invalid format (%s): %s\n", filename, line);
      return FAILURE;
    }
    vocab_.add(word);
    total_count_ += count;
    int index = vocab_.size() - 1;
    counts_[index] = count;
    yskip::load(s1, vec_.input[index]);
    yskip::load(s2, vec_.output[index]);
    yskip::load(s3, squared_grad_.input[index]);
    yskip::load(s4, squared_grad_.output[index]);
  }
  if (strcmp(filename, "-") != 0) {
    fclose(is);
  }
  
  //
  unigram_table_.initialize(unigram_table_size_);
  Random random(0);
  this->rebuild_unigram_table(random);

  return SUCCESS;
}


//
inline int Skipgram::save_text(const char* filename) const {

  // file open
  FILE* os = NULL;
  if (strcmp(filename, "-") == 0) {
    os = stdout;
  }else {
    os = fopen(filename, "w");
  }
  if (os == NULL) {
    std::fprintf(stderr, "cannot open %s\n", filename);
    return FAILURE;
  }
  setvbuf(os, NULL, _IOFBF, BUFF_SIZE);
  
  // header
  std::fprintf(os,
	       "%d\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%d\n",
	       vocab_.size(),
	       max_vocab_size_,
	       vec_size_,
	       window_size_,
	       neg_sample_num_,
	       alpha_,
	       subsampling_threshold_,
	       eta_,
	       unigram_table_.max_size());
  
  // word vectors
  std::vector<std::string> words = vocab_.all();
  for (int index = 0; index < words.size(); ++index) {
    std::fprintf(os, "%s", words[index].c_str());
    std::fprintf(os, "\t%lld", counts_[index]);
    for (int i = 0; i < vec_size_; ++i) {
      if (i == 0) {
    	std::fprintf(os, "\t%lf", vec_.input[index][i]);
      }else {
    	std::fprintf(os, " %lf", vec_.input[index][i]);
      }
    }
    for (int i = 0; i < vec_size_; ++i) {
      if (i == 0) {
    	std::fprintf(os, "\t%lf", vec_.output[index][i]);
      }else {
    	std::fprintf(os, " %lf", vec_.output[index][i]);
      }
    }
    for (int i = 0; i < vec_size_; ++i) {
      if (i == 0) {
    	std::fprintf(os, "\t%lf", squared_grad_.input[index][i]);
      }else {
    	std::fprintf(os, " %lf", squared_grad_.input[index][i]);
      }
    }
    for (int i = 0; i < vec_size_; ++i) {
      if (i == 0) {
    	std::fprintf(os, "\t%lf", squared_grad_.output[index][i]);
      }else {
    	std::fprintf(os, " %lf", squared_grad_.output[index][i]);
      }
    }
    std::fprintf(os, "\n");
  }

  // file close
  fclose(os);
  
  return SUCCESS;
}


inline int Skipgram::load_bin(const char* filename) {

  //
  FILE* is = fopen(filename, "rb");
  if (is == NULL) {
    std::fprintf(stderr, HERE "cannot open %s\n", filename);
    return FAILURE;
  }
  if (load_bin(is) == FAILURE) {
    return FAILURE;
  }
  fclose(is);
  
  return SUCCESS;
}


inline int Skipgram::load_bin(FILE* is) {
  
  //
  if (fread(&max_vocab_size_, sizeof(int), 1, is) != 1) {
    return FAILURE;
  }
  if (fread(&vec_size_, sizeof(int), 1, is) != 1) {
    return FAILURE;
  }
  if (fread(&window_size_, sizeof(int), 1, is) != 1) {
    return FAILURE;
  }
  if (fread(&neg_sample_num_, sizeof(int), 1, is) != 1) {
    return FAILURE;
  }
  if (fread(&alpha_, sizeof(real_t), 1, is) != 1) {
    return FAILURE;
  }
  if (fread(&subsampling_threshold_, sizeof(real_t), 1, is) != 1) {
    return FAILURE;
  }
  if (fread(&eta_, sizeof(real_t), 1, is) != 1) {
    return FAILURE;
  }
  if (fread(&unigram_table_size_, sizeof(int), 1, is) != 1) {
    return FAILURE;
  }

  //
  if (vocab_.load(is) == FAILURE) {
    return FAILURE;
  }
  if (vec_.input.load(is) == FAILURE) {
    return FAILURE;
  }
  if (vec_.output.load(is) == FAILURE) {
    return FAILURE;
  }
  if (squared_grad_.input.load(is) == FAILURE) {
    return FAILURE;
  }
  if (squared_grad_.output.load(is) == FAILURE) {
    return FAILURE;
  }
  
  //
  if (fread(&total_count_, sizeof(count_t), 1, is) != 1) {
    return FAILURE;
  }
  counts_.resize(max_vocab_size_);
  if (fread(&(counts_[0]), sizeof(count_t), static_cast<size_t>(max_vocab_size_), is) != static_cast<size_t>(max_vocab_size_)) {
    return FAILURE;
  }

  //
  unigram_table_.initialize(unigram_table_size_);
  Random random(0);
  this->rebuild_unigram_table(random);
    
  return SUCCESS;
}


inline int Skipgram::save_bin(const char* filename) const {

  //
  FILE* os = fopen(filename, "wb");
  if (os == NULL) {
    std::fprintf(stderr, HERE "cannot open %s\n", filename);
    return FAILURE;
  }
  if (save_bin(os) == FAILURE) {
    return FAILURE;
  }
  fclose(os);

  return SUCCESS;
}


inline int Skipgram::save_bin(FILE* os) const {

  //
  if (fwrite(&max_vocab_size_, sizeof(int), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(&vec_size_, sizeof(int), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(&window_size_, sizeof(int), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(&neg_sample_num_, sizeof(int), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(&alpha_, sizeof(real_t), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(&subsampling_threshold_, sizeof(real_t), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(&eta_, sizeof(real_t), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(&unigram_table_size_, sizeof(int), 1, os) != 1) {
    return FAILURE;
  }

  //
  if (vocab_.save(os) == FAILURE) {
    return FAILURE;
  }
  if (vec_.input.save(os) == FAILURE) {
    return FAILURE;
  }
  if (vec_.output.save(os) == FAILURE) {
    return FAILURE;
  }
  if (squared_grad_.input.save(os) == FAILURE) {
    return FAILURE;
  }
  if (squared_grad_.output.save(os) == FAILURE) {
    return FAILURE;
  }

  //
  if (fwrite(&total_count_, sizeof(count_t), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(&(counts_[0]), sizeof(count_t), static_cast<size_t>(max_vocab_size_), os) != static_cast<size_t>(max_vocab_size_)) {
    return FAILURE;
  }

  return SUCCESS;
}


inline int Skipgram::vec_size() const {

  return vec_size_;
}


inline int Skipgram::window_size() const {

  return window_size_;
}
 

inline int Skipgram::neg_sample_num() const {

  return neg_sample_num_;
}
 

inline real_t Skipgram::alpha() const {

  return alpha_;
}


inline real_t Skipgram::subsampling_threshold() const {

  return subsampling_threshold_;
}
 
 
inline real_t Skipgram::eta() const {

  return eta_;
}


inline int Skipgram::max_vocab_size() const {
  
  return max_vocab_size_;
}


inline count_t Skipgram::total_count() const {

  return total_count_;
}


inline const std::vector<count_t>& Skipgram::counts() const {

   return counts_;
}


inline const Vocab& Skipgram::vocab() const {

  return vocab_;
}


inline const Parameter& Skipgram::vec() const {

  return vec_;
}


}


