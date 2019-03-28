/*******************************************
 * Copyright (C) 2017 Yahoo! JAPAN Research
 *******************************************/
#pragma once
#include <stdio.h>
#include <stdlib.h> //
#include <numeric>  // accumulate
#include <iostream>
#include <unordered_set>
#include <cassert>
#include "util.h"


namespace yskip {


class DenseMatrix {
 public:
  DenseMatrix();
  DenseMatrix(const DenseMatrix& other);
  DenseMatrix(const int row_num, const int col_num, const real_t val=0.0);
  ~DenseMatrix();
  DenseMatrix& operator=(const DenseMatrix& other);
  const real_t* operator[](const int row) const;
  real_t* operator[](const int row);
  void reduce(const std::unordered_set<int>& reserved_rows);
  int row_num() const;
  int col_num() const;
  int load(FILE* is);
  int save(FILE* os) const;
  
  
 private:
  int     row_num_;
  int     col_num_;
  real_t* data_;
};


inline DenseMatrix::DenseMatrix() {

  row_num_ = 1;
  col_num_ = 1;
  posix_memalign((void**)&data_, 128, sizeof(real_t)*row_num_*col_num_);
  std::fill(data_, data_ + row_num_*col_num_, 0.0);
}


inline DenseMatrix::DenseMatrix(const int row_num, const int col_num, const real_t val) {

#ifdef __YSKIP_DEBUG__
  assert(0 < row_num);
  assert(0 < col_num);
#endif
  
  row_num_ = row_num;
  col_num_ = col_num;
  posix_memalign((void**)&data_, 128, sizeof(real_t)*row_num_*col_num_);
  std::fill(data_, data_ + row_num_*col_num_, val);
}


inline DenseMatrix::DenseMatrix(const DenseMatrix& other) {

  row_num_ = other.row_num();
  col_num_ = other.col_num();
  posix_memalign((void**)&data_, 128, sizeof(real_t)*row_num_*col_num_);
  for (int i = 0; i < row_num_; ++i) {
    std::copy(other[i], other[i] + col_num_, data_+col_num_*i);
  }
}


inline DenseMatrix::~DenseMatrix() {

  free(data_);
}


inline DenseMatrix& DenseMatrix::operator=(const DenseMatrix& other) {

  if (this != &other) {
    row_num_ = other.row_num();
    col_num_ = other.col_num();
    free(data_);
    posix_memalign((void**)&data_, 128, sizeof(real_t)*row_num_*col_num_);
    for (int i = 0; i < row_num_; ++i) {
      std::copy(other[i], other[i] + col_num_, data_+col_num_*i);
    }
  }
  return *this;
}


inline const real_t* DenseMatrix::operator[](const int row) const {
  
  return data_ + col_num_*row;
}


inline real_t* DenseMatrix::operator[](const int row) {

  return data_ + col_num_*row;
}


inline int DenseMatrix::row_num() const {

  return row_num_;
}
 

inline int DenseMatrix::col_num() const {
  
  return col_num_;
}


inline void DenseMatrix::reduce(const std::unordered_set<int>& reserved_rows) {

  int j = 0;
  for (int i = 0; i < row_num_; ++i) {
    if (reserved_rows.find(i) != reserved_rows.end()) {
      std::copy(data_ + i * col_num_, data_ + (i+1) * col_num_, data_ + j * col_num_);
      ++j;
    }
  }
}


inline int DenseMatrix::load(FILE* is) {

  if (fread(&row_num_, sizeof(int), 1, is) != 1) {
    return FAILURE;
  }
  if (fread(&col_num_, sizeof(int), 1, is) != 1) {
    return FAILURE;
  }
  free(data_);
  posix_memalign((void**)&data_, 128, sizeof(real_t)*row_num_*col_num_);
  if (fread(data_, sizeof(real_t), static_cast<size_t>(row_num_*col_num_), is) != static_cast<size_t>(row_num_*col_num_)) {
    return FAILURE;
  }
  return SUCCESS;
}
 

inline int DenseMatrix::save(FILE* os) const {

  if (fwrite(&row_num_, sizeof(int), 1, os) != 1) {
    return FAILURE;
  }
  if (fwrite(&col_num_, sizeof(int), 1, os) != 1) {
    return FAILURE;
  }  
  if (fwrite(data_, sizeof(real_t), static_cast<size_t>(row_num_*col_num_), os) != static_cast<size_t>(row_num_*col_num_)) {
    return FAILURE;
  }
  return SUCCESS;
}


inline bool operator==(const DenseMatrix& m1, const DenseMatrix& m2) {

  if (m1.col_num() != m2.col_num() || m1.row_num() != m2.row_num()) {
    return false;
  }
  for (int i = 0; i < m1.row_num(); ++i) {
    for (int j = 0; j < m1.col_num(); ++j) {
      if (approx_equal(m1[i][j], m2[i][j]) == false) {
	return false;
      }
    }
  }
  return true;
}


}
