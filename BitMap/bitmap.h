//
// Created by sb on 18-12-9.
//

#ifndef PROJECT_BIGMAP_H
#define PROJECT_BIGMAP_H

#include <stdexcept>
#include <cstring>

class BitMap {
 public:
  BitMap() : bitmap_(nullptr), size_(0) {}
  BitMap(int size) : size_(size) {
    bitmap_ = new char[size];
    if (bitmap_ == nullptr)
      throw;
    else
      memset(bitmap_, 0, sizeof(char) * size);
  }

  BitMap(const BitMap &rhs) : size_(rhs.size_) {
    bitmap_ = new char[size_];
    if (bitmap_ == nullptr)
      throw;
    memcpy(bitmap_, rhs.bitmap_, size_);
  }

  void swap(BitMap &rhs) {
    using std::swap;
    swap(size_, rhs.size_);
    swap(bitmap_, rhs.bitmap_);
  }

  BitMap &operator=(BitMap rhs) {
    swap(rhs);
    return *this;
  }

  BitMap(BitMap &&rhs) : size_(rhs.size_) {
    bitmap_ = new char[size_];
    if (bitmap_ == nullptr)
      throw;
    memcpy(bitmap_, std::move(rhs.bitmap_), size_);
  }

  ~BitMap() {
    delete[] bitmap_;
  }

  bool Set(int index) {
    int addr = index / 8;
    int offset = index % 8;
    unsigned char tmp = 0x1 << offset;
    if (addr > size_ + 1)
      return false;
    else {
      bitmap_[addr] |= tmp;
      return true;
    }
  }

  bool Reset(int index) {
    int addr = index / 8;
    int offset = index % 8;
    unsigned char tmp = 0x1 << offset;
    if (addr > size_ + 1)
      return false;
    else {
      bitmap_[addr] &= ~tmp;
      return true;
    }
  }

  void Clear() {
    memset(bitmap_, 0, size_ * sizeof(char));
  }

  bool Get(int index) {
    int addr = index / 8;
    int offset = index % 8;
    unsigned char tmp = 0x1 << offset;
    if (addr > size_ + 1)
      return false;
    else
      return (bitmap_[addr] & tmp) != 0;
  }

 private:
  char *bitmap_;
  int size_;
};

#endif //PROJECT_BIGMAP_H
