/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include "util.h"
#include "timer.h"
#include "skipgram.h"


using namespace yskip;


struct Configuration {
  int  train_method;
  int  iter_num;
  int  thread_num;
  int  mini_batch_size;
  int  random_seed;
  bool binary_mode;
  bool verbose;
  const char* train_file;
  const char* model_file;
  const char* initial_model_file;
  Configuration();
};


Configuration::Configuration() {

  train_method       = 0;
  thread_num         = 10;
  mini_batch_size    = 10000;
  iter_num           = 5;
  random_seed        = time(NULL);
  binary_mode        = false;
  verbose            = true;
  train_file         = NULL;
  model_file         = NULL;
  initial_model_file = NULL;
}


void print_help() {

  std::cerr << "yskip [option] <train> <model>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Skip-gram model paramters:" << std::endl;
  std::cerr << " -t, --training-method=INT          Training method" << std::endl;
  std::cerr << "                                    0: incremental (default)" << std::endl;
  std::cerr << "                                    1: min-batch" << std::endl;
  std::cerr << "                                    2: batch" << std::endl;
  std::cerr << " -d, --dimensionality=INT           Dimensionality of word embeddings (default: 100)" << std::endl;
  std::cerr << " -w, --window-size=INT              Window size (default: 5)" << std::endl;
  std::cerr << " -n, --negative-sample=INT          Number of negative samples (default: 5)" << std::endl;
  std::cerr << " -a, --alpha=FLOAT                  Distortion parameter (default: 0.75)" << std::endl;
  std::cerr << " -s, --subsampling-threshold=FLOAT  Subsampling threshold (default: 1.0e-5)" << std::endl;
  std::cerr << " -u, --unigram-table-size=INT       Unigram table size used for negative sampling (default: 1e8)" << std::endl;
  std::cerr << " -m, --max-vocabulary-size=INT      Maximum vocabulary size (default: 1e6)" << std::endl;
  std::cerr << " -e, --eta=FLOAT                    Initial learning rate of AdaGrad (default: 0.1)" << std::endl;
  std::cerr << " -b, --mini-batch-size=INT          Mini-batch size (default: 10000)" << std::endl;
  std::cerr << " -B, --binary-mode                  Read/write models in a binary format" << std::endl;
  std::cerr << " -i, --iteration-numbedr            Iteration number in batch learning (default: 5)" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Misc.:" << std::endl;
  std::cerr << " -T, --thread-num=INT               Number of threads (default: 10)" << std::endl;
  std::cerr << " -I, --initial-model=FILE           Initial model (default: NULL)" << std::endl;
  std::cerr << " -r, --random-seed=INT              Random seed (default: current Unix time)" << std::endl;
  std::cerr << " -q, --quiet                        Do not show progress messages" << std::endl;
  std::cerr << " -h, --help                         Show this message" << std::endl;
}


int parse_arg(int argc, char* argv[], Skipgram::Option &option, Configuration& config) {

  int opt;
  char* endptr;
  struct option longopts[] = {
    {"training-algorithm",    required_argument, NULL, 't'},
    {"dimensionality",        required_argument, NULL, 'd'},
    {"wind-size",             required_argument, NULL, 'w'},
    {"negative-sample-num",   required_argument, NULL, 'n'},
    {"alpha",                 required_argument, NULL, 'a'},
    {"subsampling-threshold", required_argument, NULL, 's'},
    {"unigram-table-size",    required_argument, NULL, 'u'},
    {"max-vocabulary-size",   required_argument, NULL, 'm'},
    {"eta",                   required_argument, NULL, 'e'},
    {"mini-batch-size",       required_argument, NULL, 'b'},
    {"binary-mode",           required_argument, NULL, 'B'},    
    {"iteration-number",      required_argument, NULL, 'i'},
    {"initial-model",         required_argument, NULL, 'I'},
    {"thread-num",            required_argument, NULL, 'T'},
    {"random-seed",           required_argument, NULL, 'r'},
    {"quiet",                 no_argument,       NULL, 'q'},
    {"help",                  no_argument,       NULL, 'h'},
    {0,                       0,                 0,    0  },
  };
  while((opt=getopt_long(argc, argv, "d:w:e:u:m:b:Bl:i:n:a:s:t:r:I:hq", longopts, NULL)) != -1){
    switch(opt){
    case 't':
      config.train_method = strtol(optarg, &endptr, 10);
      assert(config.train_method == 0 || config.train_method == 1 || config.train_method == 2);
      break;      
    case 'd':
      option.vec_size = strtol(optarg, &endptr, 10);
      assert(0 < option.vec_size);
      break;
    case 'e':
      option.eta = atof(optarg);
      assert(0.0 < option.eta);
      break;
    case 'w':
      option.window_size = strtol(optarg, &endptr, 10);
      assert(0 < option.window_size);
      break;
    case 'n':
      option.neg_sample_num = strtol(optarg, &endptr, 10);
      assert(0 < option.neg_sample_num);
      break;
    case 'u':
      option.unigram_table_size = strtol(optarg, &endptr, 10);
      assert(100 <= option.unigram_table_size);
      break;
    case 'm':
      option.max_vocab_size = strtol(optarg, &endptr, 10);
      assert(100 <= option.max_vocab_size);
      break;
    case 'b':
      config.mini_batch_size = strtol(optarg, &endptr, 10);
      assert(0 < config.mini_batch_size);
      break;
    case 'B':
      config.binary_mode = true;
      break;
    case 'i':
      config.iter_num = strtol(optarg, &endptr, 10);
      break;
    case 'a':
      option.alpha = atof(optarg);
      assert(0.0 < option.alpha);
      assert(option.alpha <= 1.0);
      break;
    case 's':
      option.subsampling_threshold = atof(optarg);
      assert(0.0 < option.subsampling_threshold);
      break;
    case 'T':
      config.thread_num = strtol(optarg, &endptr, 10);
      assert(0 < config.thread_num);
      break;
    case 'r':
      config.random_seed = strtol(optarg, &endptr, 10);
      break;
    case 'q':
      config.verbose = false;
      break;
    case 'I':
      config.initial_model_file = optarg;
      break;
    case 'h':
      print_help();
      return FAILURE;
    }
  }
  if (optind + 2 != argc) {
    print_help();
    return FAILURE;
  }
  config.train_file = argv[optind];
  config.model_file = argv[optind+1];
  return SUCCESS;
}


inline void print_progress(const count_t sent_num) {

  if (sent_num%100000 == 0) {
    std::fprintf(stderr, "*");
  }else if (sent_num%10000 == 0) {
    std::fprintf(stderr, ".");
  }
  if (sent_num%1000000 == 0) {
    std::fprintf(stderr, " %ldm\n", sent_num/1000000);
  }
}


inline void print_speed(const timeval start_time, const uint64_t progress, const char* unit) {

  timeval current_time;
  gettimeofday(&current_time, NULL);
  double elapsed = interval(start_time, current_time);
  std::fprintf(stderr, "\rspeed: %.2fk (%d %ss/%.2f sec)", static_cast<real_t>(progress)/1000.0/elapsed, progress, unit, elapsed);
}


inline void asyc_sgd2(Skipgram& skipgram, const int start, const int end, const std::vector<std::vector<std::string>>& mini_batch, Random& random) {

  real_t* grad;
  posix_memalign((void**)&grad, 128, sizeof(real_t)*skipgram.vec_size());
  for (int i = start; i < end; ++i) {
    skipgram.train(mini_batch[i], false, grad, random);
  }
  free(grad);
}


inline void asyc_sgd(Skipgram& skipgram, const Configuration& config, const std::vector<std::vector<std::string>> mini_batch, Random& random) {

  int n = mini_batch.size();
  std::vector<std::thread> threads; 
  for (int i = 0; i < config.thread_num; ++i) {
    threads.push_back(std::thread(&asyc_sgd2, std::ref(skipgram), i*n/config.thread_num, std::min<int>((i+1)*n/config.thread_num, n), std::ref(mini_batch), std::ref(random)));
  }
  for (int i = 0; i < config.thread_num; ++i) {
    threads[i].join();
  }
}


inline int train_batch(Skipgram& skipgram, const Configuration& config, Random& random) {

  //
  FILE* is = fopen(config.train_file, "r");
  if (is == NULL) {
    std::fprintf(stderr, "failed to open %s\n", config.train_file);
    return FAILURE;
  }
  setvbuf(is, NULL, _IOFBF, BUFF_SIZE);

  //
  if (config.verbose) {
    std::fprintf(stderr, "Precomputing unigram table...");
  }

  /*****************************************************
   *  construct unigram table
   *****************************************************/
  char line[BUFF_SIZE];
  while (fgets(line, BUFF_SIZE, is) != NULL) {
    line[strlen(line)-1] = '\0';
    skipgram.update_unigram_table(tokenize(line), random);
  }
  skipgram.rebuild_unigram_table(random); // make sure that the unigram table is calculated without approximation
  
  //
  if (config.verbose) {
    std::fprintf(stderr, " done (vocab size=%ld)\n", skipgram.vocab().size());
    std::fprintf(stderr, "Training batch SGNS\n");
  }

  /*****************************************************
   *  SGD
   *****************************************************/
  time_t start_time = time(NULL);
  count_t sent_num = 0;
  std::vector<std::vector<std::string>> mini_batch;
  for (int iter = 0; iter < config.iter_num; ++iter) {
    rewind(is);
    while (fgets(line, BUFF_SIZE, is) != NULL) {
      line[strlen(line)-1] = '\0';
      
      mini_batch.push_back(tokenize(line));
      if (mini_batch.size() == config.mini_batch_size || feof(is) != 0) {	
	asyc_sgd(skipgram, config, mini_batch, random);
	mini_batch.clear();
      }
      
      ++sent_num;
      if (config.verbose) {
	print_progress(sent_num);
      }
    }
  }
  fclose(is);
  
  //
  time_t elapsed_time = time(NULL) - start_time;;
  if (config.verbose) {
    std::fprintf(stderr, " done (%lf=%ld/%ld sent/sec)\n", static_cast<double>(sent_num)/static_cast<double>(elapsed_time), sent_num, elapsed_time);
  }
  
  return SUCCESS;
}


inline int train_incremental(Skipgram& skipgram, const Configuration& config, Random& random) {

  //
  FILE* is = stdin;
  if (strcmp(config.train_file, "-") != 0) {
    is = fopen(config.train_file, "r");
  }
  if (is == NULL) {
    std::fprintf(stderr, "failed to open %s\n", config.train_file);
    return FAILURE;
  }
  setvbuf(is, NULL, _IOFBF, BUFF_SIZE);

  //
  if (config.verbose) {
    std::fprintf(stderr, "Training incremental SGNS\n");
  }
  
  //
  time_t start_time = time(NULL);
  real_t* grad;
  posix_memalign((void**)&grad, 128, sizeof(real_t)*skipgram.vec_size());
  count_t sent_num = 0;
  char line[BUFF_SIZE];
  while (fgets(line, BUFF_SIZE, is) != NULL) {
    line[strlen(line)-1] = '\0';
    skipgram.train(tokenize(line), true, grad, random);
    ++sent_num;
    if (config.verbose) {
      print_progress(sent_num);
    }
  }
  fclose(is);
  free(grad);

  //
  time_t elapsed_time = time(NULL) - start_time;;
  if (config.verbose) {
    std::fprintf(stderr, " done (%lf=%ld/%ld sent/sec)\n", static_cast<double>(sent_num)/static_cast<double>(elapsed_time), sent_num, elapsed_time);
  }
  
  return SUCCESS;
}


inline int train_mini_batch(Skipgram& skipgram, const Configuration& config, Random& random) {
  
  //
  FILE* is = stdin;
  if (strcmp(config.train_file, "-") != 0) {
    is = fopen(config.train_file, "r");
  }
  if (is == NULL) {
    std::fprintf(stderr, "failed to open %s\n", config.train_file);
    return FAILURE;
  }
  setvbuf(is, NULL, _IOFBF, BUFF_SIZE);

  if (config.verbose) {
    std::fprintf(stderr, "Training mini-batch SGNS\n");
  }
  
  //
  char line[BUFF_SIZE];
  count_t sent_num = 0;
  std::vector<std::vector<std::string>> mini_batch;  
  while (fgets(line, BUFF_SIZE, is) != NULL) {
    line[strlen(line)-1] = '\0';

    //
    mini_batch.push_back(tokenize(line));
    skipgram.update_unigram_table(mini_batch.back(), random);
    
    //
    if (mini_batch.size() == config.mini_batch_size || feof(is) != 0) {	
      asyc_sgd(skipgram, config, mini_batch, random);
      mini_batch.clear();
    }

    //
    ++sent_num;
    if (config.verbose) {
      print_progress(sent_num);
    }
  }
  
  if (strcmp(config.train_file, "-") != 0) {
    fclose(is);
  }
  
  return SUCCESS;
}


int main(int argc, char **argv) {

  /*
   * parse arguments
   */
  Configuration config;
  Skipgram::Option option;
  if (parse_arg(argc, argv, option, config) == FAILURE) {
    return FAILURE;
  }
  
  /*
   * initialize model
   */
  if (config.verbose) {
    std::fprintf(stderr, "Initializing model...");
  }
  Random random(config.random_seed);
  Skipgram skipgram(option, random);
  if (config.initial_model_file != NULL) {
    // configuration specified by the option is overwritten
    if (skipgram.load(config.initial_model_file, config.binary_mode) == FAILURE) {
      return FAILURE;
    }
  }
  if (config.verbose) {
    std::fprintf(stderr, " done\n");
  }
  
  /*
   * train model
   */
  if (config.train_method == 0) {
    if (train_incremental(skipgram, config, random) == FAILURE) {
      return FAILURE;
    }
  }else if (config.train_method == 1) {
    if (train_mini_batch(skipgram, config, random) == FAILURE) {
      return FAILURE;
    }
  }else {
    if (train_batch(skipgram, config, random) == FAILURE) {
      return FAILURE;
    }
  }
  
  /*
   * save model
   */
  if (skipgram.save(config.model_file, config.binary_mode) == FAILURE) {
    return FAILURE;
  }
  
  return SUCCESS;
}

