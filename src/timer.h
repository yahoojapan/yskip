/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#pragma once
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include "util.h"


namespace yskip {


class Timer {
 public:
  Timer();
  ~Timer() {};
  void start();
  void stop();
  double elapsed_time() const;
  
 private:
  double  elapsed_time_;
  timeval start_time_;
  DISALLOW_COPY_AND_ASSIGN(Timer);
};


inline Timer::Timer() {

  elapsed_time_ = 0.0;
  start();
}
  

inline void Timer::start() {

  gettimeofday(&start_time_, NULL);
}


inline void Timer::stop() {

  timeval end_time;
  gettimeofday(&end_time, NULL);

  elapsed_time_ += (double)(end_time.tv_sec) + (double)(end_time.tv_usec) * 0.001 * 0.001 - (double)(start_time_.tv_sec) - (double)(start_time_.tv_usec) * 0.001 * 0.001;
}
  

inline double Timer::elapsed_time() const {

  return elapsed_time_;
}
 

}
